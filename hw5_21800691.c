//21800691 Operating Systems HW5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>

#include "queue.h" //header file for que, reside in same directory

//function declare
void *wordSearch(void *);

//global variables declare
char **key;
pthread_mutex_t mutexLock;
Queue myQue;
char *task[4096];
char *newTask[4096];
int newIndex = 0;
int idx = 0;

//function definition
void *wordSearch(void *arg)
{
    //local variables
    DIR *userPath = NULL;
    FILE *fileOpener;
    struct dirent *file = NULL;
    char taskDirectory[4096];
    char *filename;
    char readBuffer[4096];

    pthread_mutex_lock(&mutexLock); //protect que critical section

    if ((task[idx] = deque(&myQue)) == NULL) //deque a task
    {
        perror("deque fail\n");
        exit(1);
    }
    sprintf(taskDirectory, "%s", task[idx]); //trnasfer global task list to local
    idx++;

    pthread_mutex_unlock(&mutexLock); //end que critical section

    //string process that its a directory
    size_t len = strlen(taskDirectory);
    if (taskDirectory[len - 1] != '/')
        strcat(taskDirectory, "/");

    //error checking
    if ((userPath = opendir(taskDirectory)) == NULL)
    {
        perror("opendir fail\n");
        exit(1);
    }

    //iterate all the contents in the directory
    for (int i = 0; (file = readdir(userPath)) != NULL; i++)
    {
        filename = file->d_name;
        //filter parent and itself
        if (!strcmp(filename, "..") || !strcmp(filename, "."))
        {
            continue;
        }

        //open the task and check error
        char fullPath[4096];
        sprintf(fullPath, "%s %s%s", "file", taskDirectory, filename);
        if ((fileOpener = popen(fullPath, "r")) == NULL)
        {
            perror("popen fail\n");
            exit(1);
        }
        if (fgets(readBuffer, 4096, fileOpener) == NULL)
        {
            perror("fgets fail\n");
            exit(1);
        }

        //opened as text file
        if (strstr(readBuffer, "text"))
        {
            int line = 1;
            char path[4096];
            char str[4096];
            sprintf(path, "%s%s", taskDirectory, filename);
            FILE *fd = fopen(path, "r");
            if (fd == NULL)
            {
                perror("fopen fail\n");
                exit(1);
            }

            //iterate through the file till end
            while (!feof(fd))
            {
                int keyCheck = 1;
                char *p = fgets(str, 4096, fd);
                for (int j = 0; key[j] != NULL; j++)
                {
                    if (!strstr(str, key[j]))
                        keyCheck = keyCheck * 0;
                }
                //key hit, print result
                if (keyCheck != 0)
                    printf("key:%s is in line:%d of file:%s\n", str, line, path);
                line++;
            }
            fclose(fd);
        }
        //opened as directory file
        else if (strstr(readBuffer, "directory"))
        {
            //need to add new tasks to the que
            char moreTask[4096];
            sprintf(moreTask, "%s%s%s", taskDirectory, filename, "/");

            pthread_mutex_lock(&mutexLock); //mutexLock critical section of que

            newTask[newIndex] = malloc(sizeof(char) * strlen(moreTask));
            strcpy(newTask[newIndex], moreTask);
            enque(&myQue, newTask[newIndex]);

            pthread_mutex_unlock(&mutexLock); // end critical section
        }
        else
            continue;
    }
    return NULL;
} //end of wordSearch

int main(int argc, char *argv[])
{
    int option = 0;
    extern char *optarg; //option arguments
    extern int optind;   //option indexes
    pthread_mutex_init(&mutexLock, NULL);
    char *targetDirectory;
    int numberOfTasks = 0;
    int numberOfThreads = 0;

    //option handling
    while ((option = getopt(argc, argv, "t:")) != -1)
    {
        switch (option)
        {
        case 't':
            numberOfThreads = atoi(optarg);
            if (numberOfThreads < 1 || numberOfThreads > 16)
            {
                perror("thread number error \n");
                exit(1);
            }
            break;
        case '?':
            perror("wrong argument\n");
            exit(1);
        }
    }

    //precheck
    if ((targetDirectory = argv[optind]) == NULL)
    {
        printf("dir error\n");
        return 0;
    }
    if (argc - optind == 1 || argc - optind > 9)
    {
        printf("key error\n");
        return 0;
    }

    optind++;
    key = (char **)malloc(sizeof(char *) * (argc - optind));
    for (int i = 0; i < argc - optind; i++)
    {
        key[i] = (char *)malloc(sizeof(char) * strlen(argv[optind + i]));
    }
    for (int i = 0; i < argc - optind; i++)
    {
        key[i] = argv[optind + i];
    }

    //initialize que and let it roll!
    initializeQue(&myQue);
    enque(&myQue, targetDirectory);

    pthread_t searchingThread[numberOfThreads]; //as many as t
    while (1)
    {
        int i = 0;
        numberOfTasks = myQue.queCount;
        //prevent overflow
        if (numberOfThreads < numberOfTasks)
            numberOfTasks = numberOfThreads;

        for (i = 0; i < numberOfTasks; i++)
        {
            pthread_create(&searchingThread[i], NULL, wordSearch, NULL);
        }
        for (i = 0; i < numberOfTasks; i++)
        {
            pthread_join(searchingThread[i], NULL);
        }
        numberOfTasks = myQue.queCount;

        //repeat until que is empty
        if (empty(&myQue) == 1)
            break;
    }
    free(key); //return allocated key memory
    return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <dirent.h>

#include "functions.h"

int CommandHandler(char* received, clock_t begin, threadpool* pool){

    char* temp = strdup(received);
    char* parser = temp;
    temp = strtok(temp,"\t\n\r ");

    if(!strcmp(temp,"STATS")){
        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

        printf("Server up for %f sec, served %d pages, %d bytes\n",time_spent,pool->statistics->pages,pool->statistics->bytes);
    }
    else if(!strcmp(temp,"SHUTDOWN")){
        free(parser);
        return 1;
    }
    else if(!strcmp(temp,"SEARCH")){
        //JobExecutor(pool,received);
        free(parser);
        return 0;
    }
    else
        printf("Wrong Command given\n");

    free(parser);
    return 0;
}

char *multi_tok(char *input, char *delimiter) {
    static char *string;
    if (input != NULL)
        string = input;

    if (string == NULL)
        return string;

    char *end = strstr(string, delimiter);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(delimiter);
    return temp;
}

void FileManagment(threadpool* pool,char* temp, char* url){

    char* site = strtok(url,"p");

    int count = 0;
    char* sub = (char*)malloc(9*sizeof(char));
    bzero(sub,9);
    strcpy(sub,"<a href");
    const char *p;

    if (temp && sub) {
        for (p = temp; (p = strstr(p, sub)) != NULL; p++) {
            count++;
            int pos = p - temp;

            while(temp[pos]!='=')
                pos++;
            pos++;
            int i;
            char* link = (char*)malloc(256*sizeof(char));
            if(temp[pos]=='.'){
                while(temp[pos]!='/')
                    pos++;
                i=0;
                while(temp[pos]!='>'){
                    link[i] = temp[pos];
                    pos++;
                    i++;
                }
            }
            else{
                strcpy(link,site);
                i = strlen(link);
                while(temp[pos]!='>'){
                    link[i] = temp[pos];
                    pos++;
                    i++;
                }
            }
            link[i]='\0';
            printf("%s\n",link);
            if(q_search(pool->dummyQueue,link)!=1){

                pthread_mutex_lock(&(pool->lock));

                Job *newJob = (Job*)malloc(sizeof(Job));
                newJob->url = strdup(link);
                newJob->next = NULL;
                enqueue(pool->dummyQueue,newJob);
                pool->statistics->pages++;

                pthread_mutex_unlock(&(pool->lock));

                thr_pool_add(pool,link);
            }
            free(link);
            if (*p == '\0')  /* special case for the empty string */
                break;
        }
    }
    free(temp);
    free(sub);
}

void DirCreate(threadpool* pool){
    struct stat st = {0};

    if (stat(pool->directory, &st) == -1)
        mkdir(pool->directory, 0700);
    pool->fp = fopen("paths.txt","wb");
    if (pool->fp == NULL){
        perror("Error creating my_log file\n");
        exit(1);
    }
}

void FileCreate(threadpool* pool, char* file_name, char* content){

    struct stat st = {0};

    char pathFile[256];
    char dir[256];

    char* name = strdup(file_name);
    char* temp = strtok(name,"/");
    char* site = strdup(temp);

    temp = strtok(NULL,"/");
    char* page = strdup(temp);

    bzero(pathFile,256);
    bzero(dir,256);

    strcpy(dir,pool->directory);
    strcat(dir,"/");
    strcat(dir,site);

    if (stat(dir, &st) == -1)
        mkdir(dir, 0700);

    strcpy(pathFile,dir);
    strcat(pathFile,"/");
    strcat(pathFile,page);

    fprintf(pool->fp,"%s\n",dir);
    //pathFile[strlen(pathFile)] = '\0';

    printf("*************************\n");
    printf("%s\n",pathFile);
    FILE* fp = fopen(pathFile,"wb");
    if (fp == NULL){
        perror("Error creating my_log file\n");
        exit(1);
    }
    char* buf = content;
    for(int i=0;i<strlen(content);i++){
        if ((content[i]=='\n') && (content[i+1]=='\n'))
            break;
        buf++;
    }
    fprintf(fp, "%s\n", buf);
    fclose(fp);

    free(name);
    free(site);
    free(page);
}

char** StrtokPaths(int numPaths, char *path){
    char** dirs = (char**)malloc(numPaths*sizeof(char*));
    int k=0;
    char* temp = strtok(path," ");
    while(temp!=NULL){
        dirs[k++] = strdup(temp);
        temp = strtok (NULL, " ");
    }
    return dirs;
}

/*Counts number of directories given*/
int TotalDirs(char* path){
    int numPaths = 1;
    for(unsigned int i=0; i<strlen(path); i++){
        if(path[i]==' ')
            numPaths++;
    }
    return numPaths;
}

/*Counts total number of files into all directories*/
int TotalFiles(int numDirs, char** dirs){
    int file_count = 0;
    for(int i=0; i<numDirs; i++){
        DIR * dirp;
        struct dirent * entry;
        dirp = opendir(dirs[i]);
        while ((entry = readdir(dirp)) != NULL) {
            if (entry->d_type == DT_REG)
                 file_count++;
        }
        closedir(dirp);
    }
    return file_count;
}

/*Opening files*/
FILE** OpenFiles(int numPaths, char** files){
    FILE **fp = (FILE**)malloc(numPaths*sizeof(FILE*));
    for(int i=0; i<numPaths; i++){
        fp[i] = fopen(files[i], "r");
        if(!fp[i]){
            perror("Wrong Input File\n");
            exit(1);
        }
    }
    return fp;
}

/*Keeping files' names into an array*/
char** CopyFileNames(int numPaths, int numDirs, char** dirs){

    char** files = (char**)malloc(numPaths*sizeof(char*));
    for(int i=0;i<numPaths;i++)
        files[i] = (char*)malloc(BUFSIZ*sizeof(char));

    int k=0;
    for(int i=0; i<numDirs; i++){
        DIR * dirp;
        struct dirent * entry;
        dirp = opendir(dirs[i]);
        while ((entry = readdir(dirp)) != NULL){
                if((strcmp(entry->d_name,".")) && (strcmp(entry->d_name,".."))){
                    strcpy(files[k],dirs[i]);
                    strcat(files[k],"/");
                    strcat(files[k],entry->d_name);
                    k++;
                }
        }
        closedir(dirp);
    }
    return files;
}

/*Counts total number of lines from all files*/
int TotalLines(int numPaths, FILE** fp){
    int total_lines = 0;
    for(int i=0; i<numPaths; i++){
         char ReadVal;
         while ((ReadVal = fgetc(fp[i])) != EOF){
             if(ReadVal == '\n')
                 total_lines++;
         }
         rewind(fp[i]);
    }
    return total_lines;
}

/*Counts each file's number of lines*/
int* LinesPerFile(int numPaths, FILE** fp){

    int* lines = (int*)malloc(numPaths*sizeof(int));
    for(int i=0; i<numPaths; i++){
         lines[i] = 0;
         char ReadVal;
         while ((ReadVal = fgetc(fp[i])) != EOF){
             if(ReadVal == '\n')
                 lines[i]++;
         }
         rewind(fp[i]);
    }
    return lines;
}

/*Finds largest line*/
int MaxWidth(int numPaths, int pos, FILE** fp){

     int widthCount = 0;
     int max = 0;
     char ReadVal;
     while ((ReadVal = fgetc(fp[pos])) != EOF){
         widthCount++;
         if(ReadVal == '\n'){
             if(max < widthCount)
                 max = widthCount;
             widthCount = 0;
         }
     }
     widthCount = max+1;
     rewind(fp[pos]);
     return widthCount;
}

/*Keeping the context of each file into a map*/
char** MapConstruction(int numPaths, int lines, FILE** fp){
    char **map;
    map = (char**)malloc(lines*sizeof(char*));
    int k=0;
    for(int i=0; i<numPaths; i++){
         int widthCount = MaxWidth(numPaths,i,fp);
         char* buff = (char*)malloc(widthCount*sizeof(char));
         while(fgets(buff, widthCount, fp[i]) != NULL)
            map[k++] = strdup(buff);

        fclose(fp[i]);
        free(buff);
    }
    return map;
}

void removeChar(char *str, char garbage){
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

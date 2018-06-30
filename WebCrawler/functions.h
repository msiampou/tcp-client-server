
#include <time.h>

#include "threadpool.h"

int CommandHandler(char*,clock_t,threadpool*);
void FileManagment(threadpool*,char*,char*);
void DirCreate(threadpool*);
void FileCreate(threadpool*,char*,char*);
void removeChar(char*,char);
void PathSeparator(char*);
int TotalLines(int,FILE**);
FILE** OpenFiles(int,char**);
char** StrtokPaths(int,char*);
int TotalDirs(char*);
int TotalFiles(int,char**);
char** CopyFileNames(int,int,char**);
int* LinesPerFile(int,FILE**);
int MaxWidth(int,int,FILE**);
char** MapConstruction(int,int,FILE**);
void JobExecutor(threadpool*,char*);

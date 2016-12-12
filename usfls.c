#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <dirent.h>

int main(int argc, char **argv)
{
    DIR *dirp;
    struct dirent *dp;
    int len;
    char *name;
    bool flag = false;

    if (argv[1] != NULL) {
        name = argv[1];
        if (strcmp(name,"-a") == 0) { // if it has flag -a
            flag = true;
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            dirp = opendir(cwd);

        } else {
            dirp = opendir(argv[1]); // absolute path
            if (dirp == NULL) {
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                strcat(cwd,name);  // concat current directory and relative path to open relative path
                dirp = opendir(cwd);
            }
        }
    } else { // else open current working directory
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        dirp = opendir(cwd);
    }

    if (dirp == NULL) {
        printf("Cannot opendir()\n");
        exit(-1);
    }

    while ((dp = readdir(dirp)) != NULL) {
        if (flag == true) { // if the -a flag is turned on
            write(1,dp->d_name, strlen(dp->d_name));
            write(1,"\n", 1); 
        } else {
            if (dp->d_name[0] != '.') {
                write(1,dp->d_name, strlen(dp->d_name));
                write(1,"\n", 1);
            }
        }
    }
    closedir(dirp);

    return 0;
}
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

# define COMMAND_LENGTH 124
# define ARGS_SIZE 10
# define JOBS_SIZE 40

typedef struct job {
    pid_t pid;
    char jobName[COMMAND_LENGTH];
    int isBackground;
} job;
/**
 * JobsCheck.
 * going through the array and check if the job was finished
 * and updating the array accordingly.
 * @param jobs array .
 */
void JobsCheck(job jobs[JOBS_SIZE]) {
    int stat;
    for (int i = 0; i < JOBS_SIZE; ++i) {
        if (jobs[i].isBackground == 1) {
            // if child is finished .
            if (waitpid(jobs[i].pid , &stat , WNOHANG)) {
                jobs[i].isBackground = 0;
            } else {
                jobs[i].isBackground = 1;
            }
        }
    }
}
/**
 * AddJob .
 * adding new job to the jobs array .
 * @param newJob new job to add .
 * @param jobs jobs array.
 * @return 1 if succeed else 0.
 */
void AddJob(job newJob, job jobs[JOBS_SIZE]) {
    for (int i = 0; i < JOBS_SIZE ; ++i) {
        if(!jobs[i].isBackground) {
            jobs[i] = newJob;
            return;
        }
    }
    printf("Too much jobs, can't add a job to the jobs array\n");
}
/**
 * printJobs.
 * @param jobs array we print .
 */
void printJobs (job jobs[JOBS_SIZE]) {
    for (int i = 0; i < JOBS_SIZE ; ++i) {
        if(jobs[i].isBackground == 1) {
                printf("%d %s \n",jobs[i].pid, jobs[i].jobName);
            }
        }
    }
/**
 * getArgs .
 * @param args  string arrray of the arguments .
 * @param cmd command we taking out the args from it .
 * @return 0 if there's no-waiting flag , else 1 .
 */
int getArgs(char *args[ARGS_SIZE],char cmd[COMMAND_LENGTH]) {
    int i = 0;
    char *arg = strtok(cmd , " ");
    while (arg != NULL) {
        args[i++] = arg;
        arg = strtok(NULL , " ");
    }
    if (!strcmp(args[i - 1] , "&")) {
        args[i-1] = NULL;
        return 0;
    }
    args[i] = NULL;
    return 1;
}
/**
 * CD.
 * cd implemention .
 * from stackoverflow
 * @param args args of the command .
 * @return .
 */
int CD(char *args[]) {
    // If we write no path , then go the home directory
    if (args[1] == NULL) {
        chdir(getenv("HOME"));
        return 1;
        // Else we change the directory to the one specified by the argument
    } else {
        if (chdir(args[1]) == -1) {
            printf(" no such directory\n");
            return -1;
        }
        return 0;
    }
}

/**
 * main .
 */
int main() {
    int wait, ret_code ,stat;
    pid_t pid;
    char *args[ARGS_SIZE];
    char cmd[COMMAND_LENGTH];
    // string that saves the original command.
    char cmdSaver[COMMAND_LENGTH];
    job jobsArr[JOBS_SIZE];
    // initializing the jobs array
    for (int i = 0; i < JOBS_SIZE ; ++i) {
        jobsArr[i].isBackground = 0;
    }
    while (1) {
        printf("prompt >");
        fgets(cmd, COMMAND_LENGTH , stdin);
        if (cmd[0]=='\n') {
            continue;
        }
        // deleting '\n' char from the cmd .
        if (strlen(cmd) > 0) {
            cmd[strlen(cmd) - 1] = '\0';
        }
        if (!strcmp("exit", cmd)) {
            exit(1);
        }
        strcpy(cmdSaver , cmd);
        // if the command is cd
        if (!strncmp(cmd , "cd", 2)) {
            CD(args);
            //if the command is jobs
        } else if (!strcmp(cmdSaver, "jobs")) {
            //updating the jobs array before adding a new job to it .
            JobsCheck(jobsArr);
            printJobs(jobsArr);
        } else {
            wait = getArgs(args,cmd);
            pid = fork();
            if (pid < 0) {
                printf("error in fork\n");
                return -1;
            }
            if (pid == 0) {
                /*son*/
                ret_code = execvp(args[0] , args);
                fprintf(stderr , "error in system call\n");
                if (ret_code == -1) {
                    exit(-1);
                }
            } else {
                /*father*/
                printf("%d\n" , pid);
                if (wait) {
                    waitpid(pid , &stat , 0);
                    //if it's background process we add it to jobsArr
               } else {
                   job job1;
                   job1.pid = pid;
                   job1.isBackground = 1;
                   cmdSaver[strlen(cmdSaver)-1] = '\0';
                   strcpy(job1.jobName , cmdSaver);
                    //updating the jobs array before adding a new job to it .
                   JobsCheck(jobsArr);
                   AddJob(job1,jobsArr);
               }
            }
        }
    }

}
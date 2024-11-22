#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <limits.h>

#include <signal.h>

void handle_sigchld(int sig) {

    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("CHILD with PID %d, PPID %d exited with code %d\n", pid,
                   getppid(), WEXITSTATUS(status));
        }
    }
};

struct Program {
    char prog_name[NAME_MAX + 1];
    char *arv[10];
};

void create_processes(int prog_count, int repeat_count,
                      struct Program programs[], int block_parent) {
    int status;
    int index = 0, repeat = 0;

    pid_t pid;
    pid_t parent_pid = getpid();

    if (!block_parent) {
        struct sigaction sa;
        sa.sa_handler = handle_sigchld;
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }
    }

    for (int i = 0; i < prog_count * repeat_count; ++i) {
        if (getpid() == parent_pid) {

            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {
                if (block_parent) {
                    printf("CHILD with PID %d, PPID %d exited with code %d\n\n",
                           getpid(), getppid(), WEXITSTATUS(status));
                }
                execv(programs[index].prog_name, programs[index].arv);
            } else if (getpid() == parent_pid) {
                if (++repeat == repeat_count) {
                    repeat = 0;
                    index++;
                }

                if (block_parent) {
                    printf("PARENT with PID %d created CHILD with PID %d\n",
                           getpid(), pid);
                    waitpid(pid, &status, 0);
                }
            }
        }
    }
}

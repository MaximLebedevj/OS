#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t pid;
    int rv;
    int status;
    pid_t parent_pid = getpid();
    printf("START\n");

    printf("parent PID = %d\n", parent_pid);

    for (int i = 0; i < 3; ++i) {
        if (getpid() == parent_pid) {
            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {
                printf("CHILD: pid = %d, PID = %d, PPID = %d\n", pid, getpid(),
                       getppid());
                sleep(7);
                exit(228);
            } else if (getpid() == parent_pid) {
                printf("HERE_FOR\n");
                printf("PARENT: PID = %d\n", getpid());
                waitpid(pid, &status, 0);
                printf("status = %d\n", WEXITSTATUS(status));
            }
        }
    }
    return 0;
}

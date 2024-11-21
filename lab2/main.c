#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <limits.h>

struct Program {
    char prog_name[NAME_MAX + 1];
    char *arv[10];
};

void create_processes(int prog_count, int repeat_count, struct Program programs[])
{
    int status;
    int index = 0, repeat = 0;

    pid_t pid;
    pid_t parent_pid = getpid();

    for (int i = 0; i < prog_count * repeat_count; ++i) {
        if (getpid() == parent_pid) {

            pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            }
            else if (pid == 0) {
                printf("CHILD: PID = %d, PPID = %d\n", getpid(), getppid());
                execv(programs[index].prog_name, programs[index].arv);
            }
            else if (getpid() == parent_pid) {
                if (++repeat == repeat_count) {
                    repeat = 0;
                    index++;
                }
                waitpid(pid, &status, 0);
                printf("Status = %d\n", WEXITSTATUS(status));
            }
        }
    }
}

int main()
{
    struct Program programs[4] = {{"child", {"child", "0", NULL}},
    {"child", {"child", "1", NULL}},
    {"child", {"child", "2", NULL}},
    {"child", {"child", "3", NULL}}};

    create_processes(4, 2, programs);

    return 0;
}

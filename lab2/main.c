#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <limits.h>

#define BLOCK_PARENT 1
#define REPEAT 2
#define PROGRAMS_COUNT 4

struct Program {
    char prog_name[NAME_MAX + 1];
    char *arv[10];
};

void create_processes(int prog_count, int repeat_count,
                      struct Program programs[], int block_parent);

int main() {
    struct Program programs[PROGRAMS_COUNT] = {{"child", {"child", "0", NULL}},
                                               {"child", {"child", "1", NULL}},
                                               {"child", {"child", "2", NULL}},
                                               {"child", {"child", "3", NULL}}};

    create_processes(PROGRAMS_COUNT, REPEAT, programs, BLOCK_PARENT);

    for (int i = 0; i < 5; ++i) {
        printf("NON CHILD process making some actions...\n");
        sleep(1);
    }

    return 0;
}

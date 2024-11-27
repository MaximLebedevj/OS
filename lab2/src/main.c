#include <stdio.h>

#ifdef WIN32
#    include <windows.h>
#    define MAX_LEN MAX_PATH
#else
#    include <limits.h>
#    include <unistd.h>
#    define MAX_LEN NAME_MAX
#endif

#define BLOCK_PARENT 1
#define PROGRAMS_COUNT 4

struct Program {
    const char prog_name[MAX_LEN + 1];
    char *arv[10];
};

void start_processes(int prog_count, struct Program programs[],
                      int block_parent);

int main()
{
    struct Program programs[4] = {{"child", {"child", "1", NULL}},
                                  {"child", {"child", "2", NULL}},
                                  {"child", {"child", "3", NULL}},
                                  {"child", {"child", "4", NULL}}};

    printf("\n\n BLOCK_PARENT = 1\n\n\n");
    start_processes(PROGRAMS_COUNT, programs, BLOCK_PARENT);

    printf("\n\n BLOCK_PARENT = 0\n\n\n");
    start_processes(PROGRAMS_COUNT, programs, !BLOCK_PARENT);

    return 0;
}

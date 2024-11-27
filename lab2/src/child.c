#include <stdlib.h>

#ifdef _WIN32
#    include <windows.h>
#    define sleep(x) Sleep(x * 1000)
#else
#    include <unistd.h>
#    define sleep(x) sleep(x)
#endif

int main(int argc, char *argv[])
{
    if (argc > 1) {
        int sleep_time = atoi(argv[1]);
        int exit_code = sleep_time;
        sleep(sleep_time);
        exit(exit_code);
    }
    sleep(3);
    exit(0);
}

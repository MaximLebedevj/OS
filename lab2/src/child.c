#include <stdlib.h> // atoi, exit

#ifdef _WIN32
#    include <windows.h> // Sleep
#    define sleep(x) Sleep(x * 1000)
#else
#    include <unistd.h>
#    define sleep(x) sleep(x)
#endif

int main(int argc, char *argv[]) {

    if (argc > 1) {
        int num = atoi(argv[1]);
        sleep(num);
        exit(num);
    }

    sleep(3);

    exit(0);
}

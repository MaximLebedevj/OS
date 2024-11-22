#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void main(int argc, char *argv[]) {

    sleep(1);

    if (argc > 1) {
        int num = atoi(argv[1]);
        exit(num);
    }

    exit(0);
}

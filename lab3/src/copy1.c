#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#    include <windows.h>
#    define SemaphoreWait(sem) WaitForSingleObject(sem, INFINITE)
#    define SemaphorePost(sem) ReleaseSemaphore(sem, 1, NULL)
#    define GetPid() GetCurrentProcessId()
typedef DWORD thread_id;
#else
#    include <unistd.h>
#    include <sys/mman.h>
#    include <fcntl.h>
#    include <stdlib.h>
#    include <semaphore.h>
#    include <time.h>
#    include <sys/time.h>
#    include <string.h>
#    define SemaphoreWait(sem) sem_wait(sem)
#    define SemaphorePost(sem) sem_post(sem)
#    define GetPid() getpid()
typedef pid_t thread_id;
#endif

#pragma pack(push, 1)
struct shr_data {
    int64_t count;
    uint8_t thr_finished;
    thread_id parent_pid;
};
#pragma pack(pop)

// get current time in YYYY-MM-DD hh:mm:ss.sss
char *get_time(char *date, size_t size)
{
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(date, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        st.wYear, st.wMonth, st.wDay, st.wHour,
        st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    struct tm *tmp;

    if (gettimeofday(&tv, NULL) != 0) {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }
    tmp = localtime(&tv.tv_sec);
    if (tmp == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    if (strftime(date, size, "%Y-%m-%d %H:%M:%S", tmp) == 0) {
        perror("strftime");
        exit(EXIT_FAILURE);
    }

    char ms[5];
    sprintf(ms, ".%03ld", tv.tv_usec / 1000);
    strcat(date, ms);
#endif
    return date;
}

#ifdef _WIN32
void WriteLog(HANDLE hFile, const char *str)
{
    DWORD bytesWritten;
    BOOL result = WriteFile(hFile, str, strlen(str), &bytesWritten, NULL);
    if (!result) {
        perror("WriteFile failed");
        exit(EXIT_FAILURE);
    }
}
#endif

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("argc < 3\n");
        exit(EXIT_FAILURE);
    }

    // redirecting stdout to a file using freopen
#ifdef _WIN32
    HANDLE log_file = CreateFile(
        argv[0],
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
#else
    if (freopen(argv[1], "a", stdout) == NULL) {
        perror("freopen");
        exit(EXIT_FAILURE);
    }
    // turn off stdout buffer
    setvbuf(stdout, NULL, _IONBF, 0);
#endif

    // open shared-memory
#ifdef _WIN32
    HANDLE shm = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, argv[1]);
    if (shm == NULL) {
        printf("CreateFileMapping failed: %lu\n", GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    int shm = shm_open(argv[2], O_RDWR, 0);
    if (shm == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
#endif

    // mmap to process address
#ifdef _WIN32
    size_t shm_size = sizeof(struct shr_data);
    struct shr_data *data = (struct shr_data *)MapViewOfFile(
        shm,
        FILE_MAP_READ | FILE_MAP_WRITE,
        0,
        0,
        shm_size
    );
    if (data == NULL) {
        printf("MapViewOfFile failed: %lu\n", GetLastError());
        CloseHandle(shm);
        exit(EXIT_FAILURE);
    }
#else
    size_t shm_size = sizeof(struct shr_data);
    struct shr_data *data;
    data = mmap(0, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0 );
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
#endif

    // open semaphore
#ifdef _WIN32
    HANDLE sem = OpenSemaphore(
       SEMAPHORE_ALL_ACCESS,
       FALSE,
       argv[2]
    );
#else
    sem_t *sem = sem_open(argv[3], O_RDWR);
#endif

    char curr_time[24];

#ifdef _WIN32
    char log_message[256];
#endif

    get_time(curr_time, sizeof(curr_time));

#ifdef _WIN32
    snprintf(log_message, sizeof(log_message), "[%s] copy1   pid %lu, STARTED\n", curr_time, GetPid());
    WriteLog(log_file, log_message);
#else
    printf("[%s] copy1   pid %d, STARTED\n", curr_time, GetPid());
#endif

    SemaphoreWait(sem);
    data->count += 10;
    SemaphorePost(sem);

    get_time(curr_time, sizeof(curr_time));

#ifdef _WIN32
    snprintf(log_message, sizeof(log_message), "[%s] copy1   pid %lu, EXITED\n", curr_time, GetPid());
    WriteLog(log_file, log_message);
#else
    printf("[%s] copy1   pid %d, EXITED\n", curr_time, GetPid());
#endif

    SemaphoreWait(sem);
    data->thr_finished++;
    SemaphorePost(sem);

    return 0;
}
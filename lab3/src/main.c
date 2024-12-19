#include <stdio.h>
#include <stdint.h>

#ifdef _WIN32
#    include <windows.h>
#    include <conio.h>
#    define COPY1_NAME "copy1.exe"
#    define COPY2_NAME "copy2.exe"
#    define SemaphoreWait(sem) WaitForSingleObject(sem, INFINITE)
#    define SemaphorePost(sem) ReleaseSemaphore(sem, 1, NULL)
#    define GetPid() GetCurrentProcessId()
#    define ExitThread() ExitThread(0)
#    define sleep_ms(ms) Sleep(ms)
typedef DWORD thread_id;
#else
#    include <unistd.h>
#    include <stdlib.h>
#    include <sys/time.h>
#    include <string.h>
#    include <sys/mman.h>
#    include <fcntl.h>
#    include <errno.h>
#    include <semaphore.h>
#    include <pthread.h>
#    include <signal.h>
#    include <limits.h>
#    define MAX_LEN NAME_MAX
#    define COPY1_NAME "copy1"
#    define COPY2_NAME "copy2"
#    define SemaphoreWait(sem) sem_wait(sem)
#    define SemaphorePost(sem) sem_post(sem)
#    define GetPid() getpid()
#    define ExitThread() pthread_exit(NULL);
#    define sleep_ms(ms) usleep(ms * 1000)
typedef pid_t thread_id;
#endif

#define LOG_FILE_NAME "log.txt"
#define SHARED_MEMORY_OBJECT_NAME "/my_shared_memory"
#define SEMAPHORE_OBJECT_NAME "/my_semaphore"

#pragma pack(push, 1)
struct shr_data {
    int64_t count;
    uint8_t thr_finished;
    thread_id parent_pid;
};
#pragma pack(pop)

struct thread_data {
    struct shr_data *thr_data;
#ifdef _WIN32
    HANDLE *thr_sem;
#else
    sem_t *thr_sem;
#endif
};

struct thread_data_print {
    struct shr_data *thr_data;
#ifdef _WIN32
    HANDLE *thr_sem;
    HANDLE *file;
#else
    sem_t *thr_sem;
    FILE *file;
#endif
};

#ifndef _WIN32
struct Program {
    const char prog_name[MAX_LEN + 1];
    char *arv[100];
};
#endif

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

volatile unsigned char need_exit = 0;
#ifdef _WIN32
BOOL WINAPI console_handler(DWORD signal)
{
    if (signal == CTRL_C_EVENT) {
        need_exit = 1;
        return TRUE;
    }
    return FALSE;
}
#else
void sig_handler(int sig)
{
    if (sig == SIGINT)
        need_exit = 1;
}
#endif

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

int is_process_alive(thread_id pid)
{
#ifdef _WIN32
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process == NULL) {
        return 0;
    }

    DWORD exit_code;
    if (GetExitCodeProcess(process, &exit_code)) {
        if (exit_code == STILL_ACTIVE) {
            CloseHandle(process);
            return 1;
        }
    }
    CloseHandle(process);
#else
    if (kill(pid, 0) == 0) {
        return 1; // process exists
    }
    if (errno == ESRCH) {
        return 0;
    }
#endif
    return 0;
}

#ifdef _WIN32
DWORD WINAPI thr_routine_300ms(void *args)
#else
void* thr_routine_300ms(void *args)
#endif
{
    struct thread_data *thr_params = (struct thread_data*)args;
    while (!need_exit) {
        sleep_ms(300); // 300 ms
        SemaphoreWait(thr_params->thr_sem);
        thr_params->thr_data->count++;
        SemaphorePost(thr_params->thr_sem);

        SemaphoreWait(thr_params->thr_sem);
        int is_parent_alive = is_process_alive(thr_params->thr_data->parent_pid);
        SemaphorePost(thr_params->thr_sem);

        if (!is_parent_alive || need_exit) {
            ExitThread();
        }
    }
    return 0;
}

#ifdef _WIN32
DWORD WINAPI thr_routine_cli(LPVOID args)
{
    struct thread_data *thr_params = (struct thread_data*)args;

    while (!need_exit) {
        if (need_exit) {
            ExitThread();
        }

        if (_kbhit()) {
            SemaphoreWait(thr_params->thr_sem);
            scanf("%lld", &thr_params->thr_data->count);
            SemaphorePost(thr_params->thr_sem);
        }

        SemaphoreWait(thr_params->thr_sem);
        int is_parent_alive = is_process_alive(thr_params->thr_data->parent_pid);
        SemaphorePost(thr_params->thr_sem);

        if (!is_parent_alive || need_exit) {
            ExitThread();
        }
        sleep_ms(50);  // 50 ms
    }

    return 0;
}
#else
void* thr_routine_cli(void *args)
{
    struct thread_data *thr_params = (struct thread_data*)args;

    int retval;
    fd_set readfds;
    struct timeval timeout;

    while (!need_exit) {

        if (need_exit) {
            ExitThread();
        }

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        timeout.tv_sec = 0;
        timeout.tv_usec = 400 * 1000; // 400 ms

        retval = select(1, &readfds, NULL, NULL, &timeout);
        if (retval > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            SemaphoreWait(thr_params->thr_sem);
            scanf("%lld", &thr_params->thr_data->count);
            SemaphorePost(thr_params->thr_sem);
            sleep_ms(400); // 400 ms
        }

        SemaphoreWait(thr_params->thr_sem);
        int is_parent_alive = is_process_alive(thr_params->thr_data->parent_pid);
        SemaphorePost(thr_params->thr_sem);

        if (!is_parent_alive || need_exit) {
            ExitThread();
        }
    }
    return 0;
}
#endif

#ifdef _WIN32
DWORD WINAPI thr_routine_print(void *args)
{
    struct thread_data_print *thr_params = (struct thread_data_print*)args;

    char curr_time[24];

    while (!need_exit) {
        Sleep(1000);
        get_time(curr_time, sizeof(curr_time));

        char log_message[256];
        snprintf(log_message, sizeof(log_message), "[%s] process pid %d, counter = %lld\n",
                 curr_time, GetCurrentProcessId(), thr_params->thr_data->count);

        SemaphoreWait(thr_params->thr_sem);
        WriteLog(thr_params->file, log_message);
        SemaphorePost(thr_params->thr_sem);

        if (need_exit) {
            ExitThread();
        }
    }
    return 0;
}
#else
void* thr_routine_print(void *args)
{
    struct thread_data_print *thr_params = (struct thread_data_print*)args;

    char curr_time[24];

    while (!need_exit) {
        sleep(1);
        get_time(curr_time, sizeof(curr_time));
        SemaphoreWait(thr_params->thr_sem);
        fprintf(thr_params->file, "[%s] process pid %d, counter = %lld\n", curr_time, getpid(), thr_params->thr_data->count);
        SemaphorePost(thr_params->thr_sem);

        if (need_exit) {
            pthread_exit(NULL);
        }
    }
    return 0;
}
#endif

#ifndef _WIN32
void free_resources(FILE *file)
{
    shm_unlink(SHARED_MEMORY_OBJECT_NAME);
    sem_unlink(SEMAPHORE_OBJECT_NAME);
    fclose(file);
}
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    HANDLE hFile = INVALID_HANDLE_VALUE;
    STARTUPINFO si;
    PROCESS_INFORMATION pi1, pi2;
    char log_message[256];
#endif

    char curr_time[24];

    // signal CTRL-C
#ifdef _WIN32
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        printf("SetConsoleCtrlHandler failed: %d\n", GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = sig_handler;
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    act.sa_mask = set;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
#endif

    // open log file
#ifdef _WIN32
    HANDLE log_file = CreateFile(
        LOG_FILE_NAME,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
#else
    FILE *log_file = fopen(LOG_FILE_NAME, "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    setvbuf(log_file, NULL, _IONBF, 0);
#endif

    // create shared-memory
    int first_run = 0; // the first execution of the program flag
    size_t shm_size = sizeof(struct shr_data);
#ifdef _WIN32
    HANDLE shm = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, SHARED_MEMORY_OBJECT_NAME);
    if (shm == NULL) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {

            first_run = 1;

            shm = CreateFileMapping(
                hFile,
                NULL,
                PAGE_READWRITE,
                0,
                shm_size,
                SHARED_MEMORY_OBJECT_NAME
            );

            if (shm == NULL) {
                printf("CreateFileMapping failed: %d\n", GetLastError());
                exit(EXIT_FAILURE);
            }
        } else {
            printf("OpenFileMapping failed: %d\n", GetLastError());
        }
    }
#else
    int shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_CREAT | O_EXCL | O_RDWR, 0777);
    if (shm == -1) {
        if (errno == EEXIST) {
            shm = shm_open(SHARED_MEMORY_OBJECT_NAME, O_RDWR, 0777);
            if (shm == -1) {
                perror("shm_open existing");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("shm_open");
            exit(EXIT_FAILURE);
        }
    } else {
        first_run = 1;
        // allocate memory for shared_memory object
        if (ftruncate(shm, shm_size) == -1) {
            perror("ftruncate");
            exit(EXIT_FAILURE);
        }
    }
#endif

    // mmap to process address
#ifdef _WIN32
    struct shr_data *data = (struct shr_data *)MapViewOfFile(
        shm,
        FILE_MAP_READ | FILE_MAP_WRITE,
        0,
        0,
        shm_size
    );
    if (data == NULL) {
        printf("MapViewOfFile failed: %d\n", GetLastError());
        CloseHandle(shm);
        exit(EXIT_FAILURE);
    }
#else
    struct shr_data *data;
    data = mmap(0, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm, 0 );
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
#endif

    // open or create new semaphore
#ifdef _WIN32
    HANDLE sem;
    if (first_run) {
        sem = CreateSemaphore(
            NULL,
            1,
            1,
            SEMAPHORE_OBJECT_NAME
        );
    } else {
        sem = OpenSemaphore(
            SEMAPHORE_ALL_ACCESS,
            FALSE,
            SEMAPHORE_OBJECT_NAME
        );
    }
#else
    sem_t *sem = sem_open(SEMAPHORE_OBJECT_NAME, O_CREAT, 0777, 1);
#endif

    // get first-process pid
    if (first_run) {
        SemaphoreWait(sem);
        data->parent_pid = GetPid();
        SemaphorePost(sem);
    }

    thread_id main_pid;
    SemaphoreWait(sem);
    main_pid = data->parent_pid;
    SemaphorePost(sem);

    if (GetPid() == main_pid) {
        // init shared data
        SemaphoreWait(sem);
        data->count = 0;
        data->thr_finished = 0;
        SemaphorePost(sem);

        // get current time in YYYY-MM-DD hh:mm:ss.sss
        get_time(curr_time, sizeof(curr_time));

        // starting message
#ifdef _WIN32
        WriteLog(log_file, "\n-- STARTED --\n");
        snprintf(log_message, sizeof(log_message), "[%s] process pid %d\n", curr_time, GetCurrentProcessId());
        WriteLog(log_file, log_message);
#else
        fprintf(log_file, "\n-- STARTED --\n");
        fprintf(log_file, "[%s] process pid %d\n", curr_time, getpid());
#endif
    }

    // create new thread (change counter 300ms)
    struct thread_data params = {data, sem};
#ifdef _WIN32
    HANDLE thr_300ms = CreateThread(
        NULL,
        0,
        thr_routine_300ms,
        &params,
        0,
        NULL
    );
    if (thr_300ms == NULL) {
        printf("CreateThread failed: %d\n", GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    pthread_t thr_300ms;
    int status = pthread_create(&thr_300ms, NULL, thr_routine_300ms, &params);
    if (status != 0) {
        perror("pthread_create (thr_300ms)");
        free_resources(log_file);
        exit(EXIT_FAILURE);
    }
#endif

    // create new thread (cli scanf)
#ifdef _WIN32
    HANDLE thr_cli = CreateThread(
        NULL,
        0,
        thr_routine_cli,
        &params,
        0,
        NULL
    );
    if (thr_cli == NULL) {
        printf("CreateThread failed: %d\n", GetLastError());
        exit(EXIT_FAILURE);
    }
#else
    pthread_t thr_cli;
    status = pthread_create(&thr_cli, NULL, thr_routine_cli, &params);
    if (status != 0) {
        perror("pthread_create (thr_cli)");
        free_resources(log_file);
        exit(EXIT_FAILURE);
    }
#endif

    if (GetPid() == main_pid) {
        // create new thread (print to file)
        struct thread_data_print params_print = {data, sem, log_file};
#ifdef _WIN32
        HANDLE thr_print = CreateThread(
            NULL,
            0,
            thr_routine_print,
            &params_print,
            0,
            NULL
        );
        if (thr_print == NULL) {
            printf("CreateThread failed: %d\n", GetLastError());
            exit(EXIT_FAILURE);
        }
#else
        pthread_t thr_print;
        status = pthread_create(&thr_print, NULL, thr_routine_print, &params_print);
        if (status != 0) {
            perror("pthread_create (thr_print)");
            free_resources(log_file);
            exit(EXIT_FAILURE);
        }
#endif
        // create copy1, copy2
        uint8_t finished = 0;
        int startup_count = 0;
#ifdef _WIN32
        char cmdLine[256];
        snprintf(cmdLine, sizeof(cmdLine), "%s %s %s",
                 LOG_FILE_NAME, SHARED_MEMORY_OBJECT_NAME, SEMAPHORE_OBJECT_NAME);

        while (!need_exit) {

            Sleep(3000);

            SemaphoreWait(sem);
            finished = data->thr_finished;
            SemaphorePost(sem);

            if (startup_count++ == 0 || finished == 2) {

                SemaphoreWait(sem);
                data->thr_finished = 0;
                SemaphorePost(sem);

                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi1, sizeof(pi1));
                ZeroMemory(&pi2, sizeof(pi2));

                CreateProcess(
                    COPY1_NAME,
                    cmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi1
                    );

                CreateProcess(
                    COPY2_NAME,
                    cmdLine,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi2);

                HANDLE processes[2] = {pi1.hProcess, pi2.hProcess};
                WaitForMultipleObjects(2, processes, TRUE, INFINITE);
            } else {
                char log_message[256];
                snprintf(log_message, sizeof(log_message), "[%s] process pid %d: copy1 or copy2 NOT FINISHED\n", curr_time, GetCurrentProcessId());
                WriteLog(log_file, log_message);
            }

        }
        WaitForSingleObject(thr_print, INFINITE);
        CloseHandle(thr_print);
#else
        struct Program programs[2] = {{"copy1", {"copy1", LOG_FILE_NAME, SHARED_MEMORY_OBJECT_NAME, SEMAPHORE_OBJECT_NAME, NULL}},
                                    {"copy2", {"copy2", LOG_FILE_NAME, SHARED_MEMORY_OBJECT_NAME, SEMAPHORE_OBJECT_NAME, NULL}}};
        while (!need_exit) {

            sleep(3);

            SemaphoreWait(sem);
            finished = data->thr_finished;
            SemaphorePost(sem);

            if (startup_count++ == 0 || finished == 2) {  // check if first startup or copy1 and copy2 are finished

                SemaphoreWait(sem);
                data->thr_finished = 0;
                SemaphorePost(sem);

                for (int i = 0; i < 2; ++i) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        if (execv(programs[i].prog_name, programs[i].arv) == -1) {
                            perror("execv");
                            free_resources(log_file);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            } else {  // if copy1 copy2 are not finished yet -- print to log
                fprintf(log_file, "[%s] process pid %d: copy1 or copy2 NOT FINISHED\n", curr_time, getpid());
            }
        }
        pthread_join(thr_print, NULL);
#endif
    }

#ifdef _WIN32
    // wait for thread to stop
    WaitForSingleObject(thr_300ms, INFINITE);
    CloseHandle(thr_300ms);
    WaitForSingleObject(thr_cli, INFINITE);
    CloseHandle(thr_cli);

    // close
    if (GetPid() == main_pid) {
        UnmapViewOfFile(data);
        CloseHandle(log_file);
    }
    CloseHandle(shm);
    CloseHandle(sem);
    CloseHandle(pi1.hProcess);
    CloseHandle(pi1.hThread);
    CloseHandle(pi2.hProcess);
    CloseHandle(pi2.hThread);
#else
    // wait for thread to stop
    pthread_join(thr_300ms, NULL);
    pthread_join(thr_cli, NULL);

    // munmap from process adress
    if (munmap(data, shm_size) == -1) {
        perror("munmap");
        free_resources(log_file);
        exit(EXIT_FAILURE);
    }

    if (GetPid() == main_pid) {
        // unlink shared_memory and semaphore, close file
        free_resources(log_file);
    }
#endif
    return 0;
}



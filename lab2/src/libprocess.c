#include <stdio.h>

#ifdef WIN32
#    include <windows.h>

#    define MAX_LEN MAX_PATH
#    define sleep(x) Sleep(x * 1000)

typedef CRITICAL_SECTION rc_mutex;
typedef HANDLE rc_thread;
typedef DWORD rc_thread_id;
#    define init_mutex(mutex) InitializeCriticalSection(mutex)
#    define delete_mutex(mutex) DeleteCriticalSection(mutex)
#    define lock_mutex(mutex) EnterCriticalSection(mutex)
#    define unlock_mutex(mutex) LeaveCriticalSection(mutex)
#else
#    include <limits.h>
#    include <pthread.h>
#    include <stdlib.h>
#    include <sys/wait.h>
#    include <unistd.h>

#    define MAX_LEN NAME_MAX
#    define sleep(x) sleep(x)

typedef pthread_mutex_t rc_mutex;
typedef pthread_t rc_thread;
typedef pthread_t rc_thread_id;
#    define init_mutex(mutex) rc_mutex mutex = PTHREAD_MUTEX_INITIALIZER
#    define delete_mutex(mutex) pthread_mutex_destroy(mutex)
#    define lock_mutex(mutex) pthread_mutex_lock(mutex)
#    define unlock_mutex(mutex) pthread_mutex_unlock(mutex)

#endif

#ifdef WIN32
typedef struct {
    PROCESS_INFORMATION pi;
    rc_mutex *mutex;
} MonitorParams;
#endif

struct Program {
    char prog_name[MAX_LEN + 1];
    char *arv[10];
};

#ifdef WIN32
char **GetWinCmd(struct Program programs[], int prog_count, int prog_index)
{
    char extension[] = ".exe";

    char **cmd = (char **)malloc(2 * sizeof(char *));

    cmd[0] = (char *)malloc(strlen(programs[prog_index].prog_name) +
                            strlen(extension) + 1);

    strcpy(cmd[0], programs[prog_index].prog_name);
    strcat(cmd[0], extension);

    int args_total_len = 0;
    int args_count = -2;

    for (int i = 0; i < 10; ++i) {
        args_count++;
        if (programs[prog_index].arv[i] == NULL)
            break;
        args_total_len += strlen(programs[prog_index].arv[i]) + 1;
    }

    cmd[1] = (char *)malloc(args_total_len);

    strcpy(cmd[1], cmd[0]);

    for (int i = 1; i <= args_count; ++i) {
        strcat(cmd[1], " ");
        strcat(cmd[1], programs[prog_index].arv[i]);
    }

    return cmd;
}
#endif

int children_exited = 0;

#ifdef WIN32
rc_thread_id WINAPI handle_child(LPVOID param)
{
    printf(" THREAD is handling child processes...\n");

    MonitorParams *params = (MonitorParams *)param;
    PROCESS_INFORMATION pi = params->pi;
    rc_mutex *mutex = params->mutex;

    while (1) {

        rc_thread_id waitResult = WaitForSingleObject(pi.hProcess, INFINITE);

        if (waitResult == WAIT_OBJECT_0) {

            rc_thread_id exitCode;

            if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
                printf("GetExitCodeProcess failed (%d). \n", GetLastError());
            }

            printf(" CHILD with PID %d exited with code %d\n", pi.dwProcessId,
                   exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            lock_mutex(mutex);
            children_exited++;
            unlock_mutex(mutex);

            free(params);
            break;
        }
        else {
            printf(" Error waiting: %lu\n", GetLastError());
            break;
        }
    }

    return 0;
}
#else

int total_children = 0;

init_mutex(mutex);

void *handle_child(void *args)
{
    printf(" THREAD is handling child processes...\n");

    while (1) {

        int status;
        pid_t pid = waitpid(-1, &status, 0);

        if (pid > 0) {
            if (WIFEXITED(status)) {
                printf(" CHILD with PID %d exited with code %d\n", pid, WEXITSTATUS(status));

                lock_mutex(&mutex);
                children_exited++;
                unlock_mutex(&mutex);
            }
        }
        else if (pid == -1) {
            lock_mutex(&mutex);
            if (children_exited >= total_children) {
                unlock_mutex(&mutex);
                break;
            }
            unlock_mutex(&mutex);
            sleep(1);
        }
    }

    return 0;
}
#endif

void start_processes(int prog_count, struct Program programs[],
                      int block_parent) {
#ifdef WIN32

    rc_mutex mutex;

    if (!block_parent)
        init_mutex(&mutex);

    STARTUPINFO si;
    PROCESS_INFORMATION pi[prog_count];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    rc_thread threadHandles[prog_count];

    for (int i = 0; i < prog_count; ++i) {
        char **cmd = GetWinCmd(programs, prog_count, i);

        if (!CreateProcess(cmd[0], cmd[1], NULL, NULL, FALSE, 0, NULL, NULL,
                           &si, &pi[i]))
        {
            printf(" CreateProcess failed (%d). \n", GetLastError());
            exit(1);
        }
        else {
            printf(" PARENT with PID %d created CHILD with PID %d\n",
                   GetCurrentProcessId(), pi[i].dwProcessId);

            free(cmd[0]);
            free(cmd[1]);
        }

        if (block_parent) {

            WaitForSingleObject(pi[i].hProcess, INFINITE);
            rc_thread_id exitCode;

            if (!GetExitCodeProcess(pi[i].hProcess, &exitCode)) {
                printf("GetExitCodeProcess failed (%d). \n", GetLastError());
                exit(1);
            }

            printf(" CHILD with PID %d, PPID %d exited with code %d\n\n",
                   pi[i].dwProcessId, GetCurrentProcessId(), exitCode);
        }
        else {
            MonitorParams *params = (MonitorParams *)malloc(sizeof(MonitorParams));
            params->pi = pi[i];
            params->mutex = &mutex;

            threadHandles[i] = CreateThread(NULL, 0, handle_child, params, 0, NULL);

            if (threadHandles[i] == NULL) {
                printf(" Error creating thread: %d\n", GetLastError());
                free(params);
                exit(1);
            }
        }
    }
#else

    total_children = prog_count;

    rc_thread thread;

    int status;
    int status_addr;

    pid_t pid;
    pid_t parent_pid = getpid();

    if (!block_parent) {

        int status;

        status = pthread_create(&thread, NULL, handle_child, NULL);

        if (status != 0) {
            perror("main ");
        }

        status = pthread_detach(thread);

        if (status != 0) {
            perror("pthread_detach");
            exit(1);
        }
    }

    for (int i = 0; i < prog_count; ++i) {
        if (getpid() == parent_pid) {

            pid = fork();

            if (pid == 0)
                execv(programs[i].prog_name, programs[i].arv);

            else if (getpid() == parent_pid) {
                printf(" PARENT with PID %d created CHILD with PID %d\n",
                       getpid(), pid);

                if (block_parent) {
                    waitpid(pid, &status, 0);
                    printf(
                        " CHILD with PID %d, PPID %d exited with code %d\n\n",
                        pid, getpid(), WEXITSTATUS(status));
                }
            }
            else {
                perror("fork");
                exit(1);
            }
        }
    }
#endif

    // PARENT-process code block

    for (int i = 0; i < 7; ++i) {
        printf(" NON-CHILD process doing some work...\n");
        sleep(1);
    }

    // PARENT-process code block

    if (!block_parent) {
        while (1) {
            lock_mutex(&mutex);
            if ( children_exited >= prog_count) {
                unlock_mutex(&mutex);
                delete_mutex(&mutex);
                break;
            }
            unlock_mutex(&mutex);
            sleep(1);
        }
    }
}

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#else
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef WIN32
#define MAX_LEN MAX_PATH
#define sleep(x) Sleep(x * 1000)
#else
#define MAX_LEN NAME_MAX
#define sleep(x) sleep(x)
#endif

#ifdef WIN32
typedef struct {
    PROCESS_INFORMATION pi;
    CRITICAL_SECTION *cs;
} MonitorParams;
#endif

struct Program {
    char prog_name[MAX_LEN + 1];
    char *arv[10];
};

#ifdef WIN32
char **GetWinCmd(struct Program programs[], int prog_count, int index) {
    char extension[] = ".exe";

    char **cmd = (char **)malloc(2 * sizeof(char *));

    cmd[0] = (char *)malloc(strlen(programs[index].prog_name) +
                            strlen(extension) + 1);

    strcpy(cmd[0], programs[index].prog_name);
    strcat(cmd[0], extension);

    int argsLen = 0;
    int argsCount = -2;

    for (int i = 0; i < 10; ++i) {
        argsCount++;
        if (programs[index].arv[i] == NULL)
            break;
        argsLen += strlen(programs[index].arv[i]) + 1;
    }

    cmd[1] = (char *)malloc(argsLen);

    strcpy(cmd[1], cmd[0]);

    for (int i = 1; i <= argsCount; ++i) {
        strcat(cmd[1], " ");
        strcat(cmd[1], programs[index].arv[i]);
    }

    return cmd;
}
#endif

int children_exited = 0;

#ifdef WIN32
DWORD WINAPI handle_child(LPVOID param) {
    MonitorParams *params = (MonitorParams *)param;
    PROCESS_INFORMATION pi = params->pi;
    CRITICAL_SECTION *cs = params->cs;

    while (1) {
        DWORD waitResult = WaitForSingleObject(pi.hProcess, INFINITE);
        if (waitResult == WAIT_OBJECT_0) {

            DWORD exitCode;
            if (!GetExitCodeProcess(pi.hProcess, &exitCode)) {
                printf("GetExitCodeProcess failed (%d). \n", GetLastError());
            }

            printf(" CHILD with PID %d exited with code %d\n", pi.dwProcessId,
                   exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            EnterCriticalSection(cs);
            children_exited++;
            LeaveCriticalSection(cs);

            free(params);
            break;
        } else {
            printf(" Error waiting: %lu\n", GetLastError());
            break;
        }
    }

    return 0;
}
#else
int total_children = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Мьютекс для синхронизации

void *handle_child(void *args) {

    printf(" THREAD is handling child processes...\n");

    while (1) {
        int status;
        pid_t pid = waitpid(-1, &status, 0);

        if (pid > 0) {
            if (WIFEXITED(status)) {
                printf(" CHILD with PID %d exited with code %d\n", pid, WEXITSTATUS(status));
                pthread_mutex_lock(&mutex);
                children_exited++;
                pthread_mutex_unlock(&mutex);
            }
        }
        else if (pid == -1) {
            pthread_mutex_lock(&mutex);
            if (children_exited >= total_children) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }
    }

    return 0;
}
#endif

void create_processes(int prog_count, struct Program programs[],
                      int block_parent) {
#ifdef WIN32
    CRITICAL_SECTION cs;

    if (!block_parent)
        InitializeCriticalSection(&cs);

    HANDLE threadHandles[prog_count];

    STARTUPINFO si;
    PROCESS_INFORMATION pi[prog_count];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    for (int i = 0; i < prog_count; ++i) {
        char **cmd = GetWinCmd(programs, prog_count, i);

        if (!CreateProcess(cmd[0], cmd[1], NULL, NULL, FALSE, 0, NULL, NULL,
                           &si, &pi[i])) {
            printf(" CreateProcess failed (%d). \n", GetLastError());
            exit(1);
        } else {
            printf(" PARENT with PID %d created CHILD with PID %d\n",
                   GetCurrentProcessId(), pi[i].dwProcessId);
            free(cmd[0]);
            free(cmd[1]);
        }

        if (block_parent) {

            WaitForSingleObject(pi[i].hProcess, INFINITE);
            DWORD exitCode;
            if (!GetExitCodeProcess(pi[i].hProcess, &exitCode)) {
                printf("GetExitCodeProcess failed (%d). \n", GetLastError());
                exit(1);
            }
            printf(" CHILD with PID %d, PPID %d exited with code %d\n\n",
                   pi[i].dwProcessId, GetCurrentProcessId(), exitCode);
        } else {
            MonitorParams *params =
                (MonitorParams *)malloc(sizeof(MonitorParams));
            params->pi = pi[i];
            params->cs = &cs;

            threadHandles[i] =
                CreateThread(NULL, 0, handle_child, params, 0, NULL);
            if (threadHandles[i] == NULL) {
                printf(" Error creating thread: %d\n", GetLastError());
                free(params);
                exit(1);
            }
        }
    }
#else
    pthread_t thread;

    int status;
    int status_addr;

    total_children = prog_count;

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

            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            else if (pid == 0)
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
        }
    }
#endif

    // PARENT code block

    for (int i = 0; i < 7; ++i) {
        printf(" NON-CHILD process doing some work...\n");
        sleep(1);
    }

    // PARENT code block

#ifdef WIN32
    if (!block_parent) {
        while (1) {
            EnterCriticalSection(&cs);
            if ( children_exited >= prog_count) {
                break;
            }
            LeaveCriticalSection(&cs);
            sleep(1);
        }
    }

#else
    if (!block_parent) {

        while (1) {
            pthread_mutex_lock(&mutex);
            if (children_exited >= total_children) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            pthread_mutex_unlock(&mutex);
            sleep(1);
        }
    }
#endif
}

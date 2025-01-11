#include "sqlite3.h"
#include "serial.h"

#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#else
#    include <termios.h>
#    include <pthread.h>
#    include <semaphore.h>
#    include <string.h>
#    include <errno.h>
#    include <signal.h>
#    include <arpa/inet.h>
#    include <poll.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#ifdef _WIN32
#    define PORT_RD "COM9"
#else
#    define PORT_RD "/dev/pts/6"
#    define SOCKET int
#endif

#define SEC_IN_HOUR 3600
#define SEC_IN_DAY 86400

#define INTERFACE_IP "127.0.0.1"
#define PORT 8080
#define READ_WAIT_MS 50
#define MAX_CLIENTS 100

struct thr_data {
    #ifdef _WIN32
    HANDLE fd;
    #else
    int fd;
    #endif
    sqlite3 *db;
};

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

void handle_client(SOCKET client_socket)
{
    char buffer[1024];
    int read_size;

    read_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (read_size < 0) {
        perror("recv failed");
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }

    buffer[read_size] = '\0';

    char *action_value = NULL;
    char *client_type_value = NULL;
    char *header_line = strtok(buffer, "\r\n");
    while (header_line != NULL) {
        if (strncmp(header_line, "action:", strlen("action:")) == 0) {
            action_value = header_line + strlen("action: ");
        }
        else if (strncmp(header_line, "X-Client-Type:", strlen("X-Client-Type:")) == 0) {
            client_type_value = header_line + strlen("X-Client-Type: ");
        }
        header_line = strtok(NULL, "\r\n");
    }

    if (client_type_value != NULL && strcmp(client_type_value, "qt-app") == 0) {
        printf("CLIENT_TYPE: qt-app\n");

#ifdef _WIN32
        _putenv_s("REQUEST_URI", action_value);
        _putenv_s("CLIENT_TYPE", "qt-app");
#else
        setenv("REQUEST_URI", action_value, 1);
        setenv("CLIENT_TYPE", "qt-app", 1);
#endif

#ifdef _WIN32
        FILE *cgi = _popen("temp.cgi", "r");
#else
        FILE *cgi = popen("./temp.cgi", "r");
#endif
        if (cgi == NULL) {
            perror("Failed to run CGI script");
#ifdef _WIN32
            closesocket(client_socket);
#else
            close(client_socket);
#endif
            return;
        }

        char cgi_response[262144];
        char response_body[262144] = {0};
        while (fgets(cgi_response, sizeof(cgi_response), cgi) != NULL) {
            strcat(response_body, cgi_response);
        }
        fclose(cgi);

        char http_response[262144];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 strlen(response_body), response_body);

        send(client_socket, http_response, strlen(http_response), 0);

    } else {

        printf("CLIENT_TYPE: browser\n");

        char *uri_start = strstr(buffer, "GET ");
        if (!uri_start) {
#ifdef _WIN32
            closesocket(client_socket);
#else
            close(client_socket);
#endif
            return;
        }

        uri_start += 4;
        char *uri_end = strchr(uri_start, ' ');
        if (uri_end) {
            *uri_end = '\0';
        }

#ifdef _WIN32
        _putenv_s("REQUEST_URI", uri_start);
        _putenv_s("CLIENT_TYPE", "web");
#else
        setenv("REQUEST_URI", uri_start, 1);
        setenv("CLIENT_TYPE", "web", 1);
#endif

#ifdef _WIN32
        FILE *cgi = _popen("temp.cgi", "r");
#else
        FILE *cgi = popen("./temp.cgi", "r");
#endif
        if (cgi == NULL) {
            perror("Failed to run CGI script");
#ifdef _WIN32
            closesocket(client_socket);
#else
            close(client_socket);
#endif
            return;
        }

        char cgi_response[262144];
        char response_body[262144] = {0};
        while (fgets(cgi_response, sizeof(cgi_response), cgi) != NULL) {
            strcat(response_body, cgi_response);
        }

        fclose(cgi);

        char http_response[262144];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 strlen(response_body), response_body);

        send(client_socket, http_response, strlen(http_response), 0);
    }

#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif
}

void execute_sql(sqlite3 *db, const char *sql)
{
    char *err_msg = 0;
    int res = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Error: %s\n", err_msg);
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
}

void prepare_bind_step(sqlite3 *db, const char *sql, sqlite3_stmt **statement, double value, int index)
{
    char *err_msg = 0;
    int res = sqlite3_prepare_v2(db, sql, -1, statement, 0);
    if (res == SQLITE_OK) {
        sqlite3_bind_double(*statement, index, value);
        int step = sqlite3_step(*statement);
        if (step != SQLITE_DONE) {
            fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(*statement);
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    sqlite3_finalize(*statement);
}

#ifdef _WIN32
DWORD WINAPI thr_routine_db(void *args)
{
    struct thr_data *params = (struct thr_data*)args;

    char buffer[255];
    double cur_temp;

    time_t start_all = time(NULL);
    time_t start_hour = time(NULL);
    time_t start_day = time(NULL);

    char *sql;
    sqlite3_stmt *statement;

    while (!need_exit) {
        DWORD bytesRead;
        if (ReadFile(params->fd, buffer, sizeof(buffer), &bytesRead, NULL)) {
            if (bytesRead > 0) {
                cur_temp = atof(buffer);
                sql = "INSERT INTO temp_all (date, temp) VALUES (DATETIME('now', 'localtime'), ?)";
                prepare_bind_step(params->db, sql, &statement, cur_temp, 1);

                // delete old recors
                sql = "DELETE FROM temp_all "
                "WHERE date < DATETIME('now', 'localtime', '-1 day');";
                execute_sql(params->db, sql);

                // add record to temp_hour
                time_t now = time(NULL);
                if (difftime(now, start_hour) >= SEC_IN_HOUR) {
                    sql = "INSERT INTO temp_hour (date, avg_temp) "
                    "SELECT DATETIME('now', 'localtime'), "
                    "ROUND(AVG(temp), 1) FROM temp_all "
                    "WHERE date >= DATETIME('now', 'localtime', '-1 hour');";
                    execute_sql(params->db, sql);

                    // delete old records
                    sql = "DELETE FROM temp_hour "
                    "WHERE date < DATETIME('now', 'localtime', '-1 month');";
                    execute_sql(params->db, sql);

                    start_hour = now;
                }

                int db_year = -1;
                struct tm *tm_now = localtime(&now);
                int curr_year = tm_now->tm_year + 1900;

                // add record to temp_day
                if (difftime(now, start_day) >= SEC_IN_DAY) {
                    sql = "INSERT INTO temp_day (date, avg_temp) "
                    "SELECT DATETIME('now', 'localtime'), "
                    "ROUND(AVG(temp), 1) FROM temp_all "
                    "WHERE date >= DATETIME('now', 'localtime', '-1 day');";
                    execute_sql(params->db, sql);

                    if (curr_year != db_year) {
                        char *sql = "DELETE FROM temp_day WHERE strftime('%M', date) != strftime('%M', 'now');";
                        execute_sql(params->db, sql);
                        db_year = curr_year;
                    }

                    start_day = now;
                }
            }
        }
    }
    return 0;
}
#else
void* thr_routine_db(void *args)
{
    struct thr_data *params = (struct thr_data*)args;

    char buffer[255];
    double cur_temp;

    time_t start_all = time(NULL);
    time_t start_hour = time(NULL);
    time_t start_day = time(NULL);

    char *sql;
    sqlite3_stmt *statement;

    while (!need_exit) {
        ssize_t bytesRead = read(params->fd, buffer, sizeof(buffer));
        if (bytesRead > 0) {

            cur_temp = atof(buffer);
            sql = "INSERT INTO temp_all (date, temp) VALUES (DATETIME('now', 'localtime'), ?)";
            prepare_bind_step(params->db, sql, &statement, cur_temp, 1);

            // delete old recors
            sql = "DELETE FROM temp_all "
            "WHERE date < DATETIME('now', 'localtime', '-1 day');";
            execute_sql(params->db, sql);

            // add record to temp_hour
            time_t now = time(NULL);
            if (difftime(now, start_hour) >= SEC_IN_HOUR) {
                sql = "INSERT INTO temp_hour (date, avg_temp) "
                "SELECT DATETIME('now', 'localtime'), "
                "ROUND(AVG(temp), 1) FROM temp_all "
                "WHERE date >= DATETIME('now', 'localtime', '-1 hour');";
                execute_sql(params->db, sql);

                // delete old records
                sql = "DELETE FROM temp_hour "
                "WHERE date < DATETIME('now', 'localtime', '-1 month');";
                execute_sql(params->db, sql);

                start_hour = now;
            }

            int db_year = -1;
            struct tm *tm_now = localtime(&now);
            int curr_year = tm_now->tm_year + 1900;

            // add record to temp_day
            if (difftime(now, start_day) >= SEC_IN_DAY) {
                sql = "INSERT INTO temp_day (date, avg_temp) "
                "SELECT DATETIME('now', 'localtime'), "
                "ROUND(AVG(temp), 1) FROM temp_all "
                "WHERE date >= DATETIME('now', 'localtime', '-1 day');";
                execute_sql(params->db, sql);

                if (curr_year != db_year) {
                    char *sql = "DELETE FROM temp_day WHERE strftime('%M', date) != strftime('%M', 'now');";
                    execute_sql(params->db, sql);
                    db_year = curr_year;
                }

                start_day = now;
            }
        }
    }
    return 0;
}
#endif

int main(int argc, char *argv[])
{
    srand(time(0));

    // signal SIGINT
    #ifdef _WIN32
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        perror("Error setting handler");
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

    // configure port
    #ifdef _WIN32
    HANDLE fd = CreateFile(
        PORT_RD,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (fd == INVALID_HANDLE_VALUE) {
        perror("CreateFile (pd)");
        exit(EXIT_FAILURE);
    }

    if (!configure_port(fd, BAUDRATE_115200)) {
        perror("configure_port (pd)");
        CloseHandle(fd);
        exit(EXIT_FAILURE);
    }
    #else
    const char* port_rd = PORT_RD;
    speed_t baud_rate = BAUDRATE_115200;
    configure_port(port_rd, baud_rate);

    // open port
    int fd = open(port_rd, O_RDONLY | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("open port");
        exit(EXIT_FAILURE);
    }

    // flush port
    if (tcflush(fd, TCIFLUSH) == -1) {
        perror("tcflush");
        close(fd);
        exit(EXIT_FAILURE);
    }
    #endif

    sqlite3 *db;
    sqlite3_stmt *statement;
    char *sql;

    // open db
    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    // create table "temp_all"
    sql = "CREATE TABLE IF NOT EXISTS temp_all("
    "date DATETIME DEFAULT CURRENT_TIMESTAMP PRIMARY KEY,"
    "temp REAL"
    ");";
    execute_sql(db, sql);

    // create table "temp_hour"
    sql = "CREATE TABLE IF NOT EXISTS temp_hour("
    "date DATETIME DEFAULT CURRENT_TIMESTAMP PRIMARY KEY,"
    "avg_temp REAL"
    ");";
    execute_sql(db, sql);

    // create table "temp_day"
    sql = "CREATE TABLE IF NOT EXISTS temp_day("
    "date DATETIME DEFAULT CURRENT_TIMESTAMP PRIMARY KEY,"
    "avg_temp REAL"
    ");";
    execute_sql(db, sql);

    sql = "PRAGMA journal_mode = WAL;";
    res = sqlite3_exec(db, sql, 0, 0, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Error setting WAL mode: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    char buffer[255];
    double cur_temp;

    // create new thread (db_thread)
    struct thr_data params_db = {fd, db};
    #ifdef _WIN32
    HANDLE thr_db = CreateThread(
        NULL,
        0,
        thr_routine_db,
        &params_db,
        0,
        NULL);
    if (thr_db == NULL) {
        perror("CreateThread (thr_day)");
        exit(EXIT_FAILURE);
    }
    #else
    pthread_t db_thread;
    int status = pthread_create(&db_thread, NULL, thr_routine_db, &params_db);
    if (status != 0) {
        perror("pthread_create (thr_day)");
        exit(EXIT_FAILURE);
    }
    #endif

    // ---SERVER--- //

    #ifdef _WIN32
    WSADATA wsaData;
    #else
    // ignore SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);
    #endif

    SOCKET server_socket;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;

    #ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }
    #endif

    // create server_socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    // set server_socket parameters
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(INTERFACE_IP);
    server_addr.sin_port = htons(PORT);

    // bind server_socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        #ifdef _WIN32
        closesocket(server_socket);
        WSACleanup();
        #else
        close(server_socket);
        #endif
        return 1;
    }

    // start listening
    if (listen(server_socket, SOMAXCONN) < 0) {
        perror("Listen failed");
        #ifdef _WIN32
        closesocket(server_socket);
        WSACleanup();
        #else
        close(server_socket);
        #endif
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // init fds[i]
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        fds[i].fd = -1;
    }

    // fds[0] for listen socket
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    while (!need_exit) {
        int poll_count;
        #ifdef _WIN32
        poll_count = WSAPoll(fds, nfds, READ_WAIT_MS);
        #else
        poll_count = poll(fds, nfds, READ_WAIT_MS);
        #endif

        if (poll_count < 0) {
            perror("Failed to poll");
            break;
        }

        for (int i = 0; i < nfds; ++i) {
            // if have new connections
            if (fds[i].fd == server_socket && (fds[i].revents & POLLIN)) {
                struct sockaddr_in client_addr;
                #ifdef _WIN32
                int client_addr_len = sizeof(client_addr);
                #else
                socklen_t client_addr_len = sizeof(client_addr);
                #endif
                SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

                if (client_socket < 0) {
                    perror("Accept failed");
                    continue;
                }

                printf("New client connected\n");

                // add new client to fds;
                for (int j = 1; j < MAX_CLIENTS; ++j) {
                    if (fds[j].fd == -1) {
                        fds[j].fd = client_socket;
                        fds[j].events = POLLIN;
                        if (j >= nfds) {
                            nfds = j + 1;
                        }
                        break;
                    }
                }
            } else if (fds[i].fd > 0 && (fds[i].revents & POLLIN)) {
                handle_client(fds[i].fd);
                fds[i].fd = -1;
            }
        }
    }

    #ifdef _WIN32
    WaitForSingleObject(thr_db, INFINITE);
    CloseHandle(thr_db);
    closesocket(server_socket);
    WSACleanup();
    CloseHandle(fd);
    #else
    pthread_join(db_thread, NULL);
    close(server_socket);
    close(fd);
    #endif

    sqlite3_close(db);

    return 0;
}

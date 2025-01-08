#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include <string.h>

void print_html_header()
{
    printf("<html lang=\"en\">\n");
    printf("<head>\n");
    printf("<meta charset=\"UTF-8\">\n");
    printf("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    printf("<meta http-equiv=\"Refresh\" content=\"5\" />\n");
    printf("<title>Welcome to Temperature Dashboard</title>\n");
    printf("<style>\n");
    printf("    body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f4; }\n");
    printf("    header { background-color: #0078D7; color: white; padding: 20px; text-align: center; }\n");
    printf("    nav { background-color: #f2f2f2; padding: 10px; text-align: center; margin-bottom: 20px; }\n");
    printf("    nav a { margin: 0 15px; text-decoration: none; color: #0078D7; font-weight: bold; }\n");
    printf("    nav a:hover { text-decoration: underline; }\n");
    printf("    main { padding: 20px; text-align: center; }\n");
    printf("    footer { background-color: #333; color: white; text-align: center; padding: 10px; position: fixed; bottom: 0; width: 100%; }\n");
    printf("    .current-temp { font-size: 1.5em; font-weight: bold; margin-top: 20px; }\n");
    printf("    .navigation { margin-bottom: 20px; font-size: 1.1em; }\n");
    printf("    .navigation a { margin: 0 10px; color: #0078D7; text-decoration: none; padding: 8px 12px; border-radius: 5px; }\n");
    printf("    .navigation a.active { background-color: #0078D7; color: white; }\n");
    printf("    .navigation a:hover { text-decoration: underline; }\n");
    printf("    .container { max-width: 800px; margin: 0 auto; padding: 20px; background-color: white; border-radius: 8px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }\n");
    printf("    .footer-text { font-size: 0.9em; color: #bbb; }\n");
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
}

void print_html_navigation()
{
    printf("<nav class=\"navigation\">\n");
    printf("<a href=\"/secondly_1min\">Last 5 Minutes</a>\n");
    printf("<a href=\"/hourly_day\">Hourly Average</a>\n");
    printf("<a href=\"/daily_week\">Daily Average</a>\n");
    printf("</nav>\n");
}

void print_daily_navigation(const char *active_page)
{
    printf("<div class=\"navigation\">\n");
    printf("<a href=\"/daily_week\" class=\"%s\">week</a>\n", strcmp(active_page, "/daily_week") == 0 ? "active" : "");
    printf("<a href=\"/daily_month\" class=\"%s\">month</a>\n", strcmp(active_page, "/daily_month") == 0 ? "active" : "");
    printf("<a href=\"/daily_3month\" class=\"%s\">3 months</a>\n", strcmp(active_page, "/daily_3month") == 0 ? "active" : "");
    printf("<a href=\"/daily_6month\" class=\"%s\">6 months</a>\n", strcmp(active_page, "/daily_6month") == 0 ? "active" : "");
    printf("<a href=\"/daily_year\" class=\"%s\">year</a>\n", strcmp(active_page, "/daily_year") == 0 ? "active" : "");
    printf("</div>\n");
}

void print_hourly_navigation(const char *active_page)
{
    printf("<div class=\"navigation\">\n");
    printf("<a href=\"/hourly_day\" class=\"%s\">Day</a>\n", strcmp(active_page, "/hourly_day") == 0 ? "active" : "");
    printf("<a href=\"/hourly_week\" class=\"%s\">Week</a>\n", strcmp(active_page, "/hourly_week") == 0 ? "active" : "");
    printf("<a href=\"/hourly_month\" class=\"%s\">Month</a>\n", strcmp(active_page, "/hourly_month") == 0 ? "active" : "");
    printf("</div>\n");
}

void print_secondly_navigation(const char *active_page)
{
    printf("<div class=\"navigation\">\n");
    printf("<a href=\"/secondly_1min\" class=\"%s\">1 min</a>\n", strcmp(active_page, "/secondly_1min") == 0 ? "active" : "");
    printf("<a href=\"/secondly_5min\" class=\"%s\">5 min</a>\n", strcmp(active_page, "/secondly_5min") == 0 ? "active" : "");
    printf("</div>\n");
}

void print_html_footer()
{
    printf("</body>\n");
    printf("</html>\n");
}

void print_current_temperature()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql = "SELECT temp FROM temp_all ORDER BY date DESC LIMIT 1;";
    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    double curr_temp = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        curr_temp = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    printf("<div class=\"container\">\n");
    printf("<h1>Temperature Dashboard</h1>\n");
    printf("<p class=\"current-temp\">Current Temperature: %.1f &deg;C</p>\n", curr_temp);
    printf("</div>\n");
}

void print_daily_week(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d', date) AS date, "
    "           avg_temp "
    "    FROM temp_day "
    ") "
    "SELECT row_num, date, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 7;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Daily Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_daily_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *date = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", date);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_daily_month(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d', date) AS date, "
    "           avg_temp "
    "    FROM temp_day "
    ") "
    "SELECT row_num, date, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 30;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Daily Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_daily_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *date = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", date);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_daily_3month(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d', date) AS date, "
    "           avg_temp "
    "    FROM temp_day "
    ") "
    "SELECT row_num, date, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 90;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Daily Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_daily_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *date = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", date);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_daily_6month(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d', date) AS date, "
    "           avg_temp "
    "    FROM temp_day "
    ") "
    "SELECT row_num, date, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 180;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Daily Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_daily_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *date = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", date);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_daily_year(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d', date) AS date, "
    "           avg_temp "
    "    FROM temp_day "
    ") "
    "SELECT row_num, date, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 366;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Daily Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_daily_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *date = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", date);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_hourly_month_avg(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    ") "
    "SELECT row_num, datetime, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Hourly Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_hourly_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date and Time</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");


    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *datetime = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", datetime);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_hourly_day_avg(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    ") "
    "SELECT row_num, datetime, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 24;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Hourly Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_hourly_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date and Time</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");


    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *datetime = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", datetime);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_hourly_week_avg(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    ") "
    "SELECT row_num, datetime, avg_temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 168;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Hourly Average Temperature</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_hourly_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date and Time</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *datetime = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", datetime);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_secondly_minute(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           temp "
    "    FROM temp_all "
    ") "
    "SELECT row_num, datetime, temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 60;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Last Minute Temperature Records</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_secondly_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date and Time</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    int row_num = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *datetime = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num++);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", datetime);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void print_secondly_5minutes(const char *active_page)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *sql =
    "WITH numbered_data AS ("
    "    SELECT ROW_NUMBER() OVER (ORDER BY date DESC) AS row_num, "
    "           strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           temp "
    "    FROM temp_all "
    ") "
    "SELECT row_num, datetime, temp "
    "FROM numbered_data "
    "ORDER BY row_num "
    "LIMIT 300;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    printf("<div class=\"container\">\n");
    printf("<h2>Last 5 Minutes Temperature Records</h2>\n");
    printf("<table style=\"border-collapse: collapse; width: 100%;\">\n");

    print_secondly_navigation(active_page);

    printf("<thead><tr style=\"background-color: #0078D7; color: white;\">\n");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">#</th>");
    printf("<th style=\"text-align:left; padding: 10px; border: 1px solid #ddd;\">Date and Time</th>");
    printf("<th style=\"text-align:right; padding: 10px; border: 1px solid #ddd;\">Temperature (°C)</th>");
    printf("</tr></thead>\n");

    printf("<tbody>\n");

    int row_num = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int row_num = sqlite3_column_int(stmt, 0);
        const char *datetime = (const char *)sqlite3_column_text(stmt, 1);
        double temp = sqlite3_column_double(stmt, 2);

        printf("<tr>\n");
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%d</td>", row_num++);
        printf("<td style=\"padding: 8px; text-align:left; border: 1px solid #ddd;\">%s</td>", datetime);
        printf("<td style=\"padding: 8px; text-align:right; border: 1px solid #ddd;\">%.1f</td></tr>\n", temp);
    }

    printf("</tbody>\n");
    printf("</table>\n");
    printf("</div>\n");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main()
{
    print_html_header();

    print_current_temperature();

    print_html_navigation();

    char *request_uri = getenv("REQUEST_URI");
    if (request_uri == NULL || strcmp(request_uri, "/") == 0) {
    } else if (strcmp(request_uri, "/hourly_day") == 0) {
        print_hourly_day_avg(request_uri);
    } else if (strcmp(request_uri, "/hourly_week") == 0) {
        print_hourly_week_avg(request_uri);
    } else if (strcmp(request_uri, "/hourly_month") == 0) {
        print_hourly_month_avg(request_uri);
    } else if (strcmp(request_uri, "/secondly_1min") == 0) {
        print_secondly_minute(request_uri);
    } else if (strcmp(request_uri, "/secondly_5min") == 0) {
        print_secondly_5minutes(request_uri);
    } else if (strcmp(request_uri, "/daily_week") == 0) {
        print_daily_week(request_uri);
    } else if (strcmp(request_uri, "/daily_month") == 0) {
        print_daily_month(request_uri);
    } else if (strcmp(request_uri, "/daily_3month") == 0) {
        print_daily_3month(request_uri);
    } else if (strcmp(request_uri, "/daily_6month") == 0) {
        print_daily_6month(request_uri);
    } else if (strcmp(request_uri, "/daily_year") == 0) {
        print_daily_year(request_uri);
    }

    print_html_footer();

    return 0;
}

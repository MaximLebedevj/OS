#pragma once

#include <stdio.h>
#include "sqlite3.h"
#include <json-c/json.h>

void get_current_temp()
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

    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", curr_temp);

    struct json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "current_temp", json_object_new_string(tempStr));

    const char *json_string = json_object_to_json_string(response_json);

    printf("%s\n", json_string);

    json_object_put(response_json);
}

void get_hourly_day_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH hourly_data AS ("
    "    SELECT strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    "    WHERE date >= datetime('now', 'localtime', 'start of day') "
    "      AND date < datetime('now', 'localtime', 'start of day', '+1 day') "
    "    ORDER BY date ASC"
    ") "
    "SELECT datetime, avg_temp "
    "FROM hourly_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *hour = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATETIME", json_object_new_string(hour));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }
        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_hourly_weekly_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH weekly_data AS ("
    "    SELECT strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    "    WHERE date >= datetime('now', 'localtime', '-7 days') "
    "    ORDER BY date ASC"
    ") "
    "SELECT datetime, avg_temp "
    "FROM weekly_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *hour = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATETIME", json_object_new_string(hour));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }

        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_hourly_month_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH monthly_data AS ("
    "    SELECT strftime('%Y-%m-%d %H:%M:%S', date) AS datetime, "
    "           avg_temp "
    "    FROM temp_hour "
    "    WHERE date >= datetime('now', 'localtime', '-30 days') "
    "    ORDER BY date ASC"
    ") "
    "SELECT datetime, avg_temp "
    "FROM monthly_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *hour = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATETIME", json_object_new_string(hour));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }

        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_daily_week_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH daily_data AS ("
    "    SELECT strftime('%Y-%m-%d', date) AS date, "
    "           avg(avg_temp) AS avg_temp "
    "    FROM temp_day "
    "    WHERE date >= date('now', 'localtime', '-7 days') "
    "      AND date <= date('now', 'localtime') "
    "    GROUP BY strftime('%Y-%m-%d', date) "
    "    ORDER BY date ASC"
    ") "
    "SELECT date, avg_temp "
    "FROM daily_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATE", json_object_new_string(date));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }

        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_daily_month_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH daily_data AS ("
    "    SELECT strftime('%Y-%m-%d', date) AS date, "
    "           avg(avg_temp) AS avg_temp "
    "    FROM temp_day "
    "    WHERE date >= date('now', 'localtime', '-30 days') "
    "      AND date <= date('now', 'localtime') "
    "    GROUP BY strftime('%Y-%m-%d', date) "
    "    ORDER BY date ASC"
    ") "
    "SELECT date, avg_temp "
    "FROM daily_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATE", json_object_new_string(date));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }
        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_daily_year_avg()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "WITH daily_data AS ("
    "    SELECT strftime('%Y-%m-%d', date) AS date, "
    "           avg(avg_temp) AS avg_temp "
    "    FROM temp_day "
    "    WHERE date >= date('now', 'localtime', '-366 days') "
    "      AND date <= date('now', 'localtime') "
    "    GROUP BY strftime('%Y-%m-%d', date) "
    "    ORDER BY date ASC"
    ") "
    "SELECT date, avg_temp "
    "FROM daily_data;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATE", json_object_new_string(date));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }
        json_object_array_add(jsonArray, jsonObj);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

void get_last_60_seconds()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;

    int res = sqlite3_open("temperature.db", &db);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql =
    "SELECT date, temp "
    "FROM ( "
    "    SELECT date, temp "
    "    FROM temp_all "
    "    ORDER BY date DESC "
    "    LIMIT 60 "
    ") AS last_60 "
    "ORDER BY date ASC;";

    res = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    json_object *jsonArray = json_object_new_array();
    int has_data = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        has_data = 1;

        const char *date = (const char*)sqlite3_column_text(stmt, 0);
        const char *temp = (const char*)sqlite3_column_text(stmt, 1);

        json_object *jsonObj = json_object_new_object();
        json_object_object_add(jsonObj, "DATE", json_object_new_string(date));
        if (temp) {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string(temp));
        } else {
            json_object_object_add(jsonObj, "TEMP", json_object_new_string("null"));
        }
        json_object_array_add(jsonArray, jsonObj);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!has_data) {
        json_object_put(jsonArray);
        jsonArray = json_object_new_array();
    }

    const char *jsonStr = json_object_to_json_string(jsonArray);
    printf("%s\n", jsonStr);

    json_object_put(jsonArray);
}

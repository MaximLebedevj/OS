#include "html_response.h"
#include "json_response.h"

int main()
{
    char *request_uri = getenv("REQUEST_URI");
    char *client_type = getenv("CLIENT_TYPE");

    if (client_type == NULL) {
        client_type = "web";
    }

    if (strcmp(client_type, "web") == 0) {
        print_html_header();
        print_current_temperature();
        print_html_navigation();
    }

    if (strcmp(client_type, "web") == 0) {
        if (request_uri == NULL || strcmp(request_uri, "/") == 0) {
        }
        else if (strcmp(request_uri, "/hourly_day") == 0) {
            print_hourly_day_avg(request_uri);
        }
        else if (strcmp(request_uri, "/hourly_week") == 0) {
            print_hourly_week_avg(request_uri);
        }
        else if (strcmp(request_uri, "/hourly_month") == 0) {
            print_hourly_month_avg(request_uri);
        }
        else if (strcmp(request_uri, "/secondly_1min") == 0) {
            print_secondly_minute(request_uri);
        }
        else if (strcmp(request_uri, "/secondly_5min") == 0) {
            print_secondly_5minutes(request_uri);
        }
        else if (strcmp(request_uri, "/daily_week") == 0) {
            print_daily_week(request_uri);
        }
        else if (strcmp(request_uri, "/daily_month") == 0) {
            print_daily_month(request_uri);
        }
        else if (strcmp(request_uri, "/daily_3month") == 0) {
            print_daily_3month(request_uri);
        }
        else if (strcmp(request_uri, "/daily_6month") == 0) {
            print_daily_6month(request_uri);
        }
        else if (strcmp(request_uri, "/daily_year") == 0) {
            print_daily_year(request_uri);
        }
    } else if (strcmp(client_type, "qt-app") == 0) {
        if (strcmp(request_uri, "current") == 0) {
            get_current_temp();
        }
        else if (strcmp(request_uri, "hourly_day") == 0) {
            get_hourly_day_avg();
        }
        else if (strcmp(request_uri, "hourly_week") == 0) {
            get_hourly_weekly_avg();
        }
        else if (strcmp(request_uri, "hourly_month") == 0) {
            get_hourly_month_avg();
        }
        else if (strcmp(request_uri, "daily_week") == 0) {
            get_daily_week_avg();
        }
        else if (strcmp(request_uri, "daily_month") == 0) {
            get_daily_month_avg();
        }
        else if (strcmp(request_uri, "daily_year") == 0) {
            get_daily_year_avg();
        }
        else if (strcmp(request_uri, "current_minute") == 0) {
            get_last_60_seconds();
        }
    }

    if (strcmp(client_type, "web") == 0) {
        print_html_footer();
    }

    return 0;
}

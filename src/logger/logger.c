#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include "../../include/logger.h"
#include "../../include/cli.h"

static const char* get_operation_name(log_operation_type operation) 
{
    switch (operation) 
    {
        case LOG_OP_SYNC:         return "SYNC";
        case LOG_OP_BACKUP:       return "BACKUP";
        case LOG_OP_RESTORE:      return "RESTORE";
        case LOG_OP_LINK_CREATE:  return "LINK_CREATE";
        case LOG_OP_LINK_DELETE:  return "LINK_DELETE";
        case LOG_OP_CLEANUP:      return "CLEANUP";
        case LOG_OP_ERROR:        return "ERROR";
        default:                  return "UNKNOWN";
    }
}

static const char* get_status_name(log_status_type status) 
{
    switch (status) 
    {
        case LOG_STATUS_SUCCESS:  return "SUCCESS";
        case LOG_STATUS_FAILURE:  return "FAILURE";
        case LOG_STATUS_WARNING:  return "WARNING";
        default:                  return "UNKNOWN";
    }
}

int get_log_file_path(char *log_path, size_t max_len) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char month_str[16];
    strftime(month_str, sizeof(month_str), "%Y%m", timeinfo);
    
    snprintf(log_path, max_len, "%s/.dualsync/logs/%s.log", home, month_str);
    
    return 0;
}

static int ensure_logs_directory_exists(void) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    char logs_dir[PATH_MAX];
    snprintf(logs_dir, sizeof(logs_dir), "%s/.dualsync/logs", home);
    
    struct stat stat_buf;
    if (stat(logs_dir, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) 
    {
        return 0;
    }
    
    char command[PATH_MAX + 32];
    snprintf(command, sizeof(command), "mkdir -p '%s'", logs_dir);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Cannot create logs directory: %s\n", logs_dir);
        return -1;
    }
    
    return 0;
}

int log_operation(log_operation_type operation, const char *link_id, 
                  log_status_type status, const char *details) 
{
    if (ensure_logs_directory_exists() != 0) 
    {
        return -1;
    }
    
    char log_path[PATH_MAX];
    if (get_log_file_path(log_path, sizeof(log_path)) != 0) 
    {
        return -1;
    }
    
    FILE *log_file = fopen(log_path, "a");
    if (log_file == NULL) 
    {
        fprintf(stderr, "Error: Cannot open log file: %s\n", log_path);
        return -1;
    }
    
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", timeinfo);
    
    const char *op_name = get_operation_name(operation);
    const char *status_name = get_status_name(status);
    const char *link_display = (link_id != NULL && link_id[0] != '\0') ? link_id : "N/A";
    
    fprintf(log_file, "[%s] %s | %s | %s | %s\n",
            timestamp, op_name, link_display, status_name, details);
    
    fclose(log_file);
    
    printf("DEBUG: Logged operation: %s\n", op_name);
    
    return 0;
}

static time_t parse_date(const char *date_str) 
{
    struct tm tm_info = {0};
    
    if (sscanf(date_str, "%d/%d/%d", &tm_info.tm_mday, &tm_info.tm_mon, &tm_info.tm_year) != 3) 
    {
        fprintf(stderr, "Error: Invalid date format. Use DD/MM/YYYY\n");
        return -1;
    }
    
    tm_info.tm_mon -= 1;
    tm_info.tm_year -= 1900;
    tm_info.tm_isdst = -1;
    
    time_t result = mktime(&tm_info);
    if (result == -1) 
    {
        fprintf(stderr, "Error: Invalid date\n");
        return -1;
    }
    
    return result;
}

int get_sorted_log_files(char ***log_files_ptr) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    char logs_dir[PATH_MAX];
    snprintf(logs_dir, sizeof(logs_dir), "%s/.dualsync/logs", home);
    
    DIR *dir = opendir(logs_dir);
    if (dir == NULL) 
    {
        *log_files_ptr = NULL;
        return 0;
    }
    
    char **log_files = NULL;
    int log_count = 0;
    struct dirent *entry;
    
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".log") != NULL) 
        {
            log_count++;
            log_files = realloc(log_files, sizeof(char*) * log_count);
            log_files[log_count - 1] = malloc(strlen(entry->d_name) + 1);
            strcpy(log_files[log_count - 1], entry->d_name);
        }
    }
    
    closedir(dir);
    
    for (int i = 0; i < log_count - 1; i++) 
    {
        for (int j = i + 1; j < log_count; j++) 
        {
            if (strcmp(log_files[i], log_files[j]) > 0) 
            {
                char *temp = log_files[i];
                log_files[i] = log_files[j];
                log_files[j] = temp;
            }
        }
    }
    
    *log_files_ptr = log_files;
    return log_count;
}

int list_all_logs(void) 
{
    char **log_files = NULL;
    int log_count = get_sorted_log_files(&log_files);
    
    if (log_count < 0) 
    {
        return -1;
    }
    
    display_log_list(log_files, log_count);
    
    for (int i = 0; i < log_count; i++) 
    {
        free(log_files[i]);
    }
    free(log_files);
    
    return 0;
}

int list_logs_since(const char *date_str) 
{
    time_t target_date = parse_date(date_str);
    if (target_date == -1) 
    {
        return -1;
    }
    
    char **log_files = NULL;
    int log_count = get_sorted_log_files(&log_files);
    
    if (log_count < 0) 
    {
        return -1;
    }
    
    if (log_count == 0) 
    {
        display_log_list(NULL, 0);
        return 0;
    }
    
    char **filtered_files = NULL;
    int filtered_count = 0;
    
    for (int i = 0; i < log_count; i++) 
    {
        int year, month;
        if (sscanf(log_files[i], "%4d%2d", &year, &month) == 2) 
        {
            struct tm tm_info = {0};
            tm_info.tm_year = year - 1900;
            tm_info.tm_mon = month - 1;
            tm_info.tm_mday = 1;
            tm_info.tm_isdst = -1;
            
            time_t file_date = mktime(&tm_info);
            
            if (file_date >= target_date) 
            {
                filtered_count++;
                filtered_files = realloc(filtered_files, sizeof(char*) * filtered_count);
                filtered_files[filtered_count - 1] = log_files[i];
            }
        }
    }
    
    display_log_list_since(filtered_files, filtered_count, date_str);
    
    for (int i = 0; i < log_count; i++) 
    {
        free(log_files[i]);
    }
    free(log_files);
    free(filtered_files);
    
    return 0;
}

int track_logs(void) 
{
    char log_path[PATH_MAX];
    if (get_log_file_path(log_path, sizeof(log_path)) != 0) 
    {
        return -1;
    }
    
    struct stat stat_buf;
    if (stat(log_path, &stat_buf) != 0) 
    {
        printf("No logs yet. Waiting for operations...\n");
    }
    
    display_log_tracking_start();
    
    char command[PATH_MAX + 32];
    snprintf(command, sizeof(command), "tail -f '%s'", log_path);
    
    int result = system(command);
    
    display_log_tracking_end();
    
    return result;
}

int display_latest_log(void) 
{
    char log_path[PATH_MAX];
    if (get_log_file_path(log_path, sizeof(log_path)) != 0) 
    {
        return -1;
    }
    
    struct stat stat_buf;
    if (stat(log_path, &stat_buf) != 0) 
    {
        printf("No log file found\n");
        return 0;
    }
    
    display_log_latest(log_path);
    
    return 0;
}


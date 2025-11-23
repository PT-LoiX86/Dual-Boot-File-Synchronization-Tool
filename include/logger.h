#ifndef LOGGER_H
#define LOGGER_H

#include <time.h>

typedef enum 
{
    LOG_OP_SYNC,
    LOG_OP_BACKUP,
    LOG_OP_RESTORE,
    LOG_OP_LINK_CREATE,
    LOG_OP_LINK_DELETE,
    LOG_OP_CLEANUP,
    LOG_OP_CONVERT,
    LOG_OP_ERROR
} 
log_operation_type;

typedef enum 
{
    LOG_STATUS_SUCCESS,
    LOG_STATUS_FAILURE,
    LOG_STATUS_WARNING
} 
log_status_type;

int log_operation(log_operation_type operation, const char *link_id, 
                  log_status_type status, const char *details);
int get_log_file_path(char *log_path, size_t max_len);
int list_log_files(void);
int list_logs_since(const char *date_str);
int track_logs(void);
int display_latest_log(void);

#endif

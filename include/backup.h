#ifndef BACKUP_H
#define BACKUP_H

#include "filesystem.h"

typedef struct 
{
    char link_id[32];
    char location[16];
    char timestamp[16];
} 
backup_id_t;

typedef struct 
{
    char backup_id[128];
    char path[PATH_MAX];
    off_t size;
    time_t created_time;
} 
backup_info_t;

typedef struct 
{
    backup_info_t *backups;
    int count;
} 
backup_list_t;

int create_backup(const char *target_path, const char *link_id, 
                  char *backup_id_out);

int restore_backup(const char *backup_id);

int list_backups(const char *link_id, backup_list_t *backup_list);

int cleanup_backups(const char *link_id);

int parse_backup_id(const char *backup_id, backup_id_t *parsed);

int get_backup_path(const char *backup_id, char *backup_path, size_t max_len);

int backup_folder_exists(const char *link_id, const char *location);

int list_backups(const char *link_id, backup_list_t *backup_list);

void free_backup_list(backup_list_t *backup_list);

int cleanup_backups(const char *link_id);

int get_latest_backup(const char *link_id, const char *location, 
                      char *backup_id_out);

int backup_before_sync(folder_link_t *link, sync_operation_t operation);

int restore_on_sync_failure(folder_link_t *link, sync_operation_t operation);

int restore_backup_silent(const char *backup_id);

#endif
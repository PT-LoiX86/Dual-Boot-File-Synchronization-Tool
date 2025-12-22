#ifndef SYNC_H
#define SYNC_H

#include <limits.h>
#include <sys/param.h>
#include <stddef.h>
#include "filesystem.h"

typedef enum {
    FILE_STATUS_NEW = 0,
    FILE_STATUS_MODIFIED = 1,
    FILE_STATUS_DELETED = 2,
    FILE_STATUS_UNCHANGED = 3
} file_status_t;

typedef enum {
    CONFLICT_NONE = 0,
    CONFLICT_CONTENT_DIFF = 1,
    CONFLICT_TYPE_DIFF = 2
} conflict_type_t;

typedef struct {
    char path[PATH_MAX];
    file_status_t status;
    off_t size;
    time_t mtime;
    char md5_hash[33];
    conflict_type_t conflict;
} file_change_t;

typedef struct {
    file_change_t *changes;
    int count;
    int capacity;
    int new_count;
    int modified_count;
    int deleted_count;
    int conflict_count;
    unsigned long new_size;
    unsigned long modified_size;
    unsigned long deleted_size;
} sync_changes_t;

typedef enum {
    CONFLICT_CHOICE_OVERWRITE = 0,
    CONFLICT_CHOICE_KEEP_BOTH = 1
} conflict_choice_t;

typedef struct {
    file_change_t *file;
    conflict_choice_t choice;
} conflict_resolution_t;

typedef enum {
    SYNC_OP_TO_WINDOWS = 0,
    SYNC_OP_TO_UBUNTU = 1
} sync_operation_t;

sync_changes_t* create_sync_changes(void);
void free_sync_changes(sync_changes_t *changes);
void add_change(sync_changes_t *changes, const file_change_t *change);
int detect_changes(const char *source_path, const char *target_path, sync_changes_t *changes);
int scan_folder_for_changes(const char *folder_path, const char *other_folder_path, sync_changes_t *changes);
int is_folder_empty(const char *folder_path);
static void get_relative_path(const char *base_path, const char *full_path, char *relative_path, size_t max_len);
static int scan_folder_recursive(const char *folder_path, const char *base_path, sync_changes_t *changes, int is_source);

int perform_sync(folder_link_t *link, sync_operation_t operation,
                conflict_resolution_t *resolutions, int resolution_count);
int verify_sync_completion(const char *target_path, sync_changes_t *changes);
int update_sync_link(folder_link_t *link, sync_changes_t *changes);
int save_sync_config(linked_folders_t *folders, folder_link_t *updated_link);

int resolve_conflicts_interactive(sync_changes_t *changes, conflict_resolution_t **resolutions, int *resolution_count);
int restore_on_sync_failure(folder_link_t *link, sync_operation_t operation);
int backup_before_sync(folder_link_t *link, sync_operation_t operation);

#endif // SYNC_H

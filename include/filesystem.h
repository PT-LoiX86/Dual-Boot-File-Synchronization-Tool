#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <limits.h>
#include <sys/param.h>
#include <stddef.h>

// DISK / PARTITION CHECKING

typedef struct 
{
    char device[256];
    char mount_point[256];
    char filesystem_type[32];
    unsigned long total_bytes;
    unsigned long free_bytes;
    int is_mounted;
    int is_writable;
} 
disk_info_t;

typedef struct 
{
    disk_info_t *partitions;
    int count;
} 
windows_partitions_t;

int find_windows_partitions(windows_partitions_t *disk);
int get_disk_info(const char *path, disk_info_t *disk);
int get_disk_space(disk_info_t *disk);
int check_disk_status();

// FOLDER LINKING

typedef enum 
{
    SYNC_BIDIRECTIONAL = 0,
    SYNC_TO_WINDOWS = 1,
    SYNC_TO_UBUNTU = 2
} 
sync_direction_t;

typedef struct 
{
    char id[32];
    char ubuntu_path[PATH_MAX];
    char ubuntu_uuid[128];
    char windows_path[PATH_MAX];
    char windows_uuid[128];
    char windows_device[64];
    sync_direction_t sync_direction;
    char last_sync[64];
    int status;
} 
folder_link_t;

typedef struct 
{
    folder_link_t *links;
    int count;
} 
linked_folders_t;

int get_partition_uuid(const char *path, char *uuid, size_t uuid_size);
int get_device_for_path(const char *path, char *device, size_t device_size);
int check_folder_empty(const char *path);
int verify_folder_link(folder_link_t *link);
int folder_exists(const char *path);
int are_on_different_disks(const char *path1, const char *path2);
int validate_folders_for_linking(const char *ubuntu_path, const char *windows_path);
int create_folder_link(const char *ubuntu_path, const char *windows_path,
                       folder_link_t *new_link);
int verify_folder_link_accessible(folder_link_t *link);

// SYNC OPERATIONS

typedef enum 
{
    FILE_STATUS_NEW = 0,
    FILE_STATUS_MODIFIED = 1,
    FILE_STATUS_DELETED = 2,
    FILE_STATUS_UNCHANGED = 3
} 
file_status_t;

typedef enum 
{
    CONFLICT_NONE = 0,
    CONFLICT_CONTENT_DIFF = 1,
    CONFLICT_TYPE_DIFF = 2
} 
conflict_type_t;

typedef struct 
{
    char path[PATH_MAX];
    file_status_t status;
    off_t size;
    time_t mtime;
    char md5_hash[33];
    conflict_type_t conflict;
} 
file_change_t;

typedef struct 
{
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
} 
sync_changes_t;

typedef enum 
{
    CONFLICT_CHOICE_OVERWRITE = 0,
    CONFLICT_CHOICE_KEEP_BOTH = 1
} 
conflict_choice_t;

typedef struct 
{
    file_change_t *file;
    conflict_choice_t choice;
} 
conflict_resolution_t;

int calculate_file_md5(const char *filepath, char *md5_hash);
sync_changes_t* create_sync_changes(void);
void free_sync_changes(sync_changes_t *changes);
void add_change(sync_changes_t *changes, const file_change_t *change);
int scan_folder_for_changes(const char *folder_path, const char *other_folder_path, 
                            sync_changes_t *changes);
int compare_files(const char *file1, const char *file2, int *are_equal);
int detect_changes(const char *source_path, const char *target_path,
                   sync_changes_t *changes);
                   int update_sync_link(folder_link_t *link, sync_changes_t *changes);
int display_sync_summary(sync_changes_t *changes, const char *source, 
                        const char *target, int error_count);
int save_sync_config(linked_folders_t *folders, folder_link_t *updated_link);
int verify_sync_completion(const char *target_path, sync_changes_t *changes);
typedef enum 
{
    SYNC_OP_TO_WINDOWS = 0,
    SYNC_OP_TO_UBUNTU = 1
} 
sync_operation_t;
int log_sync_operation(folder_link_t *link, sync_operation_t operation, 
                       sync_changes_t *changes, int success);
int is_folder_empty(const char *folder_path);

#endif

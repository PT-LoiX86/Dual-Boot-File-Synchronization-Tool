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

#endif

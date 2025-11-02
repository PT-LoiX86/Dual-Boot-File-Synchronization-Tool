#ifndef FILESYSTEM_H
#define FILESYSTEM_H

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

#endif

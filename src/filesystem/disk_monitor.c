#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <sys/statvfs.h>
#include "../include/filesystem.h"
#include "../include/cli.h"

int get_disk_space(disk_info_t *disk) 
{
    struct statvfs stat;
    
    if (statvfs(disk->mount_point, &stat) != 0) {
        return -1;
    }

    disk->total_bytes = stat.f_blocks * stat.f_frsize;
    disk->free_bytes = stat.f_bfree * stat.f_frsize;
    disk->is_writable = !(stat.f_flag & ST_RDONLY);

    return 0;
}

int find_windows_partitions(windows_partitions_t *list) 
{
    FILE *mtab = setmntent("/proc/mounts", "r");
    struct mntent *entry;

    while ((entry = getmntent(mtab)) != NULL)
    {
        if (strcmp(entry->mnt_dir, "/boot/efi") == 0 ||
            strcmp(entry->mnt_dir, "/boot") == 0) 
        {
            continue;
        }

        if (strcmp(entry->mnt_type, "ntfs") == 0 || 
            strcmp(entry->mnt_type, "ntfs-3g") == 0 ||
            strcmp(entry->mnt_type, "ntfs3") == 0)
        { 
            disk_info_t *disk = &list->partitions[list->count];

            strncpy(disk->device, entry->mnt_fsname, sizeof(disk->device) - 1);
            strncpy(disk->mount_point, entry->mnt_dir, sizeof(disk->mount_point) - 1);
            strncpy(disk->filesystem_type, entry->mnt_type, sizeof(disk->filesystem_type) - 1);
            
            disk->is_mounted = 1;
            get_disk_space(disk);

            list->count++;
        }
    }
    
    endmntent(mtab);
    return list->count > 0 ? 0 : -1;
}

int get_ubuntu_partition_info(const char *mount_point, disk_info_t *disk) 
{
    FILE *mtab = setmntent("/proc/mounts", "r");
    struct mntent *entry;

    while ((entry = getmntent(mtab)) != NULL) 
    {
        if (strcmp(entry->mnt_dir, mount_point) == 0) 
        {
            strncpy(disk->device, entry->mnt_fsname, sizeof(disk->device) - 1);
            strncpy(disk->mount_point, entry->mnt_dir, sizeof(disk->mount_point) - 1);
            strncpy(disk->filesystem_type, entry->mnt_type, sizeof(disk->filesystem_type) - 1);
            
            disk->is_mounted = 1;
            get_disk_space(disk);
            
            endmntent(mtab);
            return 0;
        }
    }
    
    endmntent(mtab);
    return -1;
}

int check_disk_status() 
{    
    windows_partitions_t windows_partitions = {0};
    disk_info_t ubuntu_partition = {0};

    // Get Ubuntu partition info
    if (get_ubuntu_partition_info("/", &ubuntu_partition) != 0) 
    {
        fprintf(stderr, "Error: Could not get Ubuntu partition info\n");
        return 1;
    }
    
    // Get Windows partition info
    windows_partitions.partitions = malloc(sizeof(disk_info_t) * 10);
    if (windows_partitions.partitions == NULL) 
    {
        fprintf(stderr, "Error: Failed to allocate memory for Windows partitions\n");
        return 1;
    }
    
    windows_partitions.count = 0;
    if (find_windows_partitions(&windows_partitions) != 0) 
    {
        printf("Warning: No Windows partitions found\n");
    }

    int result = display_disk_status(&windows_partitions, &ubuntu_partition);
    
    if (windows_partitions.partitions != NULL) 
    {
        free(windows_partitions.partitions);
    }
    
    return result;
}
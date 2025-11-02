#include <stdio.h>
#include "../include/filesystem.h"


void print_disk_size(disk_info_t *disk) 
{
    double total_gb = disk->total_bytes / (1024.0 * 1024.0 * 1024.0);
    double free_gb = disk->free_bytes / (1024.0 * 1024.0 * 1024.0);
    double used_gb = total_gb - free_gb;

    printf("  Size: %.1f GB (%.1f GB used, %.1f GB free)\n", 
           total_gb, used_gb, free_gb);
}

void print_access_status(disk_info_t *disk) 
{
    if (!disk->is_writable) {
        printf("  Status: Mounted (read-only) ⚠️\n");
    } else {
        printf("  Status: Mounted (read/write) ✓\n");
    }
}

int display_disk_status(windows_partitions_t *windows_partitions, disk_info_t *ubuntu) 
{

    printf("[>] Ubuntu Disk Status\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    printf("  Device: %s\n", ubuntu->device);
    printf("  Mount Point: %s\n", ubuntu->mount_point);
    printf("  Type: %s\n", ubuntu->filesystem_type);
    
    print_disk_size(ubuntu);
    print_access_status(ubuntu);

    printf("\n[>] Windows Disk Status\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    if (windows_partitions->count == 0) 
    {
        printf("✗ Windows partitions not found or not mounted\n");
        return 1;
    }

    for (int i = 0; i < windows_partitions->count; i++) 
    {
        disk_info_t *disk = &windows_partitions->partitions[i];

        printf("  Device: %s\n", disk->device);
        printf("  Mount Point: %s\n", disk->mount_point);
        printf("  Type: %s\n", disk->filesystem_type);
        
        print_disk_size(disk);
        print_access_status(disk);
        printf("\n");
    }

    return 0;
}
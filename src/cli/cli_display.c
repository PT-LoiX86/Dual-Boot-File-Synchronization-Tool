#include <stdio.h>
#include <string.h>
#include "../include/filesystem.h"
#include "../../include/cli.h"


// DISK / PARTITION CHECKING

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

// FOLDER LINKING

int display_link_success(folder_link_t *link) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║             ✓ FOLDER LINK CREATED SUCCESSFULLY             ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Link ID: %s\n", link->id);
    printf("║\n");
    printf("║ Ubuntu Folder:\n");
    printf("║   Path: %s\n", link->ubuntu_path);
    printf("║   UUID: %s\n", link->ubuntu_uuid);
    printf("║\n");
    printf("║ Windows Folder:\n");
    printf("║   Path: %s\n", link->windows_path);
    printf("║   UUID: %s\n", link->windows_uuid);
    printf("║   Device: %s\n", link->windows_device);
    printf("║\n");
    printf("║ Status: Ready for sync\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    printf("Next steps:\n");
    printf("  • Run 'dualsync sync <folder-path> to-windows' to sync Ubuntu → Windows\n");
    printf("  • Run 'dualsync sync <folder-path> to-ubuntu' to sync Windows → Ubuntu\n");
    printf("  • Run 'dualsync unlink <folder-path>' to remove this link\n\n");
    
    return 0;
}

int display_link_already_exists(folder_link_t *existing_link, 
                                const char *provided_path, const char *other_path) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                 ⚠️  LINK ALREADY EXISTS                     ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ This folder is already linked to:\n");
    printf("║   %s\n", existing_link->ubuntu_path);
    printf("║            ↕️\n");
    printf("║   %s\n", existing_link->windows_path);
    printf("║\n");
    printf("║ You're trying to link it to:\n");
    printf("║   %s\n", other_path);
    printf("║\n");
    printf("║ Options:\n");
    printf("║ 1. Use 'dualsync unlink %s' to remove the existing link\n", provided_path);
    printf("║ 2. Then create a new link with the desired folder\n");
    printf("║ 3. Or choose different folders to link\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 1;
}

int display_unlink_confirmation(folder_link_t *link) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              CONFIRM UNLINK REQUEST                        ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ You are about to unlink the following folder pair:\n");
    printf("║\n");
    printf("║ Ubuntu Folder:\n");
    printf("║   %s\n", link->ubuntu_path);
    printf("║\n");
    printf("║ Windows Folder:\n");
    printf("║   %s\n", link->windows_path);
    printf("║\n");
    printf("║ This will NOT delete any files, only remove the link.\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}

int display_verify_link_error(int error_code, folder_link_t *link) 
{
    if (error_code == -2) 
    {
        char actual_windows_uuid[128] = {0};
        get_partition_uuid(link->windows_path, actual_windows_uuid, sizeof(actual_windows_uuid));
        
        fprintf(stderr, "\n");
        fprintf(stderr, "╔════════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║                 ERROR: WRONG DISK CONNECTED!               ║\n");
        fprintf(stderr, "╠════════════════════════════════════════════════════════════╣\n");
        fprintf(stderr, "║ Expected UUID: %s\n", link->windows_uuid);
        fprintf(stderr, "║ Found UUID:    %s\n", actual_windows_uuid);
        fprintf(stderr, "║                                                            ║\n");
        fprintf(stderr, "║ The Windows partition UUID does not match.                 ║\n");
        fprintf(stderr, "║ Please connect the correct external disk.                  ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════════════════════╝\n\n");
        return -1;
    }

    else if (error_code == -1) 
    {
        fprintf(stderr, "\n");
        fprintf(stderr, "╔════════════════════════════════════════════════════════════╗\n");
        fprintf(stderr, "║              ERROR: FOLDERS NOT ACCESSIBLE                 ║\n");
        fprintf(stderr, "╠════════════════════════════════════════════════════════════╣\n");
        fprintf(stderr, "║ Ubuntu folder: %s\n", link->ubuntu_path);
        fprintf(stderr, "║ Windows folder: %s\n", link->windows_path);
        fprintf(stderr, "║                                                            ║\n");
        fprintf(stderr, "║ One or both folders are not accessible.                    ║\n");
        fprintf(stderr, "║ Please check if folders exist and are properly mounted.    ║\n");
        fprintf(stderr, "╚════════════════════════════════════════════════════════════╝\n\n");
        return -1;
    }
    
    return 0;
}


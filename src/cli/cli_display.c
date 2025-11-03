#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

// SYNC OPERATIONS

static void format_size(unsigned long bytes, char *buffer, size_t buf_size) 
{
    if (bytes < 1024) 
    {
        snprintf(buffer, buf_size, "%lu B", bytes);
    } 
    else if (bytes < 1024 * 1024) 
    {
        snprintf(buffer, buf_size, "%.1f KB", bytes / 1024.0);
    } 
    else if (bytes < 1024 * 1024 * 1024) 
    {
        snprintf(buffer, buf_size, "%.1f MB", bytes / (1024.0 * 1024.0));
    } 
    else 
    {
        snprintf(buffer, buf_size, "%.1f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

int display_sync_preview(sync_changes_t *changes, const char *source, const char *target) 
{
    char new_size_str[32];
    char modified_size_str[32];
    char deleted_size_str[32];
    char total_size_str[32];
    
    unsigned long total_size = changes->new_size + changes->modified_size;
    
    format_size(changes->new_size, new_size_str, sizeof(new_size_str));
    format_size(changes->modified_size, modified_size_str, sizeof(modified_size_str));
    format_size(changes->deleted_size, deleted_size_str, sizeof(deleted_size_str));
    format_size(total_size, total_size_str, sizeof(total_size_str));
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              SYNC PREVIEW: %s → %s       ║\n", 
           source, target);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ Files to add:      %3d files (%s)\n", changes->new_count, new_size_str);
    printf("║ Files to update:   %3d files (%s)\n", changes->modified_count, modified_size_str);
    printf("║ Files to delete:   %3d files (%s)\n", changes->deleted_count, deleted_size_str);
    
    if (changes->conflict_count > 0) 
    {
        printf("║ Conflicts:         %3d file(s) (needs resolution)     \n", changes->conflict_count);
    }
    
    printf("║                                                            ║\n");
    printf("║ Total change:      %s                                  ║\n", total_size_str);
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    printf("Sync will affect %d items. Continue? (yes/no): ", 
           changes->count);
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL) 
    {
        printf("Sync cancelled\n");
        return -1;
    }
    
    if (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0) 
    {
        printf("Sync cancelled\n");
        return -1;
    }
    
    return 0;
}

int resolve_conflict_interactive(file_change_t *file, conflict_resolution_t *resolution) 
{
    char response[10];
    
    if (file == NULL || resolution == NULL) 
    {
        return -1;
    }
    
    resolution->file = file;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    FILE CONFLICT DETECTED                  ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ File: %s\n", file->path);
    printf("║                                                            ║\n");
    printf("║ This file exists on both sides with different content.    ║\n");
    printf("║ Choose one of the following options:                      ║\n");
    printf("║                                                            ║\n");
    printf("║ 1. Overwrite target with source                           ║\n");
    printf("║ 2. Keep both (rename target with timestamp)               ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("Your choice (1 or 2): ");
    
    if (fgets(response, sizeof(response), stdin) == NULL) 
    {
        printf("Conflict resolution cancelled\n");
        return -1;
    }
    
    if (response[0] == '1') 
    {
        resolution->choice = CONFLICT_CHOICE_OVERWRITE;
        printf("Choice: Overwrite target\n\n");
        return 0;
    } 
    else if (response[0] == '2') 
    {
        resolution->choice = CONFLICT_CHOICE_KEEP_BOTH;
        printf("Choice: Keep both files\n\n");
        return 0;
    } 
    else 
    {
        printf("Invalid choice. Using default (overwrite)\n\n");
        resolution->choice = CONFLICT_CHOICE_OVERWRITE;
        return 0;
    }
}

int resolve_conflicts_interactive(sync_changes_t *changes, 
                                  conflict_resolution_t **resolutions, 
                                  int *resolution_count)
{
    if (changes == NULL || resolutions == NULL || resolution_count == NULL)
    {
        return -1;
    }
    
    if (changes->conflict_count == 0) 
    {
        *resolutions = NULL;
        *resolution_count = 0;
        return 0;
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("             RESOLVING %d CONFLICT(S)\n", changes->conflict_count);
    printf("════════════════════════════════════════════════════════════\n");
    
    *resolutions = malloc(sizeof(conflict_resolution_t) * changes->conflict_count);
    if (*resolutions == NULL) 
    {
        fprintf(stderr, "Error: Cannot allocate memory for resolutions\n");
        return -1;
    }
    
    int resolution_idx = 0;
    
    for (int i = 0; i < changes->count; i++) 
    {
        if (changes->changes[i].conflict != CONFLICT_NONE) 
        {
            printf("Conflict %d of %d\n", resolution_idx + 1, changes->conflict_count);
            
            if (resolve_conflict_interactive(&changes->changes[i], 
                                             &(*resolutions)[resolution_idx]) != 0) 
            {
                printf("Error resolving conflict\n");
                free(*resolutions);
                return -1;
            }
            
            resolution_idx++;
        }
    }
    
    *resolution_count = resolution_idx;
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("All conflicts resolved\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_final_confirmation(sync_changes_t *changes, int has_conflicts) 
{
    char response[10];
    
    if (has_conflicts > 0) 
    {
        printf("════════════════════════════════════════════════════════════\n");
        printf("         Ready to sync (with conflict resolutions)\n");
        printf("════════════════════════════════════════════════════════════\n");
    } 
    else 
    {
        printf("════════════════════════════════════════════════════════════\n");
        printf("                  Ready to sync\n");
        printf("════════════════════════════════════════════════════════════\n");
    }
    
    printf("Proceed with sync? (yes/no): ");
    
    if (fgets(response, sizeof(response), stdin) == NULL) 
    {
        printf("Sync cancelled\n");
        return -1;
    }
    
    if (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0) 
    {
        printf("Sync cancelled\n");
        return -1;
    }
    
    return 0;
}

int display_sync_error_prompt(const char *filepath, const char *error_msg)
{
    char response[10];
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                      SYNC ERROR                            ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ File: %s\n", filepath);
    printf("║ Error: %s\n", error_msg);
    printf("║                                                            ║\n");
    printf("║ Options:                                                   ║\n");
    printf("║ 1. Continue sync (skip this file)                         ║\n");
    printf("║ 2. Abort sync                                             ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("Your choice (1 or 2): ");
    
    if (fgets(response, sizeof(response), stdin) == NULL) 
    {
        return -1;
    }
    
    if (response[0] == '1') 
    {
        printf("Continuing...\n\n");
        return 0;
    } 
    else
    {
        printf("Aborting sync...\n\n");
        return -1;
    }
}
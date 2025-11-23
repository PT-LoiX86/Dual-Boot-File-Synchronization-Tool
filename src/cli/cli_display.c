#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/filesystem.h"
#include "../../include/cli.h"
#include "../../include/converter.h"


// ============ DISK / PARTITION CHECKING ============

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

// ============ FOLDER LINKING ============

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

int display_linked_folders(linked_folders_t *folders) 
{
    if (folders == NULL || folders->count == 0) 
    {
        printf("No linked folders found\n");
        return 0;
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    printf("                          LINKED FOLDERS\n");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    printf("%-35s %-25s %-25s\n", "ID", "Ubuntu Path", "Windows Path");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    
    for (int i = 0; i < folders->count; i++) 
    {
        printf("%-35s %-25s %-25s\n",
               folders->links[i].id,
               folders->links[i].ubuntu_path,
               folders->links[i].windows_path);
    }
    
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    printf("Total: %d linked folder(s)\n", folders->count);
    printf("════════════════════════════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

// ============ SYNC OPERATIONS ============

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

// ============ BACKUP OPERATIONS ============

int display_backup_created(const char *backup_id, const char *backup_path, off_t size) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                    BACKUP CREATED\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Backup ID:    %s\n", backup_id);
    printf("Location:     %s\n", backup_path);
    printf("Size:         %.2f MB\n", size / (1024.0 * 1024.0));
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_restore_confirmation(const char *backup_id, const char *target_path) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  RESTORE BACKUP?                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ Backup ID:    %s\n", backup_id);
    printf("║ Target Path:  %s\n", target_path);
    printf("║                                                            ║\n");
    printf("║ WARNING: This will OVERWRITE all files in target folder!  ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("Continue? (yes/no): ");
    
    return 0;
}

int display_restore_successful(const char *backup_id, const char *target_path) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                   RESTORE SUCCESSFUL\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Restored from: %s\n", backup_id);
    printf("Target path:   %s\n", target_path);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_backup_list(const char *link_id, backup_list_t *backup_list) 
{
    if (backup_list == NULL || backup_list->count == 0 || backup_list->backups == NULL) 
    {
        printf("No backups found for link: %s\n", link_id);
        return 0;
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    printf("                  BACKUPS FOR: %s\n", link_id);
    printf("════════════════════════════════════════════════════════════════════════════════════\n\n");
    
    printf("%-45s %-20s %-12s\n", "Backup ID", "Created", "Size");
    printf("════════════════════════════════════════════════════════════════════════════════════\n");
    
    for (int i = 0; i < backup_list->count; i++) 
    {
        char *last_underscore = strrchr(backup_list->backups[i].backup_id, '_');
        char *second_last_underscore = NULL;
        
        if (last_underscore != NULL) 
        {
            *last_underscore = '\0';
            second_last_underscore = strrchr(backup_list->backups[i].backup_id, '_');
            *last_underscore = '_';
        }
        
        char timestamp_str[20] = "Unknown";
        if (last_underscore != NULL && second_last_underscore != NULL) 
        {
            const char *date_part = second_last_underscore + 1;
            const char *time_part = last_underscore + 1;
            
            if (strlen(date_part) > 8 && strlen(time_part) > 5) 
            {
                snprintf(timestamp_str, sizeof(timestamp_str), "%s %s", date_part, time_part);
            }
        }
        
        printf("%-45s %-20s %.2f MB\n",
               backup_list->backups[i].backup_id,
               timestamp_str,
               backup_list->backups[i].size / (1024.0 * 1024.0));
    }
    
    printf("════════════════════════════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}




int display_cleanup_confirmation(const char *link_id, int backup_count) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              DELETE ALL BACKUPS?                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ Link ID:      %s\n", link_id);
    printf("║ Backups:      %d\n", backup_count);
    printf("║                                                            ║\n");
    printf("║ WARNING: This action CANNOT be undone!                    ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("Continue? (yes/no): ");
    
    return 0;
}

int display_cleanup_successful(const char *link_id, int deleted_count) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                 BACKUPS DELETED\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Link ID:       %s\n", link_id);
    printf("Deleted:       %d backup(s)\n", deleted_count);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_backup_error(const char *error_msg) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   ERROR                                    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ %s\n", error_msg);
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}

int display_auto_restore_start(const char *backup_id) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                  SYNC FAILED - AUTO-RESTORING\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Attempting to restore from latest backup...\n");
    printf("Backup ID: %s\n", backup_id);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_auto_restore_successful(const char *backup_id) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                 AUTO-RESTORE SUCCESSFUL\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Restored backup: %s\n", backup_id);
    printf("Original state restored. Please investigate sync error.\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_auto_restore_failed(const char *backup_id) 
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   CRITICAL ERROR!                         ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║                                                            ║\n");
    printf("║ Sync FAILED and AUTO-RESTORE also FAILED!                 ║\n");
    printf("║                                                            ║\n");
    printf("║ Your target folder may be in an inconsistent state.       ║\n");
    printf("║                                                            ║\n");
    printf("║ Backup ID (for manual restoration):                       ║\n");
    printf("║ %s\n", backup_id);
    printf("║                                                            ║\n");
    printf("║ Manual restore command:                                   ║\n");
    printf("║ dualsync restore %s                           ║\n", backup_id);
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    return 0;
}

// ============ LOG DISPLAY FUNCTIONS ============

int display_log_list(char **log_files, int log_count) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                    ALL LOGS (oldest-latest)\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    if (log_count == 0 || log_files == NULL) 
    {
        printf("No logs found\n");
    } 
    else 
    {
        for (int i = 0; i < log_count; i++) {
            printf("%d. %s\n", i + 1, log_files[i]);
        }
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Total: %d log file(s)\n", log_count);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_log_list_since(char **log_files, int log_count, const char *date_str) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("              LOGS SINCE: %s\n", date_str);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    if (log_count == 0 || log_files == NULL) 
    {
        printf("No logs found since %s\n", date_str);
    } 
    else 
    {
        for (int i = 0; i < log_count; i++) 
        {
            printf("%d. %s\n", i + 1, log_files[i]);
        }
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Total: %d log file(s) since %s\n", log_count, date_str);
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_log_tracking_start(void) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("              TRACKING LOGS (Press Ctrl+C to exit)\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_log_tracking_end(void) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                   TRACKING STOPPED\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_log_latest(const char *log_path) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                  LATEST LOG ENTRIES\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    char command[PATH_MAX + 64];
    snprintf(command, sizeof(command), "tail -20 '%s'", log_path);
    
    if (system(command) != 0) 
    {
        printf("(No log entries yet)\n");
    }
    
    printf("\n════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

// ============ CONVERTER ============

int display_conversion_prompt(const convertible_files_list_t *files_list) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                    FILE CONVERSION\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    printf("Found %d convertible file(s):\n\n", files_list->count);
    
    for (int i = 0; i < files_list->count; i++) 
    {
        char filename_only[PATH_MAX];
        const char *basename = strrchr(files_list->files[i].filename, '/');
        if (basename != NULL) 
        {
            basename++;
            strncpy(filename_only, basename, sizeof(filename_only) - 1);
        } 
        else 
        {
            strncpy(filename_only, files_list->files[i].filename, sizeof(filename_only) - 1);
        }
        
        printf("  • %s → %s/%s.%s\n", 
               filename_only,
               files_list->files[i].target_folder,
               filename_only,
               files_list->files[i].target_ext);
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("Convert these files? (yes/no): ");
    
    return 0;
}

int display_conversion_start(void) 
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                  CONVERTING FILES\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}

int display_conversion_progress(int current_file, int total_files, 
                                const char *filename, const char *target_ext) 
{
    int progress_percent = (current_file * 100) / total_files;
    int bar_length = 20;
    int filled = (progress_percent * bar_length) / 100;
    
    printf("\r[");
    for (int i = 0; i < bar_length; i++) 
    {
        if (i < filled) 
        {
            printf("█");
        } 
        else 
        {
            printf("░");
        }
    }
    printf("] %d/%d Converting: %s → %s", current_file, total_files, 
           filename, target_ext);
    fflush(stdout);
    
    return 0;
}

int display_conversion_app_not_found(const char *app_name, const char *filename) 
{
    printf("\n");
    printf("⚠ Warning: %s not installed, skipping: %s\n", app_name, filename);
    
    return 0;
}

int display_conversion_completed(int success_count, int fail_count, int skip_count) 
{
    printf("\n\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("              CONVERSION COMPLETED\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    printf("Results:\n");
    printf("  ✓ Converted:  %d file(s)\n", success_count);
    
    if (fail_count > 0) 
    {
        printf("  ✗ Failed:     %d file(s)\n", fail_count);
    }
    
    if (skip_count > 0) 
    {
        printf("  ⊘ Skipped:    %d file(s)\n", skip_count);
    }
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"
#include "../include/filesystem.h"
#include "../../include/config.h"
#include <sync.h>

// ============ DISK / PARTITION CHECKING ============

int handle_disk_command() 
{
    return check_disk_status();
}

// ============ FOLDER LINKING ============

int handle_link_command(int argc, char *argv[]) 
{
    const char *ubuntu_path = NULL;
    const char *windows_path = NULL;
    
    printf("DEBUG: Entered handle_link_command\n");
    
    if (argc < 3) 
    {
        fprintf(stderr, "Usage: dualsync link <ubuntu-path> <windows-path>\n");
        return 1;
    }
    
    ubuntu_path = argv[1];
    windows_path = argv[2];
    
    printf("DEBUG: Ubuntu path: %s\n", ubuntu_path);
    printf("DEBUG: Windows path: %s\n", windows_path);
    
    linked_folders_t folders = {0};
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return 1;
    }
    
    printf("DEBUG: Loaded %d existing links\n", folders.count);
    
    int existing_index = find_existing_link(&folders, ubuntu_path);
    if (existing_index >= 0) 
    {
        return display_link_already_exists(&folders.links[existing_index], ubuntu_path, windows_path);
    }
    
    existing_index = find_existing_link(&folders, windows_path);
    if (existing_index >= 0) 
    {
        return display_link_already_exists(&folders.links[existing_index], windows_path, ubuntu_path);
    }
    
    printf("DEBUG: Folders not already linked\n");
    
    folder_link_t new_link = {0};
    if (create_folder_link(ubuntu_path, windows_path, &new_link) != 0) 
    {
        fprintf(stderr, "Error: Cannot create folder link\n");
        free(folders.links);
        return 1;
    }
    
    printf("DEBUG: Link created successfully\n");
    
    if (folders.count >= 50) 
    {
        fprintf(stderr, "Error: Maximum number of links (50) reached\n");
        free(folders.links);
        return 1;
    }
    
    folders.links[folders.count++] = new_link;
    
    if (save_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot save configuration\n");
        free(folders.links);
        return 1;
    }
    
    printf("DEBUG: Config saved\n");
    
    display_link_success(&new_link);
    
    free(folders.links);
    return 0;
}

int handle_unlink_command(int argc, char *argv[]) 
{
    const char *folder_path = NULL;
    
    printf("DEBUG: Entered handle_unlink_command\n");
    
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: dualsync unlink <folder-path>\n");
        return 1;
    }
    
    folder_path = argv[1];
    
    printf("DEBUG: Folder path to unlink: %s\n", folder_path);
    
    linked_folders_t folders = {0};
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return 1;
    }
    
    int link_index = find_existing_link(&folders, folder_path);
    if (link_index < 0) 
    {
        fprintf(stderr, "Error: No link found for path: %s\n", folder_path);
        free(folders.links);
        return 1;
    }
    
    display_unlink_confirmation(&folders.links[link_index]);
    
    printf("Are you sure you want to unlink this folder pair? (yes/no): ");
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL) 
    {
        printf("Unlink cancelled\n");
        free(folders.links);
        return 1;
    }
    
    if (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0) 
    {
        printf("Unlink cancelled\n");
        free(folders.links);
        return 1;
    }
    
    remove_link_from_config(&folders, link_index);
    
    if (save_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot save configuration\n");
        free(folders.links);
        return 1;
    }
    
    printf("\n✓ Folder pair unlinked successfully\n\n");
    
    free(folders.links);
    return 0;
}

int handle_links_list_command(int argc, char *argv[]) 
{
    linked_folders_t folders = {0};
    
    printf("DEBUG: Entered handle_links_list_command\n");
    (void)argc;
    (void)argv;
    
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return 1;
    }
    
    display_linked_folders(&folders);
    
    free(folders.links);
    return 0;
}


// ============ SYNC OPERATIONS ============

int handle_sync_command(int argc, char *argv[]) 
{
    const char *folder_path = NULL;
    const char *direction_str = NULL;
    sync_operation_t operation;
    linked_folders_t folders = {0};
    folder_link_t *link = NULL;
    sync_changes_t *changes = NULL;
    conflict_resolution_t *resolutions = NULL;
    int resolution_count = 0;
    
    printf("DEBUG: Entered handle_sync_command\n");
    
    if (argc < 3) 
    {
        fprintf(stderr, "Usage: dualsync sync <folder-path> <to-windows|to-ubuntu>\n");
        return 1;
    }
    
    folder_path = argv[1];
    direction_str = argv[2];
    
    printf("DEBUG: Folder path: %s\n", folder_path);
    printf("DEBUG: Direction: %s\n", direction_str);
    
    if (strcmp(direction_str, "to-windows") == 0) 
    {
        operation = SYNC_OP_TO_WINDOWS;
    } 
    else if (strcmp(direction_str, "to-ubuntu") == 0) 
    {
        operation = SYNC_OP_TO_UBUNTU;
    } 
    else 
    {
        fprintf(stderr, "Error: Invalid direction. Use 'to-windows' or 'to-ubuntu'\n");
        return 1;
    }
    
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return 1;
    }
    
    int link_index = find_existing_link(&folders, folder_path);
    if (link_index < 0) 
    {
        fprintf(stderr, "Error: No link found for path: %s\n", folder_path);
        free(folders.links);
        return 1;
    }
    
    link = &folders.links[link_index];
    
    printf("DEBUG: Found link: %s\n", link->id);
    
    changes = create_sync_changes();
    if (changes == NULL) 
    {
        fprintf(stderr, "Error: Cannot create sync changes structure\n");
        free(folders.links);
        return 1;
    }
    
    const char *source = (operation == SYNC_OP_TO_WINDOWS) ? link->ubuntu_path : link->windows_path;
    const char *target = (operation == SYNC_OP_TO_WINDOWS) ? link->windows_path : link->ubuntu_path;
    
    printf("DEBUG: Checking if source folder is empty\n");
    int source_empty = is_folder_empty(source);
    if (source_empty == 1) 
    {
        printf("\n");
        printf("╔════════════════════════════════════════════════════════════╗\n");
        printf("║                    WARNING                                 ║\n");
        printf("╠════════════════════════════════════════════════════════════╣\n");
        printf("║                                                            ║\n");
        printf("║ Source folder is EMPTY!                                   ║\n");
        printf("║ Path: %s\n", source);
        printf("║                                                            ║\n");
        printf("║ Syncing from an empty folder will DELETE all files in     ║\n");
        printf("║ the target folder!                                        ║\n");
        printf("║                                                            ║\n");
        printf("╚════════════════════════════════════════════════════════════╝\n");
        
        printf("Continue anyway? (yes/no): ");
        char response[10];
        if (fgets(response, sizeof(response), stdin) == NULL ||
            (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0))
        {
            printf("Sync cancelled\n");
            free(folders.links);
            return 1;
        }
    }

    if (detect_changes(source, target, changes) != 0) 
    {
        fprintf(stderr, "Error: Cannot detect changes\n");
        free_sync_changes(changes);
        free(folders.links);
        return 1;
    }
    
    if (display_sync_preview(changes, source, target) != 0) 
    {
        free_sync_changes(changes);
        free(folders.links);
        return 1;
    }
    
    if (changes->conflict_count > 0) 
    {
        if (resolve_conflicts_interactive(changes, &resolutions, &resolution_count) != 0) 
        {
            fprintf(stderr, "Error: Cannot resolve conflicts\n");
            free_sync_changes(changes);
            free(folders.links);
            return 1;
        }
    }
    
    if (display_final_confirmation(changes, changes->conflict_count) != 0)
    {
        free(resolutions);
        free_sync_changes(changes);
        free(folders.links);
        return 1;
    }
    
    int sync_result = perform_sync(link, operation, resolutions, resolution_count);
        
    if (sync_result == 0 || sync_result == 1) 
    {
        if (save_sync_config(&folders, link) != 0) 
        {
            fprintf(stderr, "Warning: Cannot save updated configuration\n");
        }
    }
    
    if (resolutions != NULL) 
    {
        free(resolutions);
    }
    free_sync_changes(changes);
    free(folders.links);
    
    return sync_result;
}

// ============ BACKUP OPERATIONS ============

int handle_backups_list_command(int argc, char *argv[]) 
{
    linked_folders_t folders = {0};
    backup_list_t backup_list = {0};
    
    printf("DEBUG: Entered handle_backups_list_command\n");
    
    if (argc < 1) 
    {
        printf("DEBUG: No link_id provided, showing all backups\n");
        
        if (load_config(&folders) != 0) 
        {
            fprintf(stderr, "Error: Cannot load configuration\n");
            return 1;
        }
        
        if (folders.count == 0) 
        {
            printf("No linked folders found\n");
            free(folders.links);
            return 0;
        }
        
        for (int i = 0; i < folders.count; i++) 
        {
            memset(&backup_list, 0, sizeof(backup_list));
            if (list_backups(folders.links[i].id, &backup_list) != 0) 
            {
                fprintf(stderr, "Error: Cannot list backups for link: %s\n", folders.links[i].id);
            }
            free_backup_list(&backup_list);
        }
        
        free(folders.links);
        return 0;
    }
    
    const char *link_id = argv[0];
    printf("DEBUG: Listing backups for link: %s\n", link_id);
    
    memset(&backup_list, 0, sizeof(backup_list));
    if (list_backups(link_id, &backup_list) != 0) 
    {
        fprintf(stderr, "Error: Cannot list backups\n");
        free_backup_list(&backup_list);
        return 1;
    }
    
    free_backup_list(&backup_list);
    return 0;
}


int handle_backup_command(int argc, char *argv[]) 
{
    linked_folders_t folders = {0};
    const char *target_path = NULL;
    folder_link_t *link = NULL;
    char backup_id[256];
    
    printf("DEBUG: Entered handle_backup_command\n");
    
    if (argc < 1) 
    {
        fprintf(stderr, "Usage: dualsync backup <target_path>\n");
        return 1;
    }
    
    target_path = argv[0];
    printf("DEBUG: Backing up target: %s\n", target_path);
    
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return 1;
    }
    
    for (int i = 0; i < folders.count; i++) 
    {
        if (strcmp(folders.links[i].ubuntu_path, target_path) == 0 ||
            strcmp(folders.links[i].windows_path, target_path) == 0) {
            link = &folders.links[i];
            break;
        }
    }
    
    if (link == NULL) 
    {
        fprintf(stderr, "Error: No link found for path: %s\n", target_path);
        free(folders.links);
        return 1;
    }
    
    printf("DEBUG: Found link: %s\n", link->id);

    char numeric_link_id[256];
    const char *id_ptr = link->id;
    if (strncmp(id_ptr, "link_", 5) == 0) 
    {
        strncpy(numeric_link_id, id_ptr + 5, sizeof(numeric_link_id) - 1);  // Skip "link_"
    } 
    else 
    {
        strncpy(numeric_link_id, id_ptr, sizeof(numeric_link_id) - 1);
    }
    numeric_link_id[sizeof(numeric_link_id) - 1] = '\0';
    
    if (create_backup(target_path, numeric_link_id, backup_id) != 0) 
    {
        fprintf(stderr, "Error: Failed to create backup\n");
        free(folders.links);
        return 1;
    }
    
    free(folders.links);
    return 0;
}

int handle_restore_command(int argc, char *argv[]) 
{
    const char *backup_id = NULL;
    
    printf("DEBUG: Entered handle_restore_command\n");
    
    if (argc < 1) 
    {
        fprintf(stderr, "Usage: dualsync restore <backup_id>\n");
        fprintf(stderr, "Example: dualsync restore link_1762187634_Windows_20251117_153000\n");
        return 1;
    }
    
    backup_id = argv[0];
    printf("DEBUG: Restoring backup: %s\n", backup_id);
    
    int result = restore_backup(backup_id);
    
    if (result == 1) 
    {
        printf("Restore cancelled by user\n");
        return 0;
    } 
    else if (result != 0) 
    {
        fprintf(stderr, "Error: Failed to restore backup\n");
        return 1;
    }
    
    return 0;
}

int handle_backups_clean_command(int argc, char *argv[]) 
{
    const char *link_id = NULL;
    
    printf("DEBUG: Entered handle_backups_clean_command\n");
    
    if (argc < 1) 
    {
        fprintf(stderr, "Usage: dualsync backups-clean <link_id>\n");
        return 1;
    }
    
    link_id = argv[0];
    printf("DEBUG: Cleaning backups for link: %s\n", link_id);
    
    int result = cleanup_backups(link_id);
    
    if (result == 1) 
    {
        printf("Cleanup cancelled by user\n");
        return 0;
    } 
    else if (result != 0) 
    {
        fprintf(stderr, "Error: Failed to clean backups\n");
        return 1;
    }
    
    return 0;
}

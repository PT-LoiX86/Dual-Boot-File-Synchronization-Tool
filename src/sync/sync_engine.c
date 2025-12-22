#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <asm-generic/fcntl.h>

#include "../../include/filesystem.h"
#include "../../include/cli.h"
#include "../../include/backup.h"
#include "../../include/sync.h"
#include "../../include/logger.h"
#include "../../include/converter.h"


static int ensure_directory_exists(const char *dirpath) 
{
    struct stat stat_buf;
    
    if (stat(dirpath, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) 
    {
        return 0;
    }
    
    char command[PATH_MAX + 32];
    snprintf(command, sizeof(command), "mkdir -p '%s'", dirpath);
    
    if (system(command) != 0) 
    {
        return -1;
    }
    
    return 0;
}

static int copy_file(const char *source, const char *dest) 
{
    char command[PATH_MAX * 2 + 32];
    
    //printf("DEBUG: Copying file: %s → %s\n", source, dest);
    
    char dest_dir[PATH_MAX];
    strncpy(dest_dir, dest, sizeof(dest_dir) - 1);
    char *dir = dirname(dest_dir);
    
    if (ensure_directory_exists(dir) != 0) 
    {
        fprintf(stderr, "Error: Cannot create destination directory: %s\n", dir);
        return -1;
    }
    
    snprintf(command, sizeof(command), "cp -p '%s' '%s'", source, dest);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Cannot copy file: %s\n", source);
        return -1;
    }
    
    return 0;
}

static int delete_file(const char *filepath) 
{
    char command[PATH_MAX + 16];
    
    //printf("DEBUG: Deleting file: %s\n", filepath);
    
    snprintf(command, sizeof(command), "rm -f '%s'", filepath);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Cannot delete file: %s\n", filepath);
        return -1;
    }
    
    return 0;
}

static int rename_file_with_timestamp(const char *filepath) 
{
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[32];
    char new_path[PATH_MAX];
    char command[PATH_MAX * 2 + 32];
    
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    
    char temp_path[PATH_MAX];
    strncpy(temp_path, filepath, sizeof(temp_path) - 1);
    
    char *dir = dirname(temp_path);
    char temp_path2[PATH_MAX];
    strncpy(temp_path2, filepath, sizeof(temp_path2) - 1);
    char *base = basename(temp_path2);
    
    char *ext = strrchr(base, '.');
    if (ext == NULL) 
    {
        snprintf(new_path, sizeof(new_path), "%s/%s_%s", dir, base, timestamp);
    } 
    else 
    {
        char name_only[PATH_MAX];
        strncpy(name_only, base, ext - base);
        name_only[ext - base] = '\0';
        snprintf(new_path, sizeof(new_path), "%s/%s_%s%s", dir, name_only, timestamp, ext);
    }
    
    //printf("DEBUG: Renaming file: %s → %s\n", filepath, new_path);
    
    snprintf(command, sizeof(command), "mv '%s' '%s'", filepath, new_path);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Cannot rename file: %s\n", filepath);
        return -1;
    }
    
    return 0;
}

int perform_sync(folder_link_t *link, sync_operation_t operation,
                 conflict_resolution_t *resolutions, int resolution_count) 
{
    const char *source_path = NULL;
    const char *target_path = NULL;
    sync_changes_t *changes = NULL;
    int total_items = 0;
    int current_item = 0;
    int error_count = 0;
    
    //printf("DEBUG: perform_sync called\n");
    //printf("DEBUG: Operation: %d\n", operation);
    
    if (operation == SYNC_OP_TO_WINDOWS) 
    {
        source_path = link->ubuntu_path;
        target_path = link->windows_path;
        //printf("DEBUG: Syncing Ubuntu → Windows\n");
    } 
    else 
    {
        source_path = link->windows_path;
        target_path = link->ubuntu_path;
        //printf("DEBUG: Syncing Windows → Ubuntu\n");
    }
    
    //printf("DEBUG: Verifying folder link...\n");
    if (verify_folder_link_accessible(link) != 0) 
    {
        fprintf(stderr, "Error: Cannot access one or both folders\n");
        return -1;
    }
    
    //printf("DEBUG: Detecting changes...\n");
    changes = create_sync_changes();
    if (changes == NULL) 
    {
        fprintf(stderr, "Error: Cannot create sync changes structure\n");
        return -1;
    }
    
    if (detect_changes(source_path, target_path, changes) != 0) 
    {
        fprintf(stderr, "Error: Cannot detect changes\n");
        free_sync_changes(changes);
        return -1;
    }

    printf("\nDEBUG: Creating backup before sync\n");
    if (backup_before_sync(link, operation) != 0) 
    {
        fprintf(stderr, "Error: Failed to create backup\n");
        free_sync_changes(changes);
        return -1; 
    }

    
    total_items = changes->count;
    //printf("DEBUG: Total items to process: %d\n", total_items);
    
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("                   SYNCING IN PROGRESS\n");
    printf("════════════════════════════════════════════════════════════\n\n");
    
    for (int i = 0; i < changes->count; i++) 
    {
        file_change_t *change = &changes->changes[i];
        char source_file[PATH_MAX];
        char target_file[PATH_MAX];
        int progress_percent = 0;
        
        current_item++;
        progress_percent = (current_item * 100) / total_items;
        
        printf("Progress: [");
        for (int j = 0; j < 50; j++) {
            if (j < (progress_percent / 2)) 
            {
                printf("=");
            } 
            else 
            {
                printf(" ");
            }
        }
        printf("] %d%% (%d/%d)\r", progress_percent, current_item, total_items);
        fflush(stdout);
        
        snprintf(source_file, sizeof(source_file), "%s/%s", source_path, change->path);
        snprintf(target_file, sizeof(target_file), "%s/%s", target_path, change->path);
        
        int operation_result = 0;
        
        switch (change->status) 
        {
            int resolved = 0;

            case FILE_STATUS_NEW:
                printf("\nAdding: %s\n", change->path);
                operation_result = copy_file(source_file, target_file);
                break;
                
            case FILE_STATUS_MODIFIED:    
                for (int j = 0; j < resolution_count; j++) 
                {
                    if (strcmp(resolutions[j].file->path, change->path) == 0)
                    {
                        if (resolutions[j].choice == CONFLICT_CHOICE_OVERWRITE) 
                        {
                            printf("\nUpdating (overwrite): %s\n", change->path);
                            operation_result = copy_file(source_file, target_file);
                        } 
                        else if (resolutions[j].choice == CONFLICT_CHOICE_KEEP_BOTH) 
                        {
                            printf("\nUpdating (keep both): %s\n", change->path);
                            operation_result = rename_file_with_timestamp(target_file);
                            if (operation_result == 0) 
                            {
                                operation_result = copy_file(source_file, target_file);
                            }
                        }
                        resolved = 1;
                        break;
                    }
                }
                
                if (!resolved) 
                {
                    printf("\nUpdating: %s\n", change->path);
                    operation_result = copy_file(source_file, target_file);
                }
                break;
                
            case FILE_STATUS_DELETED:
                printf("\nDeleting: %s\n", change->path);
                
                printf("Confirm delete? (yes/no): ");
                char response[10];
                if (fgets(response, sizeof(response), stdin) == NULL ||
                    (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0)) 
                {
                    printf("Skipping delete\n");
                    operation_result = 0;
                } 
                else 
                {
                    operation_result = delete_file(target_file);
                }
                break;
                
            case FILE_STATUS_UNCHANGED:
                continue;
                
            default:
                fprintf(stderr, "Unknown file status: %d\n", change->status);
                operation_result = -1;
        }
        
        if (operation_result != 0) 
        {
            error_count++;
            
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Failed to process file");
            
            int user_choice = display_sync_error_prompt(change->path, error_msg);
            
            if (user_choice != 0) 
            {
                printf("\n\nSync aborted by user\n");
                free_sync_changes(changes);
                return -1;
            }
        }
    }

    if (verify_sync_completion(target_path, changes) != 0) 
    {
        fprintf(stderr, "Warning: Sync completion verification failed\n");
    }
    
    if (update_sync_link(link, changes) != 0) 
    {
        fprintf(stderr, "Warning: Cannot update link information\n");
    }
    
    display_sync_summary(changes, source_path, target_path, error_count);
    
    char details[512];
    int files_added = changes->new_count;
    int files_updated = changes->modified_count;
    int files_deleted = changes->deleted_count;

    snprintf(details, sizeof(details), 
            "Direction: %s | Added: %d, Updated: %d, Deleted: %d",
            (operation == SYNC_OP_TO_WINDOWS) ? "Ubuntu→Windows" : "Windows→Ubuntu",
            files_added, files_updated, files_deleted);

    if (error_count == 0) 
    {
        log_operation(LOG_OP_SYNC, link->id, LOG_STATUS_SUCCESS, details);
    } 
    else 
    {
        log_operation(LOG_OP_SYNC, link->id, LOG_STATUS_FAILURE, details);
    }

    conversion_mappings_t mappings = {0};

    const char *sync_direction = (operation == SYNC_OP_TO_WINDOWS) ? 
                                "ubuntu_to_windows" : "windows_to_ubuntu";
    
    if (load_conversion_mappings(sync_direction, &mappings) == 0) 
    {
        if (needs_conversion(target_path, &mappings)) 
        {
            convertible_files_list_t files_list = {0};
            if (find_convertible_files(target_path, &mappings, &files_list) == 0 &&
                files_list.count > 0) 
                {
                
                display_conversion_prompt(&files_list);
                
                char response[10];
                if (fgets(response, sizeof(response), stdin) != NULL &&
                    (strcmp(response, "yes\n") == 0 || strcmp(response, "y\n") == 0)) 
                {
                    convert_files(target_path, &files_list, &mappings, link->id);
                }
                
                free_convertible_files_list(&files_list);
            }
        }
        
        free_conversion_mappings(&mappings);
    }
  
    free_sync_changes(changes);
    
    if (error_count > 0) 
    {
        printf("\nDEBUG: Sync had errors, attempting auto-restore\n");
        if (restore_on_sync_failure(link, operation) != 0) 
        {
            fprintf(stderr, "CRITICAL: Both sync and restore failed!\n");
            fprintf(stderr, "Manual intervention required.\n");
        }
        return -1;
    }
    
    return 0;
}

// POST SYNC

int update_sync_link(folder_link_t *link, sync_changes_t *changes) 
{
    if (link == NULL || changes == NULL) 
    {
        return -1;
    }
    
    //printf("DEBUG: Updating sync link information\n");
    
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    char timestamp[64];
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    strncpy(link->last_sync, timestamp, sizeof(link->last_sync) - 1);
    
    //printf("DEBUG: Updated last_sync: %s\n", link->last_sync);
    
    return 0;
}

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

int save_sync_config(linked_folders_t *folders, folder_link_t *updated_link) 
{
    if (folders == NULL || updated_link == NULL) 
    {
        return -1;
    }
    
    //printf("DEBUG: Saving updated configuration\n");
    
    for (int i = 0; i < folders->count; i++) 
    {
        if (strcmp(folders->links[i].id, updated_link->id) == 0) 
        {
            folders->links[i] = *updated_link;
            //printf("DEBUG: Found and updated link at index %d\n", i);
            break;
        }
    }
    
    if (save_config(folders) != 0) 
    {
        fprintf(stderr, "Warning: Cannot save configuration after sync\n");
        return -1;
    }
    
    //printf("DEBUG: Configuration saved successfully\n");
    return 0;
}

int verify_sync_completion(const char *target_path, sync_changes_t *changes) 
{
    if (target_path == NULL || changes == NULL) 
    {
        return -1;
    }
    
    //printf("DEBUG: Verifying sync completion\n");
    
    struct stat stat_buf;
    if (stat(target_path, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Target folder not accessible after sync: %s\n", target_path);
        return -1;
    }
    
    if (!S_ISDIR(stat_buf.st_mode)) 
    {
        fprintf(stderr, "Error: Target path is not a directory\n");
        return -1;
    }
    
    //printf("DEBUG: Sync verification successful\n");
    return 0;
}
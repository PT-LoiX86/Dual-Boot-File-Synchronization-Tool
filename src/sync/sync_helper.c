#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../include/filesystem.h"
#include "../../include/sync.h"
#include "../../include/utils.h"

int is_folder_empty(const char *folder_path) 
{
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;
    
    //printf("DEBUG: Checking if folder is empty: %s\n", folder_path);
    
    dir = opendir(folder_path);
    if (dir == NULL) {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            file_count++;
            break;
        }
    }
    
    closedir(dir);
    //printf("DEBUG: Folder empty: %s\n", file_count == 0 ? "yes" : "no");
    
    return file_count == 0 ? 1 : 0;
}

static void get_relative_path(const char *base_path, const char *full_path,
                             char *relative_path, size_t max_len) 
{
    size_t base_len = strlen(base_path);
    if (strncmp(full_path, base_path, base_len) == 0) {
        const char *rel = full_path + base_len;
        if (rel[0] == '/') rel++;
        strncpy(relative_path, rel, max_len - 1);
        relative_path[max_len - 1] = '\0';
    } else {
        strncpy(relative_path, full_path, max_len - 1);
        relative_path[max_len - 1] = '\0';
    }
}

static int scan_folder_recursive(const char *folder_path, const char *base_path,
                                 sync_changes_t *changes, int is_source) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;
    char full_path[PATH_MAX];
    char relative_path[PATH_MAX];
    
    //printf("DEBUG: Scanning folder: %s (source=%d)\n", folder_path, is_source);
    
    dir = opendir(folder_path);
    if (dir == NULL) 
    {
        fprintf(stderr, "Error: Cannot open directory: %s\n", folder_path);
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, entry->d_name);
        
        if (stat(full_path, &stat_buf) != 0) 
        {
            fprintf(stderr, "Warning: Cannot stat file: %s\n", full_path);
            continue;
        }
        
        get_relative_path(base_path, full_path, relative_path, sizeof(relative_path));
        
        //printf("DEBUG: Found %s: %s\n", S_ISDIR(stat_buf.st_mode) ? "dir" : "file", relative_path);
        
        if (S_ISDIR(stat_buf.st_mode)) 
        {
            if (scan_folder_recursive(full_path, base_path, changes, is_source) != 0) 
            {
                closedir(dir);
                return -1;
            }
        } 
        else if (S_ISREG(stat_buf.st_mode)) 
        {
            file_change_t change = {0};
            strncpy(change.path, relative_path, sizeof(change.path) - 1);
            change.size = stat_buf.st_size;
            change.mtime = stat_buf.st_mtime;
            change.status = FILE_STATUS_UNCHANGED;
            change.conflict = CONFLICT_NONE;
            
            if (calculate_file_md5(full_path, change.md5_hash) != 0) 
            {
                fprintf(stderr, "Warning: Cannot calculate MD5 for: %s\n", full_path);
                strcpy(change.md5_hash, "");
            }
            
            add_change(changes, &change);
        }
    }
    
    closedir(dir);
    return 0;
}

int scan_folder_for_changes(const char *folder_path, const char *other_folder_path,
                            sync_changes_t *changes) 
{
    struct stat stat_buf;
    
    //printf("DEBUG: scan_folder_for_changes called\n");
    //printf("DEBUG: Source folder: %s\n", folder_path);
    //printf("DEBUG: Other folder: %s\n", other_folder_path);
    
    if (folder_path == NULL || changes == NULL) 
    {
        fprintf(stderr, "Error: Invalid parameters to scan_folder_for_changes\n");
        return -1;
    }
    
    if (stat(folder_path, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Cannot access folder: %s\n", folder_path);
        return -1;
    }
    
    if (!S_ISDIR(stat_buf.st_mode)) 
    {
        fprintf(stderr, "Error: Not a directory: %s\n", folder_path);
        return -1;
    }
    
    if (scan_folder_recursive(folder_path, folder_path, changes, 1) != 0) 
    {
        fprintf(stderr, "Error: Failed to scan folder\n");
        return -1;
    }
    
    //printf("DEBUG: Scan complete. Found %d items\n", changes->count);
    return 0;
}

int detect_changes(const char *source_path, const char *target_path,
                   sync_changes_t *changes) 
{
    sync_changes_t *source_files = NULL;
    sync_changes_t *target_files = NULL;
    
    //printf("DEBUG: detect_changes called\n");
    //printf("DEBUG: Source: %s\n", source_path);
    //printf("DEBUG: Target: %s\n", target_path);
    
    source_files = create_sync_changes();
    if (source_files == NULL) 
    {
        fprintf(stderr, "Error: Cannot create sync changes structure\n");
        return -1;
    }
    
    if (scan_folder_for_changes(source_path, target_path, source_files) != 0) 
    {
        fprintf(stderr, "Error: Cannot scan source folder\n");
        free_sync_changes(source_files);
        return -1;
    }
    
    //printf("DEBUG: Source files: %d\n", source_files->count);
    
    target_files = create_sync_changes();
    if (target_files == NULL) 
    {
        fprintf(stderr, "Error: Cannot create sync changes structure\n");
        free_sync_changes(source_files);
        return -1;
    }
    
    if (scan_folder_for_changes(target_path, source_path, target_files) != 0) 
    {
        fprintf(stderr, "Error: Cannot scan target folder\n");
        free_sync_changes(source_files);
        free_sync_changes(target_files);
        return -1;
    }
    
    //printf("DEBUG: Target files: %d\n", target_files->count);
    
    for (int i = 0; i < source_files->count; i++) 
    {
        file_change_t *source_file = &source_files->changes[i];
        file_change_t *target_file = NULL;
        
        for (int j = 0; j < target_files->count; j++) 
        {
            if (strcmp(target_files->changes[j].path, source_file->path) == 0) 
            {
                target_file = &target_files->changes[j];
                break;
            }
        }
        
        if (target_file == NULL) 
        {
            source_file->status = FILE_STATUS_NEW;
            //printf("DEBUG: NEW file: %s\n", source_file->path);
        } 
        else 
        {
            if (strcmp(source_file->md5_hash, target_file->md5_hash) != 0) 
            {
                source_file->status = FILE_STATUS_MODIFIED;
                source_file->conflict = CONFLICT_CONTENT_DIFF;
                //printf("DEBUG: MODIFIED file: %s\n", source_file->path);
            } 
            else 
            {
                source_file->status = FILE_STATUS_UNCHANGED;
                //printf("DEBUG: UNCHANGED file: %s\n", source_file->path);
            }
            
            target_file->status = FILE_STATUS_UNCHANGED;
        }
        
        add_change(changes, source_file);
    }
    
    for (int j = 0; j < target_files->count; j++) 
    {
        file_change_t *target_file = &target_files->changes[j];
        
        int found_in_source = 0;
        for (int i = 0; i < changes->count; i++) 
        {
            if (strcmp(changes->changes[i].path, target_file->path) == 0) 
            {
                found_in_source = 1;
                break;
            }
        }
        
        if (!found_in_source) 
        {
            target_file->status = FILE_STATUS_DELETED;
            //printf("DEBUG: DELETED file: %s\n", target_file->path);
            add_change(changes, target_file);
        }
    }
    
    //printf("DEBUG: Change detection complete\n");
    //printf("DEBUG: New: %d, Modified: %d, Deleted: %d, Conflicts: %d\n",
           //changes->new_count, changes->modified_count, 
           //changes->deleted_count, changes->conflict_count);
    
    free_sync_changes(source_files);
    free_sync_changes(target_files);
    
    return 0;
}

sync_changes_t* create_sync_changes(void) 
{
    sync_changes_t *changes = malloc(sizeof(sync_changes_t));
    if (changes == NULL) 
    {
        return NULL;
    }
    
    changes->capacity = 100;
    changes->changes = malloc(sizeof(file_change_t) * changes->capacity);
    
    if (changes->changes == NULL) 
    {
        free(changes);
        return NULL;
    }
    
    changes->count = 0;
    changes->new_count = 0;
    changes->modified_count = 0;
    changes->deleted_count = 0;
    changes->conflict_count = 0;
    changes->new_size = 0;
    changes->modified_size = 0;
    changes->deleted_size = 0;
    
    return changes;
}

void free_sync_changes(sync_changes_t *changes) 
{
    if (changes == NULL) 
    {
        return;
    }
    
    if (changes->changes != NULL) 
    {
        free(changes->changes);
    }
    
    free(changes);
}

void add_change(sync_changes_t *changes, const file_change_t *change) 
{
    if (changes == NULL || change == NULL) 
    {
        return;
    }
    
    if (changes->count >= changes->capacity) 
    {
        changes->capacity *= 2;
        file_change_t *new_changes = realloc(changes->changes, 
                                             sizeof(file_change_t) * changes->capacity);
        if (new_changes == NULL) 
        {
            fprintf(stderr, "Error: Cannot expand changes array\n");
            return;
        }
        changes->changes = new_changes;
    }
    
    changes->changes[changes->count] = *change;
    
    switch (change->status) 
    {
        case FILE_STATUS_NEW:
            changes->new_count++;
            changes->new_size += change->size;
            break;
        case FILE_STATUS_MODIFIED:
            changes->modified_count++;
            changes->modified_size += change->size;
            break;
        case FILE_STATUS_DELETED:
            changes->deleted_count++;
            changes->deleted_size += change->size;
            break;
        case FILE_STATUS_UNCHANGED:
            break;
    }
    
    if (change->conflict != CONFLICT_NONE) 
    {
        changes->conflict_count++;
    }
    
    changes->count++;
}

int restore_on_sync_failure(folder_link_t *link, sync_operation_t operation)
{
    const char *location = NULL;
    char backup_id[256];
    
    //printf("DEBUG: restore_on_sync_failure called\n");
    
    if (link == NULL) 
    {
        fprintf(stderr, "Error: Invalid link\n");
        return -1;
    }
    
    if (operation == SYNC_OP_TO_WINDOWS) 
    {
        location = "Windows";
    } 
    else 
    {
        location = "Ubuntu";
    }
    
    //printf("DEBUG: Finding latest backup for location: %s\n", location);
    
    if (get_latest_backup(link->id, location, backup_id) != 0) 
    {
        fprintf(stderr, "Error: No backup found for restoration\n");
        return -1;
    }
    
    //printf("DEBUG: Found latest backup: %s\n", backup_id);
    
    display_auto_restore_start(backup_id);
    
    if (restore_backup_silent(backup_id) != 0) 
    {
        fprintf(stderr, "Error: Failed to restore backup\n");
        fprintf(stderr, "CRITICAL: Sync failed AND restore failed!\n");
        fprintf(stderr, "Target folder may be in inconsistent state.\n");
        fprintf(stderr, "Please manually restore from backup: %s\n", backup_id);
        display_auto_restore_failed(backup_id);
        return -1;
    }
    
    display_auto_restore_successful(backup_id);
    
    return 0;
}

int backup_before_sync(folder_link_t *link, sync_operation_t operation) 
{
    const char *target_path = NULL;
    char backup_id[256];
    
    //printf("DEBUG: backup_before_sync called\n");
    
    if (link == NULL) 
    {
        fprintf(stderr, "Error: Invalid link\n");
        return -1;
    }
    
    if (operation == SYNC_OP_TO_WINDOWS) 
    {
        target_path = link->windows_path;
    } 
    else 
    {
        target_path = link->ubuntu_path;
    }
    
    //printf("DEBUG: Backing up target folder: %s\n", target_path);
    
    if (create_backup(target_path, link->id, backup_id) != 0) 
    {
        fprintf(stderr, "Error: Failed to create backup\n");
        return -1;
    }
    
    //printf("DEBUG: Backup created successfully: %s\n", backup_id);
    
    return 0;
}
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include "../../include/backup.h"
#include "../../include/cli.h"
#include "../../include/config.h"
#include "../../include/logger.h"

static int get_backup_directory(const char *link_id, const char *location, 
                                char *backup_dir, size_t max_len) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    snprintf(backup_dir, max_len, "%s/.dualsync/backups/link_%s/%s", 
             home, link_id, location);
    
    printf("DEBUG: Backup directory: %s\n", backup_dir);
    return 0;
}


static int ensure_backup_directory_exists(const char *backup_dir) 
{
    char command[PATH_MAX + 32];
    struct stat stat_buf;
    
    if (stat(backup_dir, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) 
    {
        return 0;
    }
    
    snprintf(command, sizeof(command), "mkdir -p '%s'", backup_dir);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Cannot create backup directory: %s\n", backup_dir);
        return -1;
    }
    
    return 0;
}

static void generate_timestamp(char *timestamp, size_t max_len) 
{
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    strftime(timestamp, max_len, "%Y%m%d_%H%M%S", timeinfo);
}

static int determine_location(const char *target_path, char *location, size_t max_len) 
{
    if (strstr(target_path, "/media/") != NULL || 
        strstr(target_path, "/mnt/") != NULL ||
        strstr(target_path, "/run/") != NULL) 
    {
        strncpy(location, "Windows", max_len - 1);
    } 
    else if (strstr(target_path, "/home/") != NULL) 
    {
        strncpy(location, "Ubuntu", max_len - 1);
    } 
    else 
    {
        strncpy(location, "Ubuntu", max_len - 1);
    }
    
    location[max_len - 1] = '\0';
    return 0;
}

int parse_backup_id(const char *backup_id, backup_id_t *parsed) 
{
    if (backup_id == NULL || parsed == NULL) 
    {
        return -1;
    }
    
    printf("DEBUG: Parsing backup ID: %s\n", backup_id);
    
    // Format: link_<link_id>_<location>_<timestamp>
    // Example: link_1762187634_Windows_20251117_153000
    
    char temp[256];
    strncpy(temp, backup_id, sizeof(temp) - 1);
    
    char *ptr = temp;
    
    if (strncmp(ptr, "link_", 5) != 0) 
    {
        fprintf(stderr, "Error: Invalid backup ID format (missing 'link_' prefix)\n");
        return -1;
    }
    ptr += 5;
    
    char *underscore = strchr(ptr, '_');
    if (underscore == NULL) 
    {
        fprintf(stderr, "Error: Invalid backup ID format (no location found)\n");
        return -1;
    }
    
    int link_id_len = underscore - ptr;
    strncpy(parsed->link_id, ptr, link_id_len);
    parsed->link_id[link_id_len] = '\0';
    
    ptr = underscore + 1;
    
    underscore = strchr(ptr, '_');
    if (underscore == NULL) 
    {
        fprintf(stderr, "Error: Invalid backup ID format (no timestamp found)\n");
        return -1;
    }
    
    int location_len = underscore - ptr;
    strncpy(parsed->location, ptr, location_len);
    parsed->location[location_len] = '\0';
    
    ptr = underscore + 1;
    
    strncpy(parsed->timestamp, ptr, sizeof(parsed->timestamp) - 1);
    parsed->timestamp[sizeof(parsed->timestamp) - 1] = '\0';
    
    printf("DEBUG: Parsed - link_id: %s, location: %s, timestamp: %s\n",
           parsed->link_id, parsed->location, parsed->timestamp);
    
    return 0;
}


int get_backup_path(const char *backup_id, char *backup_path, size_t max_len) 
{
    backup_id_t parsed;
    char backup_dir[PATH_MAX];
    
    if (parse_backup_id(backup_id, &parsed) != 0) 
    {
        return -1;
    }
    
    if (get_backup_directory(parsed.link_id, parsed.location, backup_dir, sizeof(backup_dir)) != 0) 
    {
        return -1;
    }
    
    snprintf(backup_path, max_len, "%s/%s.tar.gz", backup_dir, parsed.timestamp);
    
    return 0;
}

int create_backup(const char *target_path, const char *link_id, 
                  char *backup_id_out) 
{
    char backup_dir[PATH_MAX];
    char location[16];
    char timestamp[32];
    char command[PATH_MAX * 2 + 64];
    char backup_file[PATH_MAX];
    struct stat stat_buf;
    
    if (determine_location(target_path, location, sizeof(location)) != 0) 
    {
        return -1;
    }
    
    if (get_backup_directory(link_id, location, backup_dir, sizeof(backup_dir)) != 0)
    {
        return -1;
    }
    
    if (ensure_backup_directory_exists(backup_dir) != 0) 
    {
        return -1;
    }
    
    if (stat(target_path, &stat_buf) != 0 || !S_ISDIR(stat_buf.st_mode)) 
    {
        fprintf(stderr, "Error: Target path is not a directory: %s\n", target_path);
        return -1;
    }
    
    generate_timestamp(timestamp, sizeof(timestamp));
    
    snprintf(backup_file, sizeof(backup_file), "%s/%s.tar.gz", backup_dir, timestamp);
    
    char parent_dir[PATH_MAX];
    char folder_name[PATH_MAX];
    strncpy(parent_dir, target_path, sizeof(parent_dir) - 1);
    strncpy(folder_name, target_path, sizeof(folder_name) - 1);
    
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash != NULL) 
    {
        *last_slash = '\0';
        strncpy(folder_name, last_slash + 1, sizeof(folder_name) - 1);
    }
    
    snprintf(command, sizeof(command), 
             "cd '%s' && tar -czf '%s' '%s' 2>&1", 
             parent_dir, backup_file, folder_name);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Failed to create backup\n");
        return -1;
    }
    
    if (stat(backup_file, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Backup file not created\n");

        log_operation(LOG_OP_BACKUP, link_id, LOG_STATUS_FAILURE, 
                  "Failed to create tar archive");

        return -1;
    }
    
    snprintf(backup_id_out, 256, "link_%s_%s_%s", link_id, location, timestamp);
    
    char details[256];
    snprintf(details, sizeof(details), "Location: %s | Size: %.2f MB", 
            location, stat_buf.st_size / (1024.0 * 1024.0));
    log_operation(LOG_OP_BACKUP, link_id, LOG_STATUS_SUCCESS, details);

    display_backup_created(backup_id_out, backup_file, stat_buf.st_size);
    
    return 0;
}

int restore_backup(const char *backup_id) 
{
    backup_id_t parsed;
    char backup_path[PATH_MAX];
    char command[PATH_MAX * 2 + 64];
    struct stat stat_buf;
    linked_folders_t folders = {0};
    folder_link_t *link = NULL;
    const char *target_path = NULL;
    char parent_dir[PATH_MAX];
    
    if (parse_backup_id(backup_id, &parsed) != 0) 
    {
        return -1;
    }
    
    if (get_backup_path(backup_id, backup_path, sizeof(backup_path)) != 0) 
    {
        return -1;
    }
    
    if (stat(backup_path, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Backup file not found: %s\n", backup_path);
        return -1;
    }
    
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return -1;
    }
    
    for (int i = 0; i < folders.count; i++) {
        if (strcmp(folders.links[i].id, parsed.link_id) == 0) 
        {
            link = &folders.links[i];
            break;
        }
        
        char search_id[256];
        snprintf(search_id, sizeof(search_id), "link_%s", parsed.link_id);
        if (strcmp(folders.links[i].id, search_id) == 0) 
        {
            link = &folders.links[i];
            break;
        }
    }

    if (link == NULL) 
    {
        fprintf(stderr, "Error: Link not found: %s\n", parsed.link_id);
        fprintf(stderr, "Available links:\n");
        for (int i = 0; i < folders.count; i++) 
        {
            fprintf(stderr, "  - %s\n", folders.links[i].id);
        }
        free(folders.links);
        return -1;
    }

    
    if (strcmp(parsed.location, "Windows") == 0) 
    {
        target_path = link->windows_path;
    } 
    else 
    {
        target_path = link->ubuntu_path;
    }
    
    strncpy(parent_dir, target_path, sizeof(parent_dir) - 1);
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash != NULL) 
    {
        *last_slash = '\0';
    }
    
    display_restore_confirmation(backup_id, target_path);
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL ||
        (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0)) 
    {
        printf("Restore cancelled\n");
        free(folders.links);
        return 1;
    }
    
    char rm_command[PATH_MAX + 16];
    snprintf(rm_command, sizeof(rm_command), "rm -rf '%s'", target_path);
    if (system(rm_command) != 0) 
    {
        fprintf(stderr, "Error: Cannot remove existing folder\n");
        free(folders.links);
        return -1;
    }
    
    snprintf(command, sizeof(command), "cd '%s' && tar -xzf '%s' 2>&1", 
             parent_dir, backup_path);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Failed to restore backup\n");

        log_operation(LOG_OP_RESTORE, link->id, LOG_STATUS_FAILURE, 
                  "Failed to extract backup archive");

        free(folders.links);
        return -1;
    }

    char details[256];
    snprintf(details, sizeof(details), "Restored to: %s | From: %s", 
            target_path, backup_id);
    log_operation(LOG_OP_RESTORE, link->id, LOG_STATUS_SUCCESS, details);
    
    display_restore_successful(backup_id, target_path);
    
    free(folders.links);
    return 0;
}

int backup_folder_exists(const char *link_id, const char *location) 
{
    char backup_dir[PATH_MAX];
    struct stat stat_buf;
    
    if (get_backup_directory(link_id, location, backup_dir, sizeof(backup_dir)) != 0) 
    {
        return 0;
    }
    
    return (stat(backup_dir, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode)) ? 1 : 0;
}

int list_backups(const char *link_id, backup_list_t *backup_list) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    char backups_base_dir[PATH_MAX];
    snprintf(backups_base_dir, sizeof(backups_base_dir), "%s/.dualsync/backups/%s", 
             home, link_id);
    
    struct stat stat_buf;
    if (stat(backups_base_dir, &stat_buf) != 0) 
    {
        printf("No backups found for link: %s\n", link_id);
        backup_list->backups = NULL;
        backup_list->count = 0;
        return 0;
    }
    
    backup_list->backups = NULL;
    backup_list->count = 0;
    int total_count = 0;
    
    backup_info_t temp_backups[100];
    memset(temp_backups, 0, sizeof(temp_backups));
    
    char windows_dir[PATH_MAX];
    char ubuntu_dir[PATH_MAX];
    snprintf(windows_dir, sizeof(windows_dir), "%s/Windows", backups_base_dir);
    snprintf(ubuntu_dir, sizeof(ubuntu_dir), "%s/Ubuntu", backups_base_dir);
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(windows_dir);
    if (dir != NULL) 
    {
        while ((entry = readdir(dir)) != NULL && total_count < 100) 
        {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".tar.gz") != NULL) 
            {
                char timestamp[32] = {0};
                strncpy(timestamp, entry->d_name, sizeof(timestamp) - 1);
                char *dot = strstr(timestamp, ".tar.gz");
                if (dot != NULL) 
                {
                    *dot = '\0';
                }
                
                snprintf(temp_backups[total_count].backup_id, 
                        sizeof(temp_backups[total_count].backup_id),
                        "%s_Windows_%s", link_id, timestamp);
                
                snprintf(temp_backups[total_count].path, 
                        sizeof(temp_backups[total_count].path),
                        "%s/%s", windows_dir, entry->d_name);
                
                struct stat backup_stat;
                if (stat(temp_backups[total_count].path, &backup_stat) == 0) 
                {
                    temp_backups[total_count].size = backup_stat.st_size;
                    temp_backups[total_count].created_time = backup_stat.st_mtime;
                }
                
                total_count++;
            }
        }
        closedir(dir);
    }
    
    dir = opendir(ubuntu_dir);
    if (dir != NULL) 
    {
        while ((entry = readdir(dir)) != NULL && total_count < 100) 
        {
            if (entry->d_type == DT_REG && strstr(entry->d_name, ".tar.gz") != NULL) 
            {
                char timestamp[32] = {0};
                strncpy(timestamp, entry->d_name, sizeof(timestamp) - 1);
                char *dot = strstr(timestamp, ".tar.gz");
                if (dot != NULL) 
                {
                    *dot = '\0';
                }
                
                snprintf(temp_backups[total_count].backup_id, 
                        sizeof(temp_backups[total_count].backup_id),
                        "%s_Ubuntu_%s", link_id, timestamp);
                
                snprintf(temp_backups[total_count].path, 
                        sizeof(temp_backups[total_count].path),
                        "%s/%s", ubuntu_dir, entry->d_name);
                
                struct stat backup_stat;
                if (stat(temp_backups[total_count].path, &backup_stat) == 0) 
                {
                    temp_backups[total_count].size = backup_stat.st_size;
                    temp_backups[total_count].created_time = backup_stat.st_mtime;
                }
                
                total_count++;
            }
        }
        closedir(dir);
    }
    
    if (total_count == 0) 
    {
        printf("No backups found for link: %s\n", link_id);
        backup_list->backups = NULL;
        backup_list->count = 0;
        return 0;
    }
    
    backup_list->backups = malloc(sizeof(backup_info_t) * total_count);
    if (backup_list->backups == NULL) 
    {
        fprintf(stderr, "Error: Cannot allocate memory\n");
        return -1;
    }
    
    memcpy(backup_list->backups, temp_backups, sizeof(backup_info_t) * total_count);
    backup_list->count = total_count;
    
    printf("DEBUG: About to call display_backup_list\n");
    printf("DEBUG: backup_list->count = %d\n", backup_list->count);
    printf("DEBUG: backup_list->backups = %p\n", (void*)backup_list->backups);

    display_backup_list(link_id, backup_list);

    printf("DEBUG: Returned from display_backup_list\n");
    
    return 0;
}

void free_backup_list(backup_list_t *backup_list) 
{
    if (backup_list != NULL && backup_list->backups != NULL) 
    {
        free(backup_list->backups);
        backup_list->backups = NULL;
        backup_list->count = 0;
    }
}

int cleanup_backups(const char *link_id) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    char backups_dir[PATH_MAX];
    snprintf(backups_dir, sizeof(backups_dir), "%s/.dualsync/backups/%s", 
             home, link_id);
    
    struct stat stat_buf;
    if (stat(backups_dir, &stat_buf) != 0) 
    {
        printf("No backups found for link: %s\n", link_id);
        return 0;
    }
    
    backup_list_t backup_list = {0};
    if (list_backups(link_id, &backup_list) != 0) 
    {
        return -1;
    }
    
    if (backup_list.count == 0) 
    {
        printf("No backups found for link: %s\n", link_id);
        return 0;
    }
    
    display_cleanup_confirmation(link_id, backup_list.count);
    
    char response[10];
    if (fgets(response, sizeof(response), stdin) == NULL ||
        (strcmp(response, "yes\n") != 0 && strcmp(response, "y\n") != 0)) 
    {
        printf("Cleanup cancelled\n");

        log_operation(LOG_OP_CLEANUP, link_id, LOG_STATUS_WARNING, 
                  "Cleanup cancelled by user");
                  
        free_backup_list(&backup_list);
        return 1;
    }
    
    int deleted_count = 0;
    for (int i = 0; i < backup_list.count; i++) 
    {
        char rm_command[PATH_MAX + 16];
        snprintf(rm_command, sizeof(rm_command), "rm -f '%s'", backup_list.backups[i].path);
        
        if (system(rm_command) == 0) 
        {
            deleted_count++;
        }
    }
    
    char windows_dir[PATH_MAX];
    char ubuntu_dir[PATH_MAX];
    snprintf(windows_dir, sizeof(windows_dir), "%s/Windows", backups_dir);
    snprintf(ubuntu_dir, sizeof(ubuntu_dir), "%s/Ubuntu", backups_dir);
    
    char rmdir_command[PATH_MAX + 16];
    snprintf(rmdir_command, sizeof(rmdir_command), "rmdir '%s' 2>/dev/null", windows_dir);
    system(rmdir_command);
    
    snprintf(rmdir_command, sizeof(rmdir_command), "rmdir '%s' 2>/dev/null", ubuntu_dir);
    system(rmdir_command);
    
    snprintf(rmdir_command, sizeof(rmdir_command), "rmdir '%s' 2>/dev/null", backups_dir);
    system(rmdir_command);

    char details[256];
    snprintf(details, sizeof(details), "Deleted %d backup(s)", deleted_count);
    log_operation(LOG_OP_CLEANUP, link_id, LOG_STATUS_SUCCESS, details);
    
    display_cleanup_successful(link_id, deleted_count);
    
    free_backup_list(&backup_list);
    
    return 0;
}

int get_latest_backup(const char *link_id, const char *location, 
                      char *backup_id_out) 
{
    const char *home = getenv("HOME");
    if (home == NULL) 
    {
        fprintf(stderr, "Error: Cannot get HOME directory\n");
        return -1;
    }
    
    char backup_dir[PATH_MAX];
    snprintf(backup_dir, sizeof(backup_dir), "%s/.dualsync/backups/%s/%s", 
             home, link_id, location);
    
    DIR *dir = opendir(backup_dir);
    if (dir == NULL) 
    {
        fprintf(stderr, "Error: No backups found for location: %s\n", location);
        return -1;
    }
    
    struct dirent *entry;
    char latest_timestamp[32] = {0};
    time_t latest_time = 0;
    
    while ((entry = readdir(dir)) != NULL) 
    {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".tar.gz") != NULL) 
        {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", backup_dir, entry->d_name);
            
            struct stat stat_buf;
            if (stat(full_path, &stat_buf) == 0) 
            {
                if (stat_buf.st_mtime > latest_time) 
                {
                    latest_time = stat_buf.st_mtime;
                    
                    strncpy(latest_timestamp, entry->d_name, sizeof(latest_timestamp) - 1);
                    char *dot = strstr(latest_timestamp, ".tar.gz");
                    if (dot != NULL) {
                        *dot = '\0';
                    }
                }
            }
        }
    }
    
    closedir(dir);
    
    if (latest_timestamp[0] == '\0') 
    {
        fprintf(stderr, "Error: No backups found\n");
        return -1;
    }
    
    snprintf(backup_id_out, 256, "link_%s_%s_%s", link_id, location, latest_timestamp);
    
    return 0;
}

int backup_before_sync(folder_link_t *link, sync_operation_t operation) 
{
    const char *target_path = NULL;
    char backup_id[256];
    
    printf("DEBUG: backup_before_sync called\n");
    
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
    
    printf("DEBUG: Backing up target folder: %s\n", target_path);
    
    if (create_backup(target_path, link->id, backup_id) != 0) 
    {
        fprintf(stderr, "Error: Failed to create backup\n");
        return -1;
    }
    
    printf("DEBUG: Backup created successfully: %s\n", backup_id);
    
    return 0;
}

int restore_backup_silent(const char *backup_id) 
{
    backup_id_t parsed;
    char backup_path[PATH_MAX];
    char command[PATH_MAX * 2 + 64];
    struct stat stat_buf;
    linked_folders_t folders = {0};
    folder_link_t *link = NULL;
    const char *target_path = NULL;
    char parent_dir[PATH_MAX];
    
    printf("DEBUG: Restoring backup silently: %s\n", backup_id);
    
    if (parse_backup_id(backup_id, &parsed) != 0) 
    {
        return -1;
    }
    
    if (get_backup_path(backup_id, backup_path, sizeof(backup_path)) != 0) 
    {
        return -1;
    }
    
    if (stat(backup_path, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Backup file not found: %s\n", backup_path);
        return -1;
    }
    
    if (load_config(&folders) != 0) 
    {
        fprintf(stderr, "Error: Cannot load configuration\n");
        return -1;
    }
    
    for (int i = 0; i < folders.count; i++) 
    {
        const char *link_id_ptr = folders.links[i].id;
        
        printf("DEBUG: Checking link[%d]: '%s'\n", i, folders.links[i].id);
        
        if (strncmp(link_id_ptr, "link_", 5) == 0) 
        {
            link_id_ptr += 5;
        }
        
        printf("DEBUG: Comparing '%s' (from link) with '%s' (parsed)\n", link_id_ptr, parsed.link_id);
        
        if (strcmp(link_id_ptr, parsed.link_id) == 0) 
        {
            link = &folders.links[i];
            printf("DEBUG: Found matching link: %s\n", folders.links[i].id);
            break;
        }
    }

    if (link == NULL) 
    {
        printf("DEBUG: Link not found. Available links:\n");
        for (int i = 0; i < folders.count; i++) 
        {
            printf("  - %s\n", folders.links[i].id);
        }
        fprintf(stderr, "Error: Link not found: %s\n", parsed.link_id);
        free(folders.links);
        return -1;
    }

    
    if (strcmp(parsed.location, "Windows") == 0) 
    {
        target_path = link->windows_path;
    } 
    else 
    {
        target_path = link->ubuntu_path;
    }
    
    printf("DEBUG: Target path: %s\n", target_path);
    
    strncpy(parent_dir, target_path, sizeof(parent_dir) - 1);
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash != NULL) 
    {
        *last_slash = '\0';
    }
    
    char rm_command[PATH_MAX + 16];
    snprintf(rm_command, sizeof(rm_command), "rm -rf '%s'", target_path);
    printf("DEBUG: Removing existing folder\n");
    
    if (system(rm_command) != 0) 
    {
        fprintf(stderr, "Error: Cannot remove existing folder\n");
        free(folders.links);
        return -1;
    }
    
    snprintf(command, sizeof(command), "cd '%s' && tar -xzf '%s' 2>&1", 
             parent_dir, backup_path);
    
    printf("DEBUG: Extracting backup\n");
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Failed to restore backup\n");
        free(folders.links);
        return -1;
    }
    
    free(folders.links);
    return 0;
}

int restore_on_sync_failure(folder_link_t *link, sync_operation_t operation)
{
    const char *location = NULL;
    char backup_id[256];
    
    printf("DEBUG: restore_on_sync_failure called\n");
    
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
    
    printf("DEBUG: Finding latest backup for location: %s\n", location);
    
    if (get_latest_backup(link->id, location, backup_id) != 0) 
    {
        fprintf(stderr, "Error: No backup found for restoration\n");
        return -1;
    }
    
    printf("DEBUG: Found latest backup: %s\n", backup_id);
    
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

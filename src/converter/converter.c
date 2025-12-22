#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <../../libs/cjson/cJSON.h>
#include "../../include/converter.h"
#include "../../include/logger.h"
#include "../../include/cli.h"

static int get_conversion_mapping_file(const char *sync_direction, 
                                       char *filepath, size_t max_len) 
{
    char cwd[PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
    {
        fprintf(stderr, "Error: Cannot get current directory\n");
        return -1;
    }
    
    //printf("DEBUG: Current working directory: %s\n", cwd);
    
    if (strcmp(sync_direction, "windows_to_ubuntu") == 0) 
    {
        snprintf(filepath, max_len, "%s/config/extension_mapping/windows_to_ubuntu_conversions.json",
                 cwd);
    } 
    else if (strcmp(sync_direction, "ubuntu_to_windows") == 0) 
    {
        snprintf(filepath, max_len, "%s/config/extension_mapping/ubuntu_to_windows_conversions.json",
                 cwd);
    } 
    else 
    {
        fprintf(stderr, "Error: Invalid sync direction: %s\n", sync_direction);
        return -1;
    }
    
    //printf("DEBUG: Looking for config file at: %s\n", filepath);
    
    return 0;
}


int load_conversion_mappings(const char *sync_direction, 
                             conversion_mappings_t *mappings) 
{
    char filepath[PATH_MAX];
    struct stat stat_buf;
    
    if (get_conversion_mapping_file(sync_direction, filepath, sizeof(filepath)) != 0) 
    {
        return -1;
    }
    
    //printf("DEBUG: Loading conversion mappings from: %s\n", filepath);
    
    if (stat(filepath, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Conversion mapping file not found: %s\n", filepath);
        return -1;
    }
    
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) 
    {
        fprintf(stderr, "Error: Cannot open conversion mapping file\n");
        return -1;
    }
    
    char *json_content = malloc(stat_buf.st_size + 1);
    if (json_content == NULL) 
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(fp);
        return -1;
    }
    
    size_t read_size = fread(json_content, 1, stat_buf.st_size, fp);
    fclose(fp);
    
    if (read_size != stat_buf.st_size) 
    {
        fprintf(stderr, "Error: Failed to read conversion mapping file\n");
        free(json_content);
        return -1;
    }
    
    json_content[stat_buf.st_size] = '\0';
    
    cJSON *json = cJSON_Parse(json_content);
    free(json_content);
    
    if (json == NULL) 
    {
        fprintf(stderr, "Error: Invalid JSON in conversion mapping file\n");
        return -1;
    }
    
    cJSON *conversions = cJSON_GetObjectItem(json, "conversions");
    if (conversions == NULL || !cJSON_IsArray(conversions)) 
    {
        fprintf(stderr, "Error: 'conversions' array not found in mapping file\n");
        cJSON_Delete(json);
        return -1;
    }
    
    int count = cJSON_GetArraySize(conversions);
    mappings->mappings = malloc(sizeof(conversion_mapping_t) * count);
    if (mappings->mappings == NULL) 
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        cJSON_Delete(json);
        return -1;
    }
    
    mappings->count = 0;
    
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, conversions) 
    {
        cJSON *source_ext = cJSON_GetObjectItem(item, "source_ext");
        cJSON *target_ext = cJSON_GetObjectItem(item, "target_ext");
        cJSON *application = cJSON_GetObjectItem(item, "application");
        cJSON *command = cJSON_GetObjectItem(item, "command");
        
        if (source_ext && target_ext && application && command &&
            cJSON_IsString(source_ext) && cJSON_IsString(target_ext) &&
            cJSON_IsString(application) && cJSON_IsString(command)) 
        {
            
            strncpy(mappings->mappings[mappings->count].source_ext, 
                   source_ext->valuestring, sizeof(mappings->mappings[0].source_ext) - 1);
            strncpy(mappings->mappings[mappings->count].target_ext, 
                   target_ext->valuestring, sizeof(mappings->mappings[0].target_ext) - 1);
            strncpy(mappings->mappings[mappings->count].application, 
                   application->valuestring, sizeof(mappings->mappings[0].application) - 1);
            strncpy(mappings->mappings[mappings->count].command, 
                   command->valuestring, sizeof(mappings->mappings[0].command) - 1);
            
            mappings->count++;
        }
    }
    
    cJSON_Delete(json);
    
    //printf("DEBUG: Loaded %d conversion mappings\n", mappings->count);
    
    return 0;
}

void free_conversion_mappings(conversion_mappings_t *mappings) 
{
    if (mappings != NULL && mappings->mappings != NULL) 
    {
        free(mappings->mappings);
        mappings->mappings = NULL;
        mappings->count = 0;
    }
}

static int scan_directory_recursive(const char *dir_path, 
                                    const conversion_mappings_t *mappings,
                                    convertible_file_t *temp_files,
                                    int *temp_count, 
                                    int max_files,
                                    const char *base_path);

static void get_file_extension(const char *filename, char *ext, size_t max_len) 
{
    const char *dot = strrchr(filename, '.');
    if (dot != NULL && dot != filename) 
    {
        strncpy(ext, dot + 1, max_len - 1);
        ext[max_len - 1] = '\0';
        
        for (int i = 0; ext[i]; i++) {
            ext[i] = tolower(ext[i]);
        }
    } 
    else 
    {
        ext[0] = '\0';
    }
}

static conversion_mapping_t* find_mapping(const conversion_mappings_t *mappings,
                                          const char *source_ext) 
{
    for (int i = 0; i < mappings->count; i++) 
    {
        if (strcasecmp(mappings->mappings[i].source_ext, source_ext) == 0) 
        {
            return &mappings->mappings[i];
        }
    }
    return NULL;
}

static int scan_directory_recursive(const char *dir_path, 
                                    const conversion_mappings_t *mappings,
                                    convertible_file_t *temp_files,
                                    int *temp_count, 
                                    int max_files,
                                    const char *base_path) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat stat_buf;
    char full_path[PATH_MAX];
    char ext[32];
    
    dir = opendir(dir_path);
    if (dir == NULL) 
    {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && *temp_count < max_files) 
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) 
        {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (stat(full_path, &stat_buf) != 0) 
        {
            continue;
        }
        
        if (S_ISDIR(stat_buf.st_mode)) 
        {
            //printf("DEBUG: Scanning subdirectory: %s\n", full_path);
            scan_directory_recursive(full_path, mappings, temp_files, temp_count, max_files, base_path);
        } 
        else if (S_ISREG(stat_buf.st_mode)) 
        {
            get_file_extension(entry->d_name, ext, sizeof(ext));
            
            if (ext[0] != '\0')
            {
                conversion_mapping_t *mapping = find_mapping(mappings, ext);
                if (mapping != NULL) 
                {
                    //printf("DEBUG: Found convertible file: %s\n", full_path);
                    
                    strncpy(temp_files[*temp_count].filename, full_path, PATH_MAX - 1);
                    strncpy(temp_files[*temp_count].source_ext, ext, sizeof(temp_files[0].source_ext) - 1);
                    strncpy(temp_files[*temp_count].target_ext, mapping->target_ext, 
                           sizeof(temp_files[0].target_ext) - 1);
                    
                    strncpy(temp_files[*temp_count].target_folder, mapping->target_ext, 
                           sizeof(temp_files[0].target_folder) - 1);
                    for (int i = 0; temp_files[*temp_count].target_folder[i]; i++) 
                    {
                        temp_files[*temp_count].target_folder[i] = 
                            toupper(temp_files[*temp_count].target_folder[i]);
                    }
                    
                    (*temp_count)++;
                }
            }
        }
    }
    
    closedir(dir);
    return 0;
}

int find_convertible_files(const char *target_path, 
                           const conversion_mappings_t *mappings,
                           convertible_files_list_t *files_list) 
{
    //printf("DEBUG: Scanning for convertible files in: %s\n", target_path);
    
    files_list->files = NULL;
    files_list->count = 0;
    
    convertible_file_t temp_files[1000];
    memset(temp_files, 0, sizeof(temp_files));
    int temp_count = 0;
    
    scan_directory_recursive(target_path, mappings, temp_files, &temp_count, 1000, target_path);
    
    if (temp_count == 0) 
    {
        //printf("DEBUG: No convertible files found\n");
        return 0;
    }
    
    files_list->files = malloc(sizeof(convertible_file_t) * temp_count);
    if (files_list->files == NULL) 
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return -1;
    }
    
    memcpy(files_list->files, temp_files, sizeof(convertible_file_t) * temp_count);
    files_list->count = temp_count;
    
    //printf("DEBUG: Found %d convertible files\n", files_list->count);
    
    return 0;
}


void free_convertible_files_list(convertible_files_list_t *files_list) 
{
    if (files_list != NULL && files_list->files != NULL) 
    {
        free(files_list->files);
        files_list->files = NULL;
        files_list->count = 0;
    }
}

int check_app_installed(const char *app_name) 
{
    char command[128];
    snprintf(command, sizeof(command), "which %s > /dev/null 2>&1", app_name);
    
    int result = system(command);
    return (result == 0) ? 1 : 0;
}

static int convert_single_file(const char *target_path,
                               const convertible_file_t *file,
                               const conversion_mapping_t *mapping,
                               int file_index, int total_files) 
{
    char output_dir[PATH_MAX];
    char target_folder_path[PATH_MAX];
    char output_file[PATH_MAX];
    char filename_only[PATH_MAX];
    char command[PATH_MAX * 2 + 256];
    struct stat stat_buf;
    
    const char *basename = strrchr(file->filename, '/');
    if (basename != NULL) 
    {
        basename++;
        strncpy(filename_only, basename, sizeof(filename_only) - 1);
    } 
    else 
    {
        strncpy(filename_only, file->filename, sizeof(filename_only) - 1);
    }
    
    char filename_no_ext[PATH_MAX];
    strncpy(filename_no_ext, filename_only, sizeof(filename_no_ext) - 1);
    char *dot = strrchr(filename_no_ext, '.');
    if (dot != NULL) 
    {
        *dot = '\0';
    }
    
    snprintf(target_folder_path, sizeof(target_folder_path), "%s/%s", 
            target_path, file->target_folder);
    
    if (stat(target_folder_path, &stat_buf) != 0) 
    {
        char mkdir_cmd[PATH_MAX + 32];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p '%s'", target_folder_path);
        if (system(mkdir_cmd) != 0) 
        {
            fprintf(stderr, "Error: Cannot create output directory: %s\n", target_folder_path);
            return -1;
        }
    }
    
    if (!check_app_installed(mapping->application)) 
    {
        display_conversion_app_not_found(mapping->application, filename_only);
        return -2;
    }
    
    snprintf(command, sizeof(command), "%s --outdir '%s' '%s' 2>&1",
            mapping->command, target_folder_path, file->filename);
    
    display_conversion_progress(file_index, total_files, filename_only, 
                               file->target_ext);
    
    if (system(command) != 0) 
    {
        fprintf(stderr, "Error: Failed to convert file\n");
        return -1;
    }
    
    snprintf(output_file, sizeof(output_file), "%s/%s.%s",
            target_folder_path, filename_no_ext, file->target_ext);
    
    if (stat(output_file, &stat_buf) != 0) 
    {
        fprintf(stderr, "Error: Converted file not found: %s\n", output_file);
        return -1;
    }
    
    printf("✓ Successfully converted: %s → %s\n", filename_only, 
           file->target_ext);
    
    return 0;
}

int convert_files(const char *target_path, 
                  const convertible_files_list_t *files_list,
                  const conversion_mappings_t *mappings,
                  const char *link_id) 
{
    int success_count = 0;
    int fail_count = 0;
    int skip_count = 0;
    
    //printf("DEBUG: Starting conversion of %d files\n", files_list->count);
    
    display_conversion_start();
    
    for (int i = 0; i < files_list->count; i++) 
    {
        conversion_mapping_t *mapping = find_mapping(mappings, 
                                                     files_list->files[i].source_ext);
        if (mapping == NULL) 
        {
            skip_count++;
            continue;
        }
        
        int result = convert_single_file(target_path, &files_list->files[i], 
                                        mapping, i + 1, files_list->count);
        
        if (result == 0) 
        {
            success_count++;
            
            char details[256];
            snprintf(details, sizeof(details), "Converted: %s → %s", 
                    files_list->files[i].source_ext, 
                    files_list->files[i].target_ext);
            log_operation(LOG_OP_CONVERT, link_id, LOG_STATUS_SUCCESS, details);
        } 
        else if (result == -2) 
        {
            skip_count++;
            
            char details[256];
            snprintf(details, sizeof(details), "%s not installed, skipped %s", 
                    mapping->application, files_list->files[i].source_ext);
            log_operation(LOG_OP_CONVERT, link_id, LOG_STATUS_WARNING, details);
        } 
        else 
        {
            fail_count++;
            
            char details[256];
            snprintf(details, sizeof(details), "Failed to convert: %s", 
                    files_list->files[i].source_ext);
            log_operation(LOG_OP_CONVERT, link_id, LOG_STATUS_FAILURE, details);
        }
    }
    
    display_conversion_completed(success_count, fail_count, skip_count);
    
    return 0;
}

int needs_conversion(const char *target_path, 
                     const conversion_mappings_t *mappings) 
{
    convertible_files_list_t files_list = {0};
    
    if (find_convertible_files(target_path, mappings, &files_list) != 0)
    {
        return 0;
    }
    
    int result = (files_list.count > 0) ? 1 : 0;
    free_convertible_files_list(&files_list);
    
    return result;
}
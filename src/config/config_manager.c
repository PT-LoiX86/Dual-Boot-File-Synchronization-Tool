#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include <unistd.h>
#include "../../include/filesystem.h"
#include "../../include/utils.h"

#define CONFIG_FILE "~/.dualsync/sync_config.json"
#define MAX_LINKS 50

int load_config(linked_folders_t *folders) 
{
    char config_path[PATH_MAX];
    FILE *fp;
    
    expand_home_path(CONFIG_FILE, config_path, sizeof(config_path));
    
    folders->links = malloc(sizeof(folder_link_t) * MAX_LINKS);
    folders->count = 0;
    
    if (access(config_path, F_OK) != 0) 
    {
        return 0;
    }
    
    fp = fopen(config_path, "r");
    if (fp == NULL) 
    {
        fprintf(stderr, "Error: Cannot read config file\n");
        return -1;
    }
    
    char *file_content = malloc(65536);
    size_t read_size = fread(file_content, 1, 65535, fp);
    file_content[read_size] = '\0';
    fclose(fp);
    
    cJSON *root = cJSON_Parse(file_content);
    free(file_content);
    
    if (root == NULL) 
    {
        fprintf(stderr, "Error: Invalid JSON in config file\n");
        return -1;
    }
    
    cJSON *links_array = cJSON_GetObjectItem(root, "linked_folders");
    if (links_array == NULL || !cJSON_IsArray(links_array)) 
    {
        cJSON_Delete(root);
        return 0;
    }
    
    int count = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, links_array) 
    {
        if (count >= MAX_LINKS) break;
        
        folder_link_t *link = &folders->links[count];
        
        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *ubuntu_path = cJSON_GetObjectItem(item, "ubuntu_path");
        cJSON *windows_path = cJSON_GetObjectItem(item, "windows_path");
        cJSON *ubuntu_uuid = cJSON_GetObjectItem(item, "ubuntu_uuid");
        cJSON *windows_uuid = cJSON_GetObjectItem(item, "windows_uuid");
        cJSON *windows_device = cJSON_GetObjectItem(item, "windows_device");
        
        if (id && ubuntu_path && windows_path && ubuntu_uuid && windows_uuid && windows_device) 
        {
            strncpy(link->id, id->valuestring, sizeof(link->id) - 1);
            strncpy(link->ubuntu_path, ubuntu_path->valuestring, sizeof(link->ubuntu_path) - 1);
            strncpy(link->windows_path, windows_path->valuestring, sizeof(link->windows_path) - 1);
            strncpy(link->ubuntu_uuid, ubuntu_uuid->valuestring, sizeof(link->ubuntu_uuid) - 1);
            strncpy(link->windows_uuid, windows_uuid->valuestring, sizeof(link->windows_uuid) - 1);
            strncpy(link->windows_device, windows_device->valuestring, sizeof(link->windows_device) - 1);
            
            link->status = 0;
            
            count++;
        }
    }
    
    folders->count = count;
    cJSON_Delete(root);
    
    return 0;
}

int save_config(linked_folders_t *folders) 
{
    char config_path[PATH_MAX];
    char config_dir[PATH_MAX];
    char mkdir_cmd[512];
    FILE *fp;
    
    expand_home_path(CONFIG_FILE, config_path, sizeof(config_path));
    strncpy(config_dir, config_path, strlen(config_path) - strlen("/sync_config.json"));
    
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", config_dir);
    int mkdir_result = system(mkdir_cmd);
    (void)mkdir_result;
    
    cJSON *root = cJSON_CreateObject();
    cJSON *links_array = cJSON_CreateArray();
    
    for (int i = 0; i < folders->count; i++) 
    {
        folder_link_t *link = &folders->links[i];
        
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "id", link->id);
        cJSON_AddStringToObject(item, "ubuntu_path", link->ubuntu_path);
        cJSON_AddStringToObject(item, "ubuntu_uuid", link->ubuntu_uuid);
        cJSON_AddStringToObject(item, "windows_path", link->windows_path);
        cJSON_AddStringToObject(item, "windows_uuid", link->windows_uuid);
        cJSON_AddStringToObject(item, "windows_device", link->windows_device);
        cJSON_AddNumberToObject(item, "status", link->status);
        
        cJSON_AddItemToArray(links_array, item);
    }
    
    cJSON_AddItemToObject(root, "linked_folders", links_array);
    
    char *json_str = cJSON_Print(root);
    fp = fopen(config_path, "w");
    
    if (fp == NULL) 
    {
        fprintf(stderr, "Error: Cannot write config\n");
        cJSON_Delete(root);
        free(json_str);
        return -1;
    }
    
    fprintf(fp, "%s\n", json_str);
    fclose(fp);
    
    cJSON_Delete(root);
    free(json_str);
    
    return 0;
}

int find_existing_link(linked_folders_t *folders, const char *path) 
{
    for (int i = 0; i < folders->count; i++) 
    {
        if (strcmp(folders->links[i].ubuntu_path, path) == 0 ||
            strcmp(folders->links[i].windows_path, path) == 0) 
        {
            return i;
        }
    }
    return -1;
}

int remove_link_from_config(linked_folders_t *folders, int index) 
{
    if (index < 0 || index >= folders->count) 
    {
        return -1;
    }
    
    for (int i = index; i < folders->count - 1; i++) 
    {
        folders->links[i] = folders->links[i + 1];
    }
    
    folders->count--;
    return 0;
}

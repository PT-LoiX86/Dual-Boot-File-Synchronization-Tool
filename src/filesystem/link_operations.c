#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../include/filesystem.h"
#include "../../include/utils.h"


int validate_folders_for_linking(const char *ubuntu_path, const char *windows_path) 
{
    printf("DEBUG: Validating folders for linking\n");
    
    if (!folder_exists(ubuntu_path)) 
    {
        fprintf(stderr, "Error: Ubuntu folder does not exist: %s\n", ubuntu_path);
        return -1;
    }
    
    if (!folder_exists(windows_path)) {
        fprintf(stderr, "Error: Windows folder does not exist: %s\n", windows_path);
        return -1;
    }
    
    printf("DEBUG: Both folders exist\n");
    
    int different_disks = are_on_different_disks(ubuntu_path, windows_path);
    
    if (different_disks < 0) 
    {
        fprintf(stderr, "Error: Cannot determine disk information\n");
        return -1;
    }
    
    if (different_disks == 0) 
    {
        fprintf(stderr, "Error: Both folders are on the same disk/partition\n");
        fprintf(stderr, "You can only link folders from different disks\n");
        return -1;
    }
    
    printf("DEBUG: Folders are on different disks\n");
    
    return 0;
}

int create_folder_link(const char *ubuntu_path, const char *windows_path,
                       folder_link_t *new_link) 
{
    char ubuntu_uuid[128] = {0};
    char windows_uuid[128] = {0};
    char windows_device[64] = {0};
    
    printf("DEBUG: Creating folder link\n");
    printf("DEBUG: Ubuntu path: %s\n", ubuntu_path);
    printf("DEBUG: Windows path: %s\n", windows_path);
    
    printf("DEBUG: About to validate folders\n");
    if (validate_folders_for_linking(ubuntu_path, windows_path) != 0) 
    {
        printf("DEBUG: Validation failed\n");
        return -1;
    }
    printf("DEBUG: Validation passed\n");
    
    printf("DEBUG: About to get Ubuntu UUID\n");
    if (get_partition_uuid(ubuntu_path, ubuntu_uuid, sizeof(ubuntu_uuid)) != 0) 
    {
        fprintf(stderr, "Error: Cannot get UUID for Ubuntu folder\n");
        return -1;
    }
    printf("DEBUG: Ubuntu UUID: %s\n", ubuntu_uuid);
    
    printf("DEBUG: About to get Windows UUID\n");
    if (get_partition_uuid(windows_path, windows_uuid, sizeof(windows_uuid)) != 0) 
    {
        fprintf(stderr, "Error: Cannot get UUID for Windows folder\n");
        return -1;
    }
    printf("DEBUG: Windows UUID: %s\n", windows_uuid);
    
    printf("DEBUG: About to get Windows device\n");
    if (get_device_for_path(windows_path, windows_device, sizeof(windows_device)) != 0) 
    {
        fprintf(stderr, "Error: Cannot get device for Windows folder\n");
        return -1;
    }
    printf("DEBUG: Windows device: %s\n", windows_device);
    
    printf("DEBUG: About to generate link ID\n");
    generate_link_id(new_link->id, sizeof(new_link->id));
    printf("DEBUG: Generated ID: %s\n", new_link->id);
    
    printf("DEBUG: About to copy paths\n");
    strncpy(new_link->ubuntu_path, ubuntu_path, sizeof(new_link->ubuntu_path) - 1);
    strncpy(new_link->windows_path, windows_path, sizeof(new_link->windows_path) - 1);
    printf("DEBUG: Paths copied\n");
    
    printf("DEBUG: About to copy UUIDs\n");
    strncpy(new_link->ubuntu_uuid, ubuntu_uuid, sizeof(new_link->ubuntu_uuid) - 1);
    strncpy(new_link->windows_uuid, windows_uuid, sizeof(new_link->windows_uuid) - 1);
    strncpy(new_link->windows_device, windows_device, sizeof(new_link->windows_device) - 1);
    printf("DEBUG: UUIDs copied\n");
    
    new_link->status = 0;
    
    printf("DEBUG: Folder link created with ID: %s\n", new_link->id);
    return 0;
}


int verify_folder_link_accessible(folder_link_t *link) 
{
    char actual_windows_uuid[128] = {0};
    
    printf("DEBUG: Verifying folder link accessibility\n");
    
    if (!folder_exists(link->ubuntu_path)) 
    {
        fprintf(stderr, "Error: Ubuntu folder not accessible: %s\n", link->ubuntu_path);
        return -1;
    }
    
    if (!folder_exists(link->windows_path)) 
    {
        fprintf(stderr, "Error: Windows folder not accessible: %s\n", link->windows_path);
        return -1;
    }
    
    if (get_partition_uuid(link->windows_path, actual_windows_uuid, sizeof(actual_windows_uuid)) != 0) 
    {
        fprintf(stderr, "Error: Cannot get UUID of Windows partition\n");
        return -1;
    }
    
    if (strcmp(actual_windows_uuid, link->windows_uuid) != 0) 
    {
        return -2;
    }
    
    printf("DEBUG: Folder link verified successfully\n");
    return 0;
}

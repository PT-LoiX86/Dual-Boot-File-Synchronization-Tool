#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"
#include "../include/filesystem.h"
#include "../../include/config.h"

// DISK / PARTITION CHECKING

int handle_disk_command() 
{
    return check_disk_status();
}

// FOLDER LINKING

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


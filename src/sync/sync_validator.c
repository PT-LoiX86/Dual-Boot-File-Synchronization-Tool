#include <dirent.h>
#include "../../include/filesystem.h"

int is_folder_empty(const char *folder_path) 
{
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;
    
    printf("DEBUG: Checking if folder is empty: %s\n", folder_path);
    
    dir = opendir(folder_path);
    if (dir == NULL) 
    {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        {
            file_count++;
            break;
        }
    }
    
    closedir(dir);
    
    printf("DEBUG: Folder empty: %s\n", file_count == 0 ? "yes" : "no");
    return file_count == 0 ? 1 : 0;
}

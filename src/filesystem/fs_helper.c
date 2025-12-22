#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <dirent.h>
#include <unistd.h>
#include "../../include/filesystem.h"

int folder_exists(const char *path) 
{
    struct stat stat_buf;
    
    if (stat(path, &stat_buf) != 0) 
    {
        return 0;
    }
    
    if (!S_ISDIR(stat_buf.st_mode)) 
    {
        return 0;
    }
    
    return 1;
}

int check_folder_empty(const char *path) 
{
    DIR *dir;
    struct dirent *entry;
    int is_empty = 1;
    
    dir = opendir(path);
    if (dir == NULL) 
    {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
        {
            is_empty = 0;
            break;
        }
    }
    
    closedir(dir);
    return is_empty ? 0 : 1;
}

int get_device_for_path(const char *path, char *device, size_t device_size) 
{
    struct stat stat_buf;
    unsigned int major_num, minor_num;
    FILE *fp;
    char line[256];
    
    if (stat(path, &stat_buf) != 0) 
    {
        return -1;
    }
    
    major_num = major(stat_buf.st_dev);
    minor_num = minor(stat_buf.st_dev);
    
    fp = fopen("/proc/partitions", "r");
    if (fp == NULL) 
    {
        return -1;
    }
    
    while (fgets(line, sizeof(line), fp) != NULL) 
    {
        unsigned int maj, min;
        char name[64];
        
        if (sscanf(line, "%u %u %*u %63s", &maj, &min, name) == 3) 
        {
            if (maj == major_num && min == minor_num) 
            {
                snprintf(device, device_size, "/dev/%s", name);
                fclose(fp);
                return 0;
            }
        }
    }
    
    fclose(fp);
    return -1;
}

int get_partition_uuid(const char *path, char *uuid, size_t uuid_size) 
{
    char device[64] = {0};
    char command[512] = {0};
    char temp_file[64] = {0};
    FILE *fp = NULL;
    char buffer[256] = {0};
    int result = -1;
    
    //printf("DEBUG: get_partition_uuid() called with path: %s\n", path);
    
    if (path == NULL || uuid == NULL || uuid_size == 0) 
    {
        fprintf(stderr, "Error: Invalid parameters\n");
        return -1;
    }
    
    if (get_device_for_path(path, device, sizeof(device)) != 0) 
    {
        fprintf(stderr, "Error: Cannot determine device\n");
        return -1;
    }
    //printf("DEBUG: Got device: %s\n", device);
    
    strcpy(temp_file, "/tmp/dualsync_uuid_XXXXXX");
    int fd = mkstemp(temp_file);
    if (fd == -1) 
    {
        fprintf(stderr, "Error: Cannot create temp file\n");
        return -1;
    }
    close(fd);
    
    //printf("DEBUG: Using temp file: %s\n", temp_file);
    
    snprintf(command, sizeof(command), "blkid -s UUID -o value %s > %s 2>&1", device, temp_file);
    //printf("DEBUG: Command: %s\n", command);
    
    system(command);
    
    fp = fopen(temp_file, "r");
    if (fp == NULL) 
    {
        fprintf(stderr, "Error: Cannot read temp file\n");
        unlink(temp_file);
        return -1;
    }
    
    if (fgets(buffer, sizeof(buffer), fp) != NULL) 
    {
        //printf("DEBUG: Read from temp file: %s\n", buffer);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) > 0) 
        {
            strncpy(uuid, buffer, uuid_size - 1);
            uuid[uuid_size - 1] = '\0';
            //printf("DEBUG: UUID: %s\n", uuid);
            fclose(fp);
            unlink(temp_file);
            return 0;
        }
    }
    fclose(fp);
    
    //printf("DEBUG: UUID not found, trying NTFS serial number\n");
    
    // Try NTFS serial number (for Windows partitions)
    snprintf(command, sizeof(command), "blkid -s UUID_SUB -o value %s > %s 2>&1", device, temp_file);
    //printf("DEBUG: Trying UUID_SUB: %s\n", command);
    system(command);
    
    fp = fopen(temp_file, "r");
    if (fp == NULL) 
    {
        unlink(temp_file);
        return -1;
    }
    
    if (fgets(buffer, sizeof(buffer), fp) != NULL) 
    {
        //printf("DEBUG: Read UUID_SUB: %s\n", buffer);
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) > 0) 
        {
            strncpy(uuid, buffer, uuid_size - 1);
            uuid[uuid_size - 1] = '\0';
            //printf("DEBUG: NTFS Serial: %s\n", uuid);
            fclose(fp);
            unlink(temp_file);
            return 0;
        }
    }
    fclose(fp);
    
    //printf("DEBUG: UUID_SUB not found, trying alternative method\n");
    
    // Fallback: Use device path as identifier (works without root)
    snprintf(command, sizeof(command), "blkid -s TYPE -o value %s > %s 2>&1", device, temp_file);
    //printf("DEBUG: Getting filesystem type: %s\n", command);
    system(command);
    
    fp = fopen(temp_file, "r");
    if (fp == NULL) 
    {
        unlink(temp_file);
        return -1;
    }
    
    char fstype_str[64] = {0};
    if (fgets(fstype_str, sizeof(fstype_str), fp) != NULL) 
    {
        fstype_str[strcspn(fstype_str, "\n")] = 0;
        
        // Create composite ID: device + filesystem type
        if (strlen(fstype_str) > 0) 
        {
            snprintf(uuid, uuid_size, "%s_%s", device, fstype_str);
        } 
        else 
        {
            snprintf(uuid, uuid_size, "%s_unknown", device);
        }
        
        //printf("DEBUG: Created composite ID: %s\n", uuid);
        fclose(fp);
        unlink(temp_file);
        return 0;
    }
    
    fclose(fp);
    unlink(temp_file);
    //printf("DEBUG: All UUID methods failed, using device as fallback\n");
    
    snprintf(uuid, uuid_size, "%s", device);
    return 0;
}

int are_on_different_disks(const char *path1, const char *path2) 
{
    char device1[64] = {0};
    char device2[64] = {0};
    
    if (get_device_for_path(path1, device1, sizeof(device1)) != 0) 
    {
        return -1;
    }
    
    if (get_device_for_path(path2, device2, sizeof(device2)) != 0) 
    {
        return -1;
    }
    
    return strcmp(device1, device2) != 0 ? 1 : 0;
}

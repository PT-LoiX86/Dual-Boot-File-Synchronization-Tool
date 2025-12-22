#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../include/filesystem.h"


int calculate_file_md5(const char *filepath, char *md5_hash) 
{
    char command[PATH_MAX + 64];
    FILE *fp;
    char temp_file[64] = {0};
    
    //printf("DEBUG: Calculating MD5 for: %s\n", filepath);
    
    if (filepath == NULL || md5_hash == NULL) 
    {
        return -1;
    }
    
    strcpy(temp_file, "/tmp/md5_XXXXXX");
    int fd = mkstemp(temp_file);
    if (fd == -1) 
    {
        return -1;
    }
    close(fd);
    
    snprintf(command, sizeof(command), "md5sum '%s' > %s 2>&1", filepath, temp_file);
    int result = system(command);
    
    if (result != 0) 
    {
        //printf("DEBUG: md5sum command failed\n");
        unlink(temp_file);
        return -1;
    }
    
    fp = fopen(temp_file, "r");
    if (fp == NULL) 
    {
        unlink(temp_file);
        return -1;
    }
    
    char buffer[128] = {0};
    if (fgets(buffer, sizeof(buffer), fp) != NULL) 
    {
        sscanf(buffer, "%32s", md5_hash);
        //printf("DEBUG: MD5 hash: %s\n", md5_hash);
        fclose(fp);
        unlink(temp_file);
        return 0;
    }
    
    fclose(fp);
    unlink(temp_file);
    return -1;
}

int compare_files(const char *file1, const char *file2, int *are_equal) 
{
    struct stat stat1, stat2;
    char hash1[33] = {0};
    char hash2[33] = {0};
    
    //printf("DEBUG: Comparing files: %s vs %s\n", file1, file2);
    
    if (stat(file1, &stat1) != 0 || stat(file2, &stat2) != 0) 
    {
        return -1;
    }
    
    if (stat1.st_size != stat2.st_size) 
    {
        *are_equal = 0;
        //printf("DEBUG: Files differ in size\n");
        return 0;
    }
    
    if (stat1.st_mtime != stat2.st_mtime) 
    {
        if (calculate_file_md5(file1, hash1) != 0 || calculate_file_md5(file2, hash2) != 0) 
        {
            return -1;
        }
        
        *are_equal = (strcmp(hash1, hash2) == 0) ? 1 : 0;
        //printf("DEBUG: Hash comparison result: %s\n", *are_equal ? "equal" : "different");
        return 0;
    }
    
    *are_equal = 1;
    //printf("DEBUG: Files appear identical (same size and timestamp)\n");
    return 0;
}

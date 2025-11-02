#include <stdio.h>
#include <string.h>
#include "cli.h"

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        printf("Usage: fsync <command> [options]\n");
        return 1;
    }

    if (strcmp(argv[1], "disk") == 0) 
    {
        return handle_disk_command();
    }
    
    if (strcmp(argv[1], "--help") == 0) 
    {
        printf("fsync - Dual-boot file synchronization tool\n");
        return 0;
    }

    printf("Unknown command: %s\n", argv[1]);
    return 1;
}

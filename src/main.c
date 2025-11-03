#include <stdio.h>
#include <string.h>
#include "cli.h"

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        printf("Usage: dualsync <command> [options]\n");
        printf("Commands:\n");
        printf("  disk              - Check connected disks\n");
        printf("  link              - Link two folders for synchronization\n");
        printf("  unlink            - Remove a folder link\n");
        printf("  sync              - Synchronize linked folders\n");
        printf("  status            - Check link status\n");
        printf("  --help            - Show help\n");
        return 1;
    }

    if (strcmp(argv[1], "disk") == 0) 
    {
        return handle_disk_command();
    }

    if (strcmp(argv[1], "link") == 0) 
    {
        return handle_link_command(argc - 1, argv + 1);
    }
    
    if (strcmp(argv[1], "unlink") == 0) 
    {
        return handle_unlink_command(argc - 1, argv + 1);
    }

    if (strcmp(argv[1], "sync") == 0) 
    {
        return handle_sync_command(argc - 1, argv + 1);
    }

    
    if (strcmp(argv[1], "--help") == 0) 
    {
        printf("dualsync - Dual-boot file synchronization tool\n");
        return 0;
    }

    printf("Unknown command: %s\n", argv[1]);
    return 1;
}

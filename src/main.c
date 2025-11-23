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

    if (strcmp(argv[1], "links-list") == 0)
    {
        return handle_links_list_command(argc - 2, argv + 2);
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

    if (strcmp(argv[1], "backups-list") == 0) 
    {
        return handle_backups_list_command(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "backup") == 0) 
    {
        return handle_backup_command(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "restore") == 0) 
    {
        return handle_restore_command(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "backups-clean") == 0) 
    {
        return handle_backups_clean_command(argc - 2, argv + 2);
    }

    if (strcmp(argv[1], "log") == 0) 
    {
        if (argc < 3) 
        {
            fprintf(stderr, "Usage: dualsync log <--list | --since <DD/MM/YYYY> | --track>\n");
            return 1;
        }
        
        if (strcmp(argv[2], "--list") == 0) 
        {
            return handle_log_list_command(argc - 3, argv + 3);
        }
        
        if (strcmp(argv[2], "--since") == 0) 
        {
            return handle_log_since_command(argc - 3, argv + 3);
        }
        
        if (strcmp(argv[2], "--track") == 0) 
        {
            return handle_log_track_command(argc - 3, argv + 3);
        }
        
        fprintf(stderr, "Unknown log command: %s\n", argv[2]);
        fprintf(stderr, "Usage: dualsync log <--list | --since <DD/MM/YYYY> | --track>\n");
        return 1;
    }
    
    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}

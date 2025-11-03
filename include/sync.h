#ifndef SYNC_H
#define SYNC_H

#include "filesystem.h"

typedef struct 
{
    int files_added;
    int files_updated;
    int files_deleted;
    unsigned long bytes_copied;
    unsigned long bytes_updated;
    unsigned long bytes_deleted;
    int errors_count;
    time_t sync_start_time;
    time_t sync_end_time;
} 
sync_stats_t;

int handle_sync_command(int argc, char *argv[]);
int perform_sync(folder_link_t *link, sync_operation_t operation, 
                 conflict_resolution_t *resolutions, int resolution_count);
int display_sync_preview(sync_changes_t *changes, const char *source, const char *target);
int resolve_conflicts_interactive(sync_changes_t *changes, conflict_resolution_t **resolutions, int *resolution_count);
int perform_sync(folder_link_t *link, sync_operation_t operation,
                 conflict_resolution_t *resolutions, int resolution_count);

#endif

#ifndef CLI_H
#define CLI_H

#include "filesystem.h"
#include "../include/filesystem.h"

// DISK / PARTITION CHECKING

int handle_disk_command();

// FOLDER LINKING

int handle_link_command(int argc, char *argv[]);
int handle_unlink_command(int argc, char *argv[]);
int display_link_success(folder_link_t *link);
int display_link_already_exists(folder_link_t *existing_link, 
                                const char *provided_path, const char *other_path);
int display_unlink_confirmation(folder_link_t *link);
int display_verify_link_error(int error_code, folder_link_t *link);

// SYNC OPERATIONS

int display_sync_preview(sync_changes_t *changes, const char *source, const char *target);
int resolve_conflicts_interactive(sync_changes_t *changes, 
                                  conflict_resolution_t **resolutions, 
                                  int *resolution_count);
int display_final_confirmation(sync_changes_t *changes, int has_conflicts);
int display_sync_error_prompt(const char *filepath, const char *error_msg);
int handle_sync_command(int argc, char *argv[]);

#endif

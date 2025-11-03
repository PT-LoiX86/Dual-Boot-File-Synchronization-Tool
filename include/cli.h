#ifndef CLI_H
#define CLI_H

#include "filesystem.h"
#include "../include/filesystem.h"

int handle_disk_command();
int handle_link_command(int argc, char *argv[]);
int handle_unlink_command(int argc, char *argv[]);
int display_link_success(folder_link_t *link);
int display_link_already_exists(folder_link_t *existing_link, 
                                const char *provided_path, const char *other_path);
int display_unlink_confirmation(folder_link_t *link);
int display_verify_link_error(int error_code, folder_link_t *link);

#endif

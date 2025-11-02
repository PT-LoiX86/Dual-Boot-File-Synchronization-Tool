#include "../include/filesystem.h"
#ifndef CLI_H
#define CLI_H
#include "filesystem.h"

int handle_disk_command();
int display_disk_status(windows_partitions_t *windows_partitions, disk_info_t *ubuntu);

#endif

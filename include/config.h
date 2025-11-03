#ifndef CONFIG_H
#define CONFIG_H

#include "filesystem.h"

int load_config(linked_folders_t *folders);
int save_config(linked_folders_t *folders);
int find_existing_link(linked_folders_t *folders, const char *path);
int remove_link_from_config(linked_folders_t *folders, int index);

#endif

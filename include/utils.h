#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void generate_link_id(char *id, size_t id_size);
void expand_home_path(const char *path, char *expanded, size_t size);

#endif

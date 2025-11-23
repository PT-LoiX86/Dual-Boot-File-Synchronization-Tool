#ifndef CONVERTER_H
#define CONVERTER_H

#include <limits.h>

typedef struct 
{
    char source_ext[32];
    char target_ext[32];
    char application[64];
    char command[PATH_MAX];
} 
conversion_mapping_t;

typedef struct 
{
    conversion_mapping_t *mappings;
    int count;
} 
conversion_mappings_t;

typedef struct 
{
    char filename[PATH_MAX];
    char source_ext[32];
    char target_ext[32];
    char target_folder[64];
} 
convertible_file_t;

typedef struct 
{
    convertible_file_t *files;
    int count;
} 
convertible_files_list_t;

int load_conversion_mappings(const char *sync_direction, 
                             conversion_mappings_t *mappings);
void free_conversion_mappings(conversion_mappings_t *mappings);
int find_convertible_files(const char *target_path, 
                           const conversion_mappings_t *mappings,
                           convertible_files_list_t *files_list);
void free_convertible_files_list(convertible_files_list_t *files_list);
int check_app_installed(const char *app_name);
int convert_files(const char *target_path, 
                  const convertible_files_list_t *files_list,
                  const conversion_mappings_t *mappings,
                  const char *link_id);
int needs_conversion(const char *target_path, 
                     const conversion_mappings_t *mappings);

#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../include/utils.h"

void generate_link_id(char *id, size_t id_size) 
{
    time_t now = time(NULL);
    snprintf(id, id_size, "link_%ld", now);
}

void expand_home_path(const char *path, char *expanded, size_t size) 
{
    if (path[0] == '~') 
    {
        const char *home = getenv("HOME");
        if (home != NULL) 
        {
            snprintf(expanded, size, "%s%s", home, path + 1);
        } 
        else 
        {
            strncpy(expanded, path, size - 1);
        }
    } 
    else 
    {
        strncpy(expanded, path, size - 1);
    }
}
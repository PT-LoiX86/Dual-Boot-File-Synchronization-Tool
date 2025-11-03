#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/filesystem.h"

sync_changes_t* create_sync_changes(void) 
{
    sync_changes_t *changes = malloc(sizeof(sync_changes_t));
    if (changes == NULL) 
    {
        return NULL;
    }
    
    changes->capacity = 100;
    changes->changes = malloc(sizeof(file_change_t) * changes->capacity);
    
    if (changes->changes == NULL) 
    {
        free(changes);
        return NULL;
    }
    
    changes->count = 0;
    changes->new_count = 0;
    changes->modified_count = 0;
    changes->deleted_count = 0;
    changes->conflict_count = 0;
    changes->new_size = 0;
    changes->modified_size = 0;
    changes->deleted_size = 0;
    
    return changes;
}

void free_sync_changes(sync_changes_t *changes) 
{
    if (changes == NULL) 
    {
        return;
    }
    
    if (changes->changes != NULL) 
    {
        free(changes->changes);
    }
    
    free(changes);
}

void add_change(sync_changes_t *changes, const file_change_t *change) 
{
    if (changes == NULL || change == NULL) 
    {
        return;
    }
    
    if (changes->count >= changes->capacity) 
    {
        changes->capacity *= 2;
        file_change_t *new_changes = realloc(changes->changes, 
                                             sizeof(file_change_t) * changes->capacity);
        if (new_changes == NULL) 
        {
            fprintf(stderr, "Error: Cannot expand changes array\n");
            return;
        }
        changes->changes = new_changes;
    }
    
    changes->changes[changes->count] = *change;
    
    switch (change->status) 
    {
        case FILE_STATUS_NEW:
            changes->new_count++;
            changes->new_size += change->size;
            break;
        case FILE_STATUS_MODIFIED:
            changes->modified_count++;
            changes->modified_size += change->size;
            break;
        case FILE_STATUS_DELETED:
            changes->deleted_count++;
            changes->deleted_size += change->size;
            break;
        case FILE_STATUS_UNCHANGED:
            break;
    }
    
    if (change->conflict != CONFLICT_NONE) 
    {
        changes->conflict_count++;
    }
    
    changes->count++;
}

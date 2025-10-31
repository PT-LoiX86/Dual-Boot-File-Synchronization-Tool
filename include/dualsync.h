#ifndef DUALSYNC_H
#define DUALSYNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

// Version information
#define DUALSYNC_VERSION "MVP-0-0-1"
#define DUALSYNC_VERSION_MAJOR 0
#define DUALSYNC_VERSION_MINOR 0
#define DUALSYNC_VERSION_PATCH 1

// Configuration
#define MAX_PATH_LENGTH 4096
#define MAX_COMMAND_LENGTH 1024
#define MAX_ERROR_MESSAGE 256
#define CONFIG_DIR_LINUX ".DUALSYNC"
#define CONFIG_DIR_WINDOWS "DUALSYNC"

// Return codes
typedef enum 
{
    DUALSYNC_SUCCESS = 0,
    DUALSYNC_ERROR_INVALID_ARGS = 1,
    DUALSYNC_ERROR_FILE_NOT_FOUND = 2,
    DUALSYNC_ERROR_PERMISSION_DENIED = 3,
    DUALSYNC_ERROR_DISK_NOT_AVAILABLE = 4,
    DUALSYNC_ERROR_CONFIG_INVALID = 5,
    DUALSYNC_ERROR_SYNC_FAILED = 6,
    DUALSYNC_ERROR_MEMORY_ALLOCATION = 7,
    DUALSYNC_ERROR_UNKNOWN = 99
} 
DUALSYNC_error_t;

// Forward declarations
typedef struct sync_pair sync_pair_t;
typedef struct sync_config sync_config_t;
typedef struct sync_log sync_log_t;

#endif
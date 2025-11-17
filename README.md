# Dual-Boot File Synchronization Tool

## About

**Concept:** An open-source tool for file synchronization when working on dual-boot (Ubuntu (main) - Windows).

**Version:** MVP-0-0-1 (Minimum Viable Product)

**Language:** C (C11 standard)

**Documentation:**
- [Complete Code Inspection Guide](CODE_INSPECTION.md) - Comprehensive step-by-step guide for understanding the codebase
- [Project Planning Sheet](https://docs.google.com/spreadsheets/d/1CnZLICE2wTZ6S9hp4bg_5lvL-tPmGUVPoM-AeqvXKUE/edit?usp=sharing)

## Project Purpose

This tool solves the common problem faced by developers and users who maintain a dual-boot setup with Linux (Ubuntu) and Windows. When working across both operating systems, keeping files synchronized between the partitions can be challenging. This tool automates the synchronization process with features like:

- **Bidirectional Synchronization**: Sync files from Linux to Windows or vice versa
- **Intelligent Conflict Resolution**: Detect and resolve conflicts when files are modified in both locations
- **File Format Conversion**: Handle platform-specific file formats (line endings, paths, etc.)
- **Backup and Rollback**: Create backups before modifications and rollback if needed
- **Cross-Platform Path Handling**: Automatically resolve paths between Linux and Windows filesystems
- **Comprehensive Logging**: Track all synchronization operations for audit and debugging

## Architecture Overview

### High-Level Components

```
┌─────────────┐
│     CLI     │  ← Command-line interface for user interaction
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   CONFIG    │  ← Configuration management and validation
└──────┬──────┘
       │
       ▼
┌─────────────┐
│SYNC ENGINE  │  ← Core synchronization logic
└──────┬──────┘
       │
   ┌───┴────┬──────────┬────────┐
   ▼        ▼          ▼        ▼
┌────┐  ┌──────┐  ┌────────┐ ┌──────┐
│ FS │  │BACKUP│  │CONVERT │ │LOGGER│  ← Supporting modules
└────┘  └──────┘  └────────┘ └──────┘
```

### Module Descriptions

1. **CLI Module** (`src/cli/`): Handles command-line argument parsing, interactive mode, and formatted output display

2. **Configuration Module** (`src/config/`): Loads and validates JSON configuration files, manages sync settings

3. **Sync Engine** (`src/sync/`): Core synchronization logic including file scanning, change detection, and conflict resolution

4. **Filesystem Module** (`src/filesystem/`): Abstracts file operations, handles cross-platform paths, monitors disk status

5. **Backup Module** (`src/backup/`): Creates backups before modifications, manages backup history, handles rollback

6. **Converter Module** (`src/converter/`): Converts file formats between platforms (line endings, encodings, etc.)

7. **Logger Module** (`src/logger/`): Comprehensive logging with rotation, formatting, and multiple log levels

8. **JSON Module** (`src/json/`): Handles JSON parsing and generation using the cJSON library

9. **Utils Module** (`src/utils/`): Common utilities for error handling, hashing, string operations, and time management

### Data Flow

```
User Command → CLI Parser → Config Loader → Sync Engine
                                                  ↓
                                    ┌─────────────┴─────────────┐
                                    ↓                           ↓
                              File Scanner              Conflict Detector
                                    ↓                           ↓
                              File Comparator         Resolution Strategy
                                    ↓                           ↓
                              Backup Creator          Apply Changes
                                    ↓                           ↓
                              File Operations   →    Update Logs
```

## Code Structure Explained

### Header Files (`include/`)

**dualsync.h** - Main header containing:
- Version information (`DUALSYNC_VERSION`)
- Error code enumeration (`DUALSYNC_error_t`)
- Configuration constants (`MAX_PATH_LENGTH`, `CONFIG_DIR_LINUX`, etc.)
- Forward type declarations

Other headers define interfaces for each module (backup, CLI, config, converter, filesystem, JSON, logger, sync, utils).

### Source Organization

Each module is organized into its own directory under `src/` with focused, single-responsibility files:

**Sync Module** (`src/sync/`):
- `sync_engine.c` - Orchestrates the synchronization process
- `file_scanner.c` - Scans directory trees and collects file metadata
- `conflict_resolver.c` - Detects and resolves file conflicts

**Filesystem Module** (`src/filesystem/`):
- `fs_operations.c` - Basic file operations (copy, move, delete)
- `disk_monitor.c` - Checks disk availability and space
- `path_resolver.c` - Resolves and normalizes cross-platform paths
- `permission_handler.c` - Manages file permissions

**Backup Module** (`src/backup/`):
- `backup_manager.c` - Creates and manages backups
- `cleaner.c` - Cleans up old backup files
- `rollback_handler.c` - Restores files from backups

**Logger Module** (`src/logger/`):
- `logger.c` - Core logging functionality
- `log_formatter.c` - Formats log messages with timestamps and levels
- `log_rotation.c` - Rotates log files to prevent unlimited growth

### Configuration System

The tool uses JSON for configuration with schema validation:

**config/schemas/** - JSON schemas for validation
**config/templates/** - Template configuration files

Example configuration structure:
```json
{
  "version": "1.0",
  "sync_pairs": [
    {
      "linux_path": "/home/user/documents",
      "windows_path": "/mnt/windows/Users/User/Documents",
      "direction": "bidirectional",
      "backup_enabled": true
    }
  ],
  "options": {
    "conflict_resolution": "ask",
    "log_level": "info",
    "exclude_patterns": ["*.tmp", "*.bak"]
  }
}
```

### Key Algorithms

**Change Detection:**
1. Scan both source and destination directory trees
2. Calculate SHA-256 hashes for files (using `libs/sha256/`)
3. Compare metadata (size, timestamp, hash)
4. Classify as: new, modified, deleted, or unchanged

**Conflict Resolution:**
1. Detect when a file is modified in both locations
2. Apply resolution strategy:
   - **Ask**: Prompt user for decision
   - **Newest**: Keep the most recently modified version
   - **Largest**: Keep the larger file
   - **Both**: Rename and keep both versions

**Backup Strategy:**
1. Before any destructive operation, create backup
2. Store in `.DUALSYNC/backups/` with timestamp
3. Maintain metadata for each backup session
4. Support rollback to any previous state

### Error Handling

The codebase uses a consistent error handling pattern with enumerated error codes:

```c
typedef enum {
    DUALSYNC_SUCCESS = 0,
    DUALSYNC_ERROR_INVALID_ARGS = 1,
    DUALSYNC_ERROR_FILE_NOT_FOUND = 2,
    DUALSYNC_ERROR_PERMISSION_DENIED = 3,
    DUALSYNC_ERROR_DISK_NOT_AVAILABLE = 4,
    DUALSYNC_ERROR_CONFIG_INVALID = 5,
    DUALSYNC_ERROR_SYNC_FAILED = 6,
    DUALSYNC_ERROR_MEMORY_ALLOCATION = 7,
    DUALSYNC_ERROR_UNKNOWN = 99
} DUALSYNC_error_t;
```

All functions return these error codes, and errors are logged with context for debugging.

## Installation

1. **Clone the repository with submodules:**

   `git clone --recurse-submodules https://github.com/PT-LoiX86/Dual-Boot-File-Synchronization-Tool.git`

2. **Build the project:** locate to the project folder

   `make`

3. **Run tests:** locate to the project folder

   `make test`

4. **Install (optional):**

   `sudo make install`

**Makefile usage:**

    make              # Build the main executable (default)
    make debug        # Build with debugging symbols and no optimization
    make test         # Compile and run all tests
    make clean        # Remove all build files
    make install      # Install fsync to /usr/local/bin/
    make format       # Auto-format all source code

### Third-Party Dependencies

This project uses the following libraries as git submodules:

- **cJSON** - Lightweight JSON parsing library
  - **Purpose**: Parse configuration files, generate JSON reports
  - **Location**: `libs/cjson/`
  - **Usage**: Configuration management, logging output

- **sha256** - File hashing for integrity verification
  - **Purpose**: Calculate file hashes for change detection
  - **Location**: `libs/sha256/`
  - **Usage**: Detect file modifications, verify integrity

These are automatically downloaded when you clone with `--recurse-submodules` or run `git submodule update --init --recursive`.

## How It Works

### Typical Synchronization Flow

1. **Initialization**
   ```
   User runs: dualsync --config ~/.dualsync/config.json
   ```

2. **Configuration Loading**
   - Load and parse JSON configuration
   - Validate against schema
   - Set up sync pairs (Linux ↔ Windows paths)

3. **Pre-flight Checks**
   - Verify disk availability (is Windows partition mounted?)
   - Check permissions on source and destination
   - Ensure sufficient disk space

4. **File Scanning**
   - Recursively scan source directory
   - Recursively scan destination directory
   - Build file metadata lists (path, size, timestamp, hash)

5. **Change Detection**
   - Compare file lists between source and destination
   - Identify new files (exist in one location only)
   - Identify modified files (different hash or timestamp)
   - Identify deleted files (exist in old state but not current)

6. **Conflict Detection**
   - Find files modified in both locations since last sync
   - Apply configured conflict resolution strategy
   - Prompt user if strategy is "ask"

7. **Backup Creation**
   - Create timestamped backup directory
   - Backup files that will be modified or deleted
   - Store backup metadata (original paths, timestamps)

8. **Apply Changes**
   - Copy new files to destination
   - Update modified files
   - Delete removed files (if configured)
   - Convert file formats if needed (line endings, etc.)

9. **Verification**
   - Verify file integrity with hash comparison
   - Log all operations
   - Generate summary report

10. **Cleanup**
    - Remove old backups (if retention policy configured)
    - Update sync state
    - Display results to user

### Example Usage Scenarios

**Scenario 1: Sync Documents from Linux to Windows**
```bash
# Configuration specifies Documents folder sync
dualsync --sync documents --direction linux-to-windows

# Output:
# Scanning /home/user/Documents...
# Scanning /mnt/windows/Users/User/Documents...
# Found 5 new files, 2 modified files
# Creating backup...
# Copying files...
# Sync completed successfully
```

**Scenario 2: Bidirectional Sync with Conflict**
```bash
dualsync --sync projects --direction bidirectional

# Output:
# Conflict detected: project.txt
#   Linux:   Modified 2024-01-15 10:30
#   Windows: Modified 2024-01-15 14:20
# Resolution options:
#   1. Keep Linux version
#   2. Keep Windows version
#   3. Keep both (rename)
# Your choice [1-3]: 
```

**Scenario 3: Rollback After Bad Sync**
```bash
# Something went wrong, rollback to previous state
dualsync --rollback

# Lists available backups:
# 1. 2024-01-15_10-30-45 (5 files)
# 2. 2024-01-14_16-20-10 (3 files)
# Select backup [1-2]: 1
# Restoring from backup...
# Rollback completed successfully
```

## For contributors

All changes must be committed into develop branch, not main.

### Understanding the Codebase

Before contributing, please read the [CODE_INSPECTION.md](CODE_INSPECTION.md) document which provides:
- Detailed step-by-step guide for inspecting the code
- Module-by-module analysis
- Architecture diagrams
- Data flow documentation
- Development best practices

### Code Style Guidelines

- **Indentation**: Use consistent indentation (follow existing style)
- **Naming**: Use descriptive names for functions and variables
  - Functions: `snake_case` (e.g., `sync_engine_run`)
  - Types: `snake_case_t` (e.g., `sync_config_t`)
  - Constants: `UPPER_CASE` (e.g., `MAX_PATH_LENGTH`)
- **Comments**: Add comments for complex logic, not obvious code
- **Error Handling**: Always check return values and handle errors
- **Memory Management**: Free all allocated memory, no leaks

### Development Workflow

1. **Fork and clone the repository**
2. **Create a feature branch from develop**
   ```bash
   git checkout develop
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**
   - Write code following the style guide
   - Add tests for new functionality
   - Update documentation
4. **Format your code**
   ```bash
   make format
   ```
5. **Test your changes**
   ```bash
   make clean
   make test
   ```
6. **Commit and push**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin feature/your-feature-name
   ```
7. **Create a Pull Request to develop branch**

## Development

### Updating Submodules

    git submodule update --remote --merge

### Building for Debug

    make debug
    ./build/fsync

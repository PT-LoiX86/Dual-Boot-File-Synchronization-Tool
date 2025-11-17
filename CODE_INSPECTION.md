# Code Inspection Guide - Dual-Boot File Synchronization Tool

This document provides a comprehensive step-by-step guide for inspecting and understanding the Dual-Boot File Synchronization Tool codebase.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Repository Structure](#repository-structure)
3. [Inspection Prerequisites](#inspection-prerequisites)
4. [Step-by-Step Inspection Process](#step-by-step-inspection-process)
5. [Module-by-Module Analysis](#module-by-module-analysis)
6. [Architecture Overview](#architecture-overview)
7. [Development Workflow](#development-workflow)
8. [Testing Strategy](#testing-strategy)

---

## Project Overview

**Purpose**: The Dual-Boot File Synchronization Tool is designed to synchronize files between Ubuntu (main OS) and Windows partitions in a dual-boot setup.

**Language**: C (C11 standard)
**Version**: MVP-0-0-1
**Build System**: GNU Make

### Key Features (Planned)
- Bidirectional file synchronization between OS partitions
- Conflict resolution for concurrent file modifications
- File conversion support for cross-platform compatibility
- Backup and rollback capabilities
- Comprehensive logging and monitoring
- Interactive CLI interface

---

## Repository Structure

```
Dual-Boot-File-Synchronization-Tool/
├── README.md                 # Main documentation
├── Makefile                  # Build configuration
├── .gitignore               # Git ignore rules
├── CODE_INSPECTION.md       # This file
│
├── include/                 # Header files (.h)
│   ├── dualsync.h          # Main header with version info and error codes
│   ├── backup.h            # Backup management interfaces
│   ├── cli.h               # Command-line interface
│   ├── config.h            # Configuration management
│   ├── converter.h         # File conversion utilities
│   ├── filesystem.h        # File system operations
│   ├── json_parser.h       # JSON parsing
│   ├── logger.h            # Logging system
│   ├── sync.h              # Synchronization engine
│   └── utils.h             # Utility functions
│
├── src/                     # Source files (.c)
│   ├── main.c              # Entry point
│   │
│   ├── backup/             # Backup and restore functionality
│   │   ├── backup_manager.c      # Manages backup operations
│   │   ├── cleaner.c             # Cleanup old backups
│   │   └── rollback_handler.c    # Restore previous states
│   │
│   ├── cli/                # Command-line interface
│   │   ├── cli_parser.c          # Parse command-line arguments
│   │   ├── cli_display.c         # Display formatted output
│   │   └── interactive.c         # Interactive mode
│   │
│   ├── config/             # Configuration management
│   │   ├── config_manager.c      # Load/save configurations
│   │   └── schema_validator.c    # Validate JSON schemas
│   │
│   ├── converter/          # File format conversion
│   │   ├── converter_engine.c    # Main conversion logic
│   │   ├── command_executor.c    # Execute conversion commands
│   │   └── extension_mapper.c    # Map file extensions
│   │
│   ├── filesystem/         # File system operations
│   │   ├── fs_operations.c       # Basic file operations
│   │   ├── disk_monitor.c        # Monitor disk availability
│   │   ├── path_resolver.c       # Resolve cross-platform paths
│   │   └── permission_handler.c  # Handle file permissions
│   │
│   ├── json/               # JSON processing
│   │   ├── json_handler.c        # JSON manipulation
│   │   └── schema_loader.c       # Load JSON schemas
│   │
│   ├── logger/             # Logging system
│   │   ├── logger.c              # Core logging functionality
│   │   ├── log_formatter.c       # Format log messages
│   │   └── log_rotation.c        # Rotate log files
│   │
│   ├── sync/               # Synchronization engine
│   │   ├── sync_engine.c         # Main sync logic
│   │   ├── file_scanner.c        # Scan directories
│   │   └── conflict_resolver.c   # Resolve file conflicts
│   │
│   └── utils/              # Utility functions
│       ├── error_handling.c      # Error management
│       ├── hash_utils.c          # File hashing (SHA-256)
│       ├── string_utils.c        # String operations
│       └── time_utils.c          # Time/date utilities
│
├── config/                  # Configuration templates
│   ├── schemas/
│   │   ├── config_schema.json    # Configuration validation schema
│   │   └── log_schema.json       # Log format schema
│   └── templates/
│       ├── sync_config.json      # Sync configuration template
│       └── extension_mapping.json # File extension mappings
│
├── tests/                   # Test suites
│   ├── unit/               # Unit tests
│   │   ├── test_config.c
│   │   ├── test_filesystem.c
│   │   ├── test_sync.c
│   │   └── test_utils.c
│   └── integration/        # Integration tests
│       ├── test_full_sync.c
│       └── test_cli.c
│
├── libs/                    # Third-party libraries (submodules)
│   ├── cjson/              # JSON parsing library
│   └── sha256/             # SHA-256 hashing library
│
└── scripts/                # Development scripts
    ├── install.sh          # Installation script
    ├── run_tests.sh        # Test runner
    └── setup_dev.sh        # Development environment setup
```

---

## Inspection Prerequisites

### Required Tools

1. **GCC Compiler** (version 7.0 or higher)
   ```bash
   gcc --version
   ```

2. **GNU Make**
   ```bash
   make --version
   ```

3. **Git** (for version control)
   ```bash
   git --version
   ```

4. **clang-format** (optional, for code formatting)
   ```bash
   clang-format --version
   ```

### Environment Setup

1. **Clone the repository with submodules:**
   ```bash
   git clone --recurse-submodules https://github.com/PT-LoiX86/Dual-Boot-File-Synchronization-Tool.git
   cd Dual-Boot-File-Synchronization-Tool
   ```

2. **Verify submodules are initialized:**
   ```bash
   git submodule status
   ```

3. **Update submodules if needed:**
   ```bash
   git submodule update --init --recursive
   ```

---

## Step-by-Step Inspection Process

### Step 1: Initial Repository Exploration

**Objective**: Get familiar with the project structure and available files.

1. **List the main directories:**
   ```bash
   ls -la
   ```

2. **Check the README:**
   ```bash
   cat README.md
   ```

3. **Examine the Makefile:**
   ```bash
   cat Makefile
   ```
   
   **Key observations:**
   - Compiler: GCC with C11 standard
   - Flags: `-Wall -Wextra` (all warnings enabled)
   - Optimization: `-O2` for release, `-O0` for debug
   - Include paths: `include/` and `third_party/cjson/`
   - Libraries: cJSON and math library

### Step 2: Understand the Build System

**Objective**: Learn how to compile and test the project.

1. **View available Make targets:**
   ```bash
   make help  # or just read the Makefile
   ```
   
   Available targets:
   - `make` or `make all` - Build the main executable
   - `make debug` - Build with debugging symbols
   - `make test` - Build and run tests
   - `make clean` - Remove build artifacts
   - `make install` - Install to system
   - `make format` - Format code with clang-format

2. **Perform a clean build:**
   ```bash
   make clean
   make
   ```
   
   **Expected output:** 
   - Compilation messages for each source file
   - Object files created in `build/` directory
   - Final executable: `build/dualsync`

3. **Check build artifacts:**
   ```bash
   ls -R build/
   ```

### Step 3: Examine Core Headers

**Objective**: Understand the main data structures and API interfaces.

1. **Start with the main header (`dualsync.h`):**
   ```bash
   cat include/dualsync.h
   ```
   
   **Key elements:**
   - Version constants: `DUALSYNC_VERSION`, `DUALSYNC_VERSION_MAJOR/MINOR/PATCH`
   - Configuration constants: `MAX_PATH_LENGTH`, `CONFIG_DIR_LINUX`, etc.
   - Error codes enumeration: `DUALSYNC_error_t`
   - Forward declarations for main types

2. **Examine each module header systematically:**
   ```bash
   # View all headers
   for header in include/*.h; do
       echo "=== $header ==="
       cat "$header"
       echo ""
   done
   ```

3. **Document the API surface:**
   - Function declarations
   - Type definitions
   - Constants and macros
   - Dependencies between modules

### Step 4: Analyze Source Code Organization

**Objective**: Understand the implementation structure and module relationships.

1. **Count lines of code per module:**
   ```bash
   for dir in src/*/; do
       echo "=== $(basename $dir) ==="
       wc -l "$dir"*.c
   done
   ```

2. **Examine the entry point (`main.c`):**
   ```bash
   cat src/main.c
   ```
   
   **Look for:**
   - Command-line argument parsing
   - Initialization sequence
   - Main program flow
   - Error handling patterns
   - Cleanup procedures

3. **Trace the execution flow:**
   - CLI parsing → Configuration loading → Sync engine → Output
   - Identify the critical path for file synchronization

### Step 5: Inspect Individual Modules

Examine each module in logical order (see [Module-by-Module Analysis](#module-by-module-analysis) section below).

**For each module:**

1. **Read the header file** to understand the interface
2. **Read the implementation files** to understand the logic
3. **Look for:**
   - Function signatures and purposes
   - Data structures and their usage
   - Error handling patterns
   - Dependencies on other modules
   - TODO comments or incomplete features

4. **Document:**
   - Module purpose and responsibilities
   - Key functions and their roles
   - Interaction with other modules
   - Potential improvement areas

### Step 6: Review Configuration Files

**Objective**: Understand how the tool is configured.

1. **Examine JSON schemas:**
   ```bash
   cat config/schemas/config_schema.json
   cat config/schemas/log_schema.json
   ```

2. **Review configuration templates:**
   ```bash
   cat config/templates/sync_config.json
   cat config/templates/extension_mapping.json
   ```

3. **Understand configuration flow:**
   - Where configs are read from
   - How they're validated
   - Default values and overrides

### Step 7: Analyze the Test Suite

**Objective**: Understand how the code is tested and validated.

1. **List all test files:**
   ```bash
   find tests/ -name "*.c" -type f
   ```

2. **Examine unit tests:**
   ```bash
   # Review each unit test
   cat tests/unit/test_config.c
   cat tests/unit/test_filesystem.c
   cat tests/unit/test_sync.c
   cat tests/unit/test_utils.c
   ```

3. **Examine integration tests:**
   ```bash
   cat tests/integration/test_full_sync.c
   cat tests/integration/test_cli.c
   ```

4. **Run the tests:**
   ```bash
   make test
   ```
   
   **Analyze:**
   - Test coverage
   - Test patterns and conventions
   - Edge cases covered
   - Missing test scenarios

### Step 8: Trace Data Flow

**Objective**: Understand how data moves through the system.

1. **Identify key data structures:**
   - Configuration objects
   - File metadata structures
   - Sync state tracking
   - Log entries

2. **Trace a typical synchronization flow:**
   ```
   User Input → CLI Parser → Config Manager → Sync Engine
        ↓
   File Scanner → Conflict Resolver → Filesystem Ops
        ↓
   Backup Manager → Logger → User Output
   ```

3. **Document data transformations:**
   - What formats are used
   - Where conversions happen
   - How state is maintained

### Step 9: Analyze Dependencies

**Objective**: Understand external and internal dependencies.

1. **External dependencies (from submodules):**
   ```bash
   ls -la libs/
   git submodule status
   ```
   
   **cJSON library:**
   - Purpose: JSON parsing and generation
   - Usage: Configuration files, logging
   
   **sha256 library:**
   - Purpose: File integrity verification
   - Usage: Detect file changes, conflict resolution

2. **Internal module dependencies:**
   - Create a dependency graph
   - Identify circular dependencies (should be avoided)
   - Note tightly coupled modules

3. **Check dependency injection patterns:**
   - How modules receive their dependencies
   - Configuration passing mechanisms
   - Resource management

### Step 10: Review Error Handling

**Objective**: Understand how errors are managed throughout the system.

1. **Examine error code definitions:**
   ```bash
   grep -n "typedef enum" include/dualsync.h
   ```

2. **Trace error propagation:**
   - How errors are detected
   - How they're logged
   - How they're reported to users
   - Recovery mechanisms

3. **Check for:**
   - Consistent error handling patterns
   - Memory leak risks in error paths
   - Proper cleanup on failures

### Step 11: Assess Code Quality

**Objective**: Evaluate the overall code quality and maintainability.

1. **Run the code formatter:**
   ```bash
   make format
   git diff  # Check what would change
   ```

2. **Check for common issues:**
   ```bash
   # Look for potential bugs
   grep -r "TODO\|FIXME\|XXX\|HACK" src/
   
   # Check for magic numbers
   grep -r "[^a-zA-Z0-9_][0-9]\{2,\}" src/*.c
   ```

3. **Evaluate:**
   - Code organization and modularity
   - Naming conventions
   - Comment quality and documentation
   - Complexity of functions (should be manageable)

### Step 12: Document Findings

**Objective**: Create a comprehensive understanding document.

1. **Create an inspection report covering:**
   - Architecture overview
   - Module responsibilities
   - Key algorithms and data structures
   - Potential issues or improvements
   - Security considerations
   - Performance characteristics

2. **Update this document** with your findings

3. **Create diagrams** (text-based or using tools like PlantUML)

---

## Module-by-Module Analysis

### 1. Main Entry Point (`src/main.c`)

**Purpose**: Application entry point and orchestration

**Key Responsibilities:**
- Parse command-line arguments
- Initialize all subsystems
- Coordinate the main execution flow
- Handle graceful shutdown

**Inspection Checklist:**
- [ ] Verify proper argument validation
- [ ] Check initialization order
- [ ] Ensure proper cleanup on exit
- [ ] Review error handling for startup failures

**Code Flow:**
```c
main()
  ├── Parse CLI arguments (cli_parser)
  ├── Load configuration (config_manager)
  ├── Initialize logger (logger)
  ├── Initialize sync engine (sync_engine)
  ├── Execute sync operation
  ├── Generate report
  └── Cleanup and exit
```

---

### 2. CLI Module (`src/cli/`)

**Purpose**: Command-line interface and user interaction

**Files:**
- `cli_parser.c` - Parse and validate command-line arguments
- `cli_display.c` - Format and display output to users
- `interactive.c` - Handle interactive mode and prompts

**Key Functions:**
- Parse command-line flags and options
- Validate user input
- Display progress and status
- Handle user confirmations for conflicts

**Inspection Checklist:**
- [ ] Command-line option parsing correctness
- [ ] Help text completeness and accuracy
- [ ] Error message clarity
- [ ] Interactive prompts user-friendliness
- [ ] Edge case handling (empty input, invalid options)

---

### 3. Configuration Module (`src/config/`)

**Purpose**: Configuration management and validation

**Files:**
- `config_manager.c` - Load, save, and manage configurations
- `schema_validator.c` - Validate JSON against schemas

**Key Responsibilities:**
- Load configuration from JSON files
- Validate against schemas
- Provide default values
- Support user preferences
- Handle configuration errors gracefully

**Inspection Checklist:**
- [ ] JSON parsing error handling
- [ ] Schema validation implementation
- [ ] Default value handling
- [ ] Configuration file location resolution
- [ ] Cross-platform path handling

**Configuration Structure:**
```json
{
  "version": "1.0",
  "sync_pairs": [
    {
      "linux_path": "/path/to/folder",
      "windows_path": "/mnt/windows/path",
      "direction": "bidirectional"
    }
  ],
  "options": {
    "backup_enabled": true,
    "conflict_resolution": "ask",
    "log_level": "info"
  }
}
```

---

### 4. Synchronization Module (`src/sync/`)

**Purpose**: Core file synchronization logic

**Files:**
- `sync_engine.c` - Main synchronization orchestration
- `file_scanner.c` - Scan directories and detect changes
- `conflict_resolver.c` - Resolve file conflicts

**Key Algorithms:**
1. **File Scanning:**
   - Traverse directory trees
   - Collect file metadata (size, timestamp, permissions)
   - Calculate file hashes for change detection

2. **Change Detection:**
   - Compare timestamps
   - Compare file hashes
   - Identify new, modified, and deleted files

3. **Conflict Resolution:**
   - Detect simultaneous modifications
   - Apply resolution strategy (newest, manual, etc.)
   - Preserve both versions if needed

**Inspection Checklist:**
- [ ] Efficient directory traversal
- [ ] Proper symlink handling
- [ ] Hidden file handling
- [ ] Large file support
- [ ] Performance optimization (caching, incremental sync)
- [ ] Transaction safety (atomic operations)

**Sync Flow:**
```
Scan Source → Scan Destination → Compare
     ↓
Detect Changes (New/Modified/Deleted)
     ↓
Check for Conflicts
     ↓
Apply Resolution Strategy
     ↓
Execute File Operations (Copy/Update/Delete)
     ↓
Update Sync State
```

---

### 5. Filesystem Module (`src/filesystem/`)

**Purpose**: File system operations abstraction

**Files:**
- `fs_operations.c` - Basic file operations (copy, move, delete)
- `disk_monitor.c` - Monitor disk availability and status
- `path_resolver.c` - Resolve and normalize paths
- `permission_handler.c` - Handle file permissions cross-platform

**Key Responsibilities:**
- Abstract OS-specific file operations
- Handle Linux and Windows path differences
- Manage file permissions and attributes
- Monitor disk space and availability
- Ensure safe file operations (atomic writes, etc.)

**Inspection Checklist:**
- [ ] Cross-platform compatibility (Linux/Windows paths)
- [ ] Permission preservation
- [ ] Error handling for disk full scenarios
- [ ] Atomic file operations
- [ ] Proper handling of special files (symlinks, devices)

**Path Resolution:**
```
Linux:    /home/user/documents/file.txt
Windows:  /mnt/windows/Users/User/Documents/file.txt
Mapped:   {WINDOWS_ROOT}/Users/User/Documents/file.txt
```

---

### 6. Backup Module (`src/backup/`)

**Purpose**: Backup and rollback functionality

**Files:**
- `backup_manager.c` - Manage backup operations
- `cleaner.c` - Clean up old backups
- `rollback_handler.c` - Restore previous states

**Key Features:**
- Create backups before modifications
- Maintain backup history
- Rollback to previous states
- Automatic cleanup of old backups

**Inspection Checklist:**
- [ ] Backup creation before destructive operations
- [ ] Backup integrity verification
- [ ] Rollback reliability
- [ ] Disk space management for backups
- [ ] Backup retention policy

**Backup Structure:**
```
.DUALSYNC/backups/
  ├── 2024-01-15_10-30-45/
  │   ├── metadata.json
  │   └── files/
  ├── 2024-01-15_14-20-10/
  └── latest -> 2024-01-15_14-20-10/
```

---

### 7. Converter Module (`src/converter/`)

**Purpose**: File format conversion between platforms

**Files:**
- `converter_engine.c` - Main conversion logic
- `command_executor.c` - Execute conversion commands
- `extension_mapper.c` - Map file extensions to converters

**Use Cases:**
- Line ending conversion (CRLF ↔ LF)
- File encoding conversion
- Path separator normalization
- Platform-specific file formats

**Inspection Checklist:**
- [ ] Supported conversion types
- [ ] Conversion accuracy
- [ ] Handling of binary files
- [ ] Performance for large files
- [ ] Extensibility for new converters

---

### 8. Logger Module (`src/logger/`)

**Purpose**: Comprehensive logging system

**Files:**
- `logger.c` - Core logging functionality
- `log_formatter.c` - Format log messages
- `log_rotation.c` - Manage log file rotation

**Log Levels:**
- `DEBUG` - Detailed debugging information
- `INFO` - General informational messages
- `WARNING` - Warning messages
- `ERROR` - Error messages
- `CRITICAL` - Critical errors

**Inspection Checklist:**
- [ ] Thread-safe logging (if multi-threaded)
- [ ] Log rotation implementation
- [ ] Performance impact of logging
- [ ] Structured logging support
- [ ] Log file location and naming

**Log Format:**
```
[2024-01-15 10:30:45.123] [INFO] [sync_engine] Starting synchronization
[2024-01-15 10:30:46.456] [DEBUG] [file_scanner] Scanning: /home/user/documents
[2024-01-15 10:30:47.789] [WARNING] [conflict_resolver] Conflict detected: file.txt
```

---

### 9. JSON Module (`src/json/`)

**Purpose**: JSON parsing and generation

**Files:**
- `json_handler.c` - JSON manipulation utilities
- `schema_loader.c` - Load and parse JSON schemas

**Responsibilities:**
- Parse JSON configuration files
- Generate JSON reports
- Validate JSON against schemas
- Handle JSON errors gracefully

**Inspection Checklist:**
- [ ] cJSON library integration
- [ ] Memory management (JSON parsing can be memory-intensive)
- [ ] Error handling for malformed JSON
- [ ] Schema validation coverage

---

### 10. Utils Module (`src/utils/`)

**Purpose**: Common utility functions

**Files:**
- `error_handling.c` - Error management utilities
- `hash_utils.c` - File hashing (SHA-256)
- `string_utils.c` - String manipulation utilities
- `time_utils.c` - Time and date utilities

**Key Utilities:**
- String operations (copy, compare, format)
- File hashing for integrity checks
- Time formatting and parsing
- Error code to string conversion

**Inspection Checklist:**
- [ ] Buffer overflow protection in string operations
- [ ] Hash calculation efficiency
- [ ] Time zone handling
- [ ] Error message clarity

---

## Architecture Overview

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    User Interface (CLI)                  │
│                                                           │
│  ┌─────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │ CLI Parser  │  │  Interactive │  │ CLI Display  │   │
│  └─────────────┘  └──────────────┘  └──────────────┘   │
└─────────────────────────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────┐
│                   Configuration Layer                    │
│                                                           │
│  ┌──────────────────┐  ┌──────────────────────────┐    │
│  │ Config Manager   │  │  Schema Validator        │    │
│  └──────────────────┘  └──────────────────────────┘    │
└─────────────────────────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────┐
│                   Synchronization Core                   │
│                                                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │ Sync Engine  │  │File Scanner  │  │  Conflict    │  │
│  │              │  │              │  │  Resolver    │  │
│  └──────────────┘  └──────────────┘  └──────────────┘  │
└─────────────────────────────────────────────────────────┘
                             │
                ┌────────────┼────────────┐
                ▼            ▼            ▼
┌────────────────────┐  ┌──────────┐  ┌─────────────┐
│   Filesystem Ops   │  │  Backup  │  │  Converter  │
│                    │  │  Manager │  │   Engine    │
│  • FS Operations   │  │          │  │             │
│  • Path Resolver   │  │          │  │             │
│  • Disk Monitor    │  │          │  │             │
│  • Permissions     │  │          │  │             │
└────────────────────┘  └──────────┘  └─────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────┐
│                   Supporting Services                    │
│                                                           │
│  ┌────────┐  ┌──────────────┐  ┌────────────────────┐  │
│  │ Logger │  │ JSON Handler │  │  Utilities (Hash,  │  │
│  │        │  │              │  │  String, Time)     │  │
│  └────────┘  └──────────────┘  └────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Data Flow Diagram

```
┌─────────────┐
│  User Input │
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│   Parse Arguments   │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Load Configuration │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│   Scan File Trees   │
│   (Source & Dest)   │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Calculate Hashes   │
│  (Changed Files)    │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Detect Differences │
│  (New/Mod/Deleted)  │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ Identify Conflicts  │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Resolve Conflicts  │
│  (Auto or Manual)   │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Create Backups     │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Apply Changes      │
│  (Copy/Update/Del)  │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│   Update Logs       │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│  Display Results    │
└─────────────────────┘
```

### Component Dependencies

```
main.c
  ├─→ cli_parser (CLI input handling)
  ├─→ config_manager (Load configuration)
  ├─→ logger (Initialize logging)
  └─→ sync_engine (Execute sync)
        ├─→ file_scanner (Scan directories)
        │     ├─→ fs_operations (File I/O)
        │     ├─→ path_resolver (Path handling)
        │     └─→ hash_utils (Calculate hashes)
        ├─→ conflict_resolver (Handle conflicts)
        │     └─→ cli_display (User prompts)
        ├─→ backup_manager (Create backups)
        │     └─→ fs_operations (Backup files)
        └─→ converter_engine (Convert files)
              ├─→ extension_mapper (Map extensions)
              └─→ command_executor (Run converters)
```

---

## Development Workflow

### Setting Up Development Environment

1. **Clone and initialize:**
   ```bash
   git clone --recurse-submodules https://github.com/PT-LoiX86/Dual-Boot-File-Synchronization-Tool.git
   cd Dual-Boot-File-Synchronization-Tool
   ```

2. **Build in debug mode:**
   ```bash
   make debug
   ```

3. **Run tests:**
   ```bash
   make test
   ```

### Making Changes

1. **Create a feature branch:**
   ```bash
   git checkout develop
   git pull origin develop
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes:**
   - Edit source files
   - Update headers if needed
   - Add tests for new functionality

3. **Format your code:**
   ```bash
   make format
   ```

4. **Build and test:**
   ```bash
   make clean
   make debug
   make test
   ```

5. **Commit and push:**
   ```bash
   git add .
   git commit -m "Description of changes"
   git push origin feature/your-feature-name
   ```

6. **Create a pull request** to the `develop` branch

### Debugging Tips

1. **Build with debug symbols:**
   ```bash
   make debug
   ```

2. **Use GDB for debugging:**
   ```bash
   gdb ./build/dualsync
   (gdb) break main
   (gdb) run
   (gdb) next
   ```

3. **Enable verbose logging:**
   - Set log level to DEBUG in configuration
   - Check logs in `.DUALSYNC/logs/`

4. **Use Valgrind for memory issues:**
   ```bash
   valgrind --leak-check=full ./build/dualsync
   ```

---

## Testing Strategy

### Unit Tests

**Location:** `tests/unit/`

**Purpose:** Test individual functions and modules in isolation

**Coverage:**
- Configuration parsing
- File scanning logic
- Hash calculation
- Path resolution
- String utilities
- Error handling

**Running unit tests:**
```bash
make test
```

### Integration Tests

**Location:** `tests/integration/`

**Purpose:** Test end-to-end scenarios

**Scenarios:**
- Full synchronization flow
- CLI argument parsing
- Configuration loading
- Conflict resolution
- Backup and rollback

### Manual Testing

**Test Cases:**

1. **Basic Synchronization:**
   - Create test files in source directory
   - Run sync
   - Verify files appear in destination
   - Verify file content integrity (hashes)

2. **Conflict Handling:**
   - Modify same file in both locations
   - Run sync
   - Verify conflict detection
   - Test resolution strategies

3. **Backup and Rollback:**
   - Run sync with backups enabled
   - Check backup creation
   - Modify files
   - Test rollback functionality

4. **Cross-Platform Testing:**
   - Test on Linux
   - Test on Windows mount
   - Verify path handling
   - Verify permission preservation

---

## Security Considerations

### Potential Security Issues to Inspect

1. **Path Traversal:**
   - Check for proper path validation
   - Ensure no directory traversal attacks (../)
   - Validate symbolic link handling

2. **Permission Handling:**
   - Verify proper permission checks
   - Ensure no privilege escalation
   - Check SETUID/SETGID handling

3. **Input Validation:**
   - Validate all user inputs
   - Check for buffer overflows
   - Sanitize file names

4. **Resource Exhaustion:**
   - Check for memory leaks
   - Verify disk space checks
   - Limit recursion depth

5. **Data Integrity:**
   - Verify hash calculations
   - Ensure atomic file operations
   - Check backup integrity

---

## Performance Considerations

### Optimization Opportunities

1. **Incremental Sync:**
   - Only process changed files
   - Use change detection (timestamps, hashes)
   - Cache previous sync state

2. **Parallel Processing:**
   - Scan directories in parallel
   - Copy multiple files concurrently
   - Hash files in parallel

3. **Memory Efficiency:**
   - Stream large files instead of loading into memory
   - Use memory pools for frequent allocations
   - Implement lazy loading

4. **Disk I/O:**
   - Batch file operations
   - Use buffered I/O
   - Minimize disk seeks

### Profiling

```bash
# Compile with profiling
gcc -pg -o dualsync src/*.c

# Run the program
./dualsync

# Generate profile report
gprof dualsync gmon.out > profile.txt
```

---

## Common Issues and Troubleshooting

### Build Issues

**Problem:** `cannot find -lcjson`
**Solution:** Ensure submodules are initialized and cJSON is built
```bash
git submodule update --init --recursive
cd libs/cjson
make
```

**Problem:** `undefined reference to...`
**Solution:** Check Makefile for correct linking order and missing object files

### Runtime Issues

**Problem:** Configuration file not found
**Solution:** Check configuration path resolution and create default config

**Problem:** Permission denied errors
**Solution:** Verify user has read/write permissions on sync directories

**Problem:** Disk space errors
**Solution:** Check available disk space before sync operations

---

## Contributing Guidelines

### Code Style

- Use consistent indentation (spaces or tabs)
- Follow existing naming conventions
- Add comments for complex logic
- Keep functions focused and small
- Use descriptive variable names

### Documentation

- Update README when adding features
- Document public APIs in headers
- Add inline comments for tricky code
- Update this inspection guide with findings

### Pull Request Process

1. Branch from `develop`
2. Make focused, atomic commits
3. Write clear commit messages
4. Ensure all tests pass
5. Update documentation
6. Request review from maintainers
7. Address review feedback
8. Merge to `develop` (maintainers only)

---

## Additional Resources

### External Documentation

- [cJSON Library Documentation](https://github.com/DaveGamble/cJSON)
- [SHA-256 Implementation](https://github.com/B-Con/crypto-algorithms)
- [C11 Standard Reference](https://en.cppreference.com/w/c/11)
- [GNU Make Manual](https://www.gnu.org/software/make/manual/)

### Related Projects

- rsync - File synchronization tool
- Syncthing - Continuous file synchronization
- Unison - Cross-platform file synchronization

---

## Conclusion

This inspection guide provides a comprehensive framework for understanding the Dual-Boot File Synchronization Tool codebase. Follow the step-by-step process to gain a thorough understanding of:

- Project structure and organization
- Module responsibilities and interactions
- Build and test processes
- Data flow and architecture
- Development workflow

As you inspect the code, update this document with your findings, insights, and recommendations for improvements.

**Happy inspecting! 🔍**

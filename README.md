# Dual-Boot File Synchronization Tool

## About

**Concept:** An open-source tool for file synchronization when working on dual-boot (Ubuntu (main) - Windows).

**Docs:** All about the project will be described in the following sheet:  
 `https://docs.google.com/spreadsheets/d/1CnZLICE2wTZ6S9hp4bg_5lvL-tPmGUVPoM-AeqvXKUE/edit?usp=sharing`

## For contributors

All changes must be committed into develop branch, not main.

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
    make install      # Install dualsync to /usr/local/bin/
    make format       # Auto-format all source code

### Third-Party Dependencies

This project uses the following libraries as git submodules:

- **cJSON** - Lightweight JSON parsing library
- **sha256** - File hashing for integrity verification

These are automatically downloaded when you clone with `--recurse-submodules` or run `git submodule update --init --recursive`.

## Development

### Updating Submodules

    git submodule update --remote --merge

### Building for Debug

    make debug
    ./build/dualsync

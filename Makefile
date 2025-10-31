# --Compiler and flags--
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
DEBUG_FLAGS = -DDEBUG -g -O0
INCLUDE_DIRS = -Iinclude -Ithird_party/cjson
LIB_DIRS = -Lthird_party/cjson
LIBS = -lcjson -lm

# --Directories--
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
INCLUDE_DIR = include

# --Source files--
SOURCES = $(wildcard $(SRC_DIR)/*.c) \
          $(wildcard $(SRC_DIR)/*/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# --Test files--
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c) \
               $(wildcard $(TEST_DIR)/*/*.c)
TEST_OBJECTS = $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%.o)

# --Target executable--
TARGET = $(BUILD_DIR)/dualsync
TEST_TARGET = $(BUILD_DIR)/test_runner

# --Default target--
all: $(TARGET)

# --Build main executable--
$(TARGET): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LIB_DIRS) $(LIBS)

# --Compile source files--
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

# --Create build directory--
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# --Debug build--
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# --Test build--
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) | $(BUILD_DIR)
	$(CC) $(TEST_OBJECTS) $(filter-out $(BUILD_DIR)/main.o, $(OBJECTS)) -o $@ $(LIB_DIRS) $(LIBS)

$(BUILD_DIR)/test_%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

# --Install--
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/dualsync
	sudo chmod +x /usr/local/bin/dualsync

# --Clean--
clean:
	rm -rf $(BUILD_DIR)

# --Format code--
format:
	find $(SRC_DIR) $(INCLUDE_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i

.PHONY: all debug test install clean format

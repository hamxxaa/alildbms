CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = src
INCLUDE_DIR = src/include
OBJ_DIR = obj
HASH_DIR = hashmaps
META_DIR = metadatas
BIN_DIR = bins

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
EXECUTABLE = dbms

# Ensure the object directory exists before building
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Create the object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean rule to remove all build artifacts
clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)

fclean: clean
	rm -rf $(HASH_DIR)/* $(META_DIR)/* $(BIN_DIR)/*

valgrind: $(EXECUTABLE)
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all --verbose ./$(EXECUTABLE)

# Generate dependencies for header files
-include $(OBJECTS:.o=.d)

# Rule to generate dependency files
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) -MM -I$(INCLUDE_DIR) $< > $@.tmp
	sed 's|\($*\)\.o[ :]*|$(OBJ_DIR)/\1.o $(OBJ_DIR)/\1.d: |g' < $@.tmp > $@
	rm -f $@.tmp

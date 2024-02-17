# File for test cases

CC = gcc
FLAGS = -std=c99 -DTEST -DUNITY_INCLUDE_CONFIG_H -g3
BUILD_DIR = $(UNITY_DIR)/build
TARGET = $(BUILD_DIR)/tests.out
CFLAGS = -DTEST -DUNITY_INCLUDE_CONFIG_H
TESTS_DIR = Tests
UNITY_DIR = External/Unity-2.5.2
BOOTLOADER = External/bootloader

C_INCLUDES += \
-I$(BOOTLOADER)/Inc \
-I$(UNITY_DIR)/src \
-I$(UNITY_DIR)/extras/fixture/src \
-I$(UNITY_DIR)/extras/memory/src \
-I$(TESTS_DIR) \
-I$(TESTS_DIR)/mocks/Inc \
-ICore/Inc \

C_SOURCES += \
$(BOOTLOADER)/Src/bootloader_cmd.c \
$(UNITY_DIR)/src/unity.c \
$(UNITY_DIR)/extras/fixture/src/unity_fixture.c \
$(UNITY_DIR)/extras/memory/src/unity_memory.c \
$(TESTS_DIR)/host_tests.c \
$(TESTS_DIR)/host_tests/bootloader/bootloader_test_runner.c \
$(TESTS_DIR)/host_tests/bootloader/bootloader_test.c \
$(TESTS_DIR)/mocks/Src/mock_bootloader_io.c

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))

all: $(TARGET)

vpath %.c $(dir $(C_SOURCES))

$(BUILD_DIR)/%.o: %.c
	$(CC) $(FLAGS) $(CFLAGS) -MD $(C_INCLUDES) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(TARGET)

.PHONY = start
start: $(TARGET)
	./$(TARGET) -v # -v - print tests

.PHONY = clean
clean:
	rm -f $(BUILD_DIR)/*

-include $(OBJECTS:.o=.d)

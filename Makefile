# compiler
CXX	:= clang++
CC	:= clang

CBASEFLAGS	:= -MD -O2
CBASEFLAGS	+= -Wall -Wshadow -Werror
CBASEFLAGS	+= -Iinclude

CFLAGS := $(CBASEFLAGS)

CXXFLAGS := -std=c++17
CXXFLAGS += $(CBASEFLAGS)
CXXFLAGS += -Wnull-dereference

LDFLAGS	:= -static

# output
OBJ_DIR := build
SRC_DIR := source
OUTPUT  := bin/snmy.exe

SRCS_CPP	:= $(shell find $(SRC_DIR) -name *.cpp)
SRCS_C		:= $(shell find $(SRC_DIR) -name *.c)

OBJS := $(subst $(SRC_DIR),$(OBJ_DIR),$(SRCS_CPP:.cpp=.o))
OBJS += $(subst $(SRC_DIR),$(OBJ_DIR),$(SRCS_C:.c=.o))
DEPS := $(OBJS:.o=.d)

-include $(DEPS)

all: $(OUTPUT)

# building
$(OUTPUT): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(C) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build/*.o build/*.d $(OUTPUT)


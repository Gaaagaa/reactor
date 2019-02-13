#========================================
# Target name

TARGET = ../bin/reactor

# makefile command: make CC_EXTRA='-D _DEBUG'
# makefile command: make CC_EXTRA='-D _DEBUG -g'
# startup  command: ./reactor -d -l log.conf -r run.conf

#========================================
# Compiler

CC = g++

#========================================
# Options

CC_EXTRA = 
CC_FLAGS = -Wall -O2 -std=c++11 $(CC_EXTRA)
LD_FLAGS = -lpthread -llog4cpp -ljsoncpp

#========================================
# Paths

TMP_DIR = ../tmp
OBJ_DIR = $(TMP_DIR)/server/obj

SRC_DIR = .
INC_DIR = -I./comm -I./xtcp -I./xftp -I../third/include
LIB_DIR = -L../third/lib

# 递归遍历 5 级子目录
DIRS := $(shell find $(SRC_DIR) -maxdepth 5 -type d)

# 将每个子目录添加到搜索路径
VPATH = $(DIRS)

#========================================
# Modules

SRCS = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(SRCS)))

# OBJS = $(addprefix $(OBJ_DIR)/, $(patsubst %.cpp, %.o, $(notdir $(SRCS))))

#========================================
# Build

$(TARGET):$(OBJS)
	$(CC) $^ $(LIB_DIR) $(LD_FLAGS) -o $(TARGET)

$(OBJ_DIR)/%.o:%.cpp
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi;\
	$(CC) $(INC_DIR) $(CC_FLAGS) -c $< -o $@

#$(OBJ_DIR)/%.o:%.cpp
#	if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;\
#	$(CC) $(INC_DIR) $(CC_FLAGS) -c $< -o $@

#========================================
# PHONY

.PHONY: clean

clean:
	@rm -rvf $(TMP_DIR)

#========================================

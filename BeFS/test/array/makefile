## array-test makefile -- axeld.
# The BeOS makefile-engine doesn't support compiling source files
# in another directory, but this one does

##
## Environment
##

SOURCES= array.cpp Utility.cpp
NAME= arrayTest
TARGET_DIR=.

CFLAGS= -g -O0 -DUSER -DDEBUG -fno-exceptions -fno-rtti #-fcheck-memory-usage -D_NO_INLINE_ASM
INCLUDES= -I../../source
LIBS= -lbe

##
## Targets
##

default: $(NAME)

clean:
	@rm -f $(OBJ_DIR)/*.o

tar zip backup:
	zip -y $(NAME)-`date +%Y-%m-%d`.zip *.[ch]* makefile ToDo *.rsrc test/*.[ch]* test/makefile

##
## Build stuff
##

CC=gcc
PLATFORM=x86

define SOURCES_LIST_TO_OBJECTS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(foreach file, $(SOURCES), \
	$(basename $(notdir $(file))))))
endef

OBJ_DIR=obj.$(PLATFORM)
OBJECTS= $(SOURCES_LIST_TO_OBJECTS)

$(NAME): $(OBJ_DIR) $(OBJECTS)
	$(CC) -o $(TARGET_DIR)/$(NAME) $(OBJECTS) $(LIBS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# the order of the last two rules is important!

$(OBJ_DIR)/%.o : %.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $(OBJ_DIR)/$*.o $*.cpp

$(OBJ_DIR)/%.o : ../../source/%.cpp
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $(OBJ_DIR)/$*.o ../../source/$*.cpp


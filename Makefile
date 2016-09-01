### DIRS ##################################################################

SRC_DIR = src
OBJ_DIR = obj

### OBJS ##################################################################

HTAB_OBJS 	= $(OBJ_DIR)/hashset/hashtable.o
HSET_OBJS 	= $(HTAB_OBJS) $(OBJ_DIR)/hashset/string_hashset.o $(OBJ_DIR)/hashset/fd_hashset.o
PACKET_OBJS = $(OBJ_DIR)/packet/packet.o $(OBJ_DIR)/packet/serializer.o
QUEUE_OBJS 	= $(OBJ_DIR)/queue/queue.o
USERS_OBJS	= $(OBJ_DIR)/server/users.o
SOCKET_OBJS = $(OBJ_DIR)/server/server_speaker.o $(OBJ_DIR)/server/server_listener.o
SERVER_OBJS = $(HSET_OBJS) $(PACKET_OBJS) $(QUEUE_OBJS) $(USERS_OBJS) $(SOCKET_OBJS) 

OBJS = $(SERVER_OBJS)
TESTEXES = 
EXES = server

### FLAGS #################################################################

CFLAGS = -Wall -Wextra -ansi -pedantic -g -O
DBGFLAGS = -DDEBUG #-DPDEBUG
LFLAGS = -pthread

### COMMANDS ##############################################################

CC 		= gcc
RM 		= rm -f -v
COMPILE = $(CC) $(CFLAGS) $(DBGFLAGS)

### RULES #################################################################

all: $(EXES)

tests: $(TESTEXES)

#serialsolver: $(SRC_DIR)/server.c $(SERVER_OBJS)
server: $(SERVER_OBJS) $(SRC_DIR)/server/chat_server.c
	$(COMPILE) -o $@ $^ $(LFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILE) -c -o $@ $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(EXES) $(TESTEXES)
#	$(RM) $(OBJ_DIR)/*.o
#	$(RM) $(OBJ_DIR)/server/*.o


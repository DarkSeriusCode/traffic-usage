PROJECT_NAME = traffic_usage
INSTALL_PREFIX = /usr/local

CC = clang
CFLAGS = -Wall
LDFLAGS = -lncurses

ifdef debug
	CFLAGS += -O0 -g
else
	CFLAGS += -O2
endif

OBJ_DIR = obj
BIN_DIR = bin

SRC     = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJ     = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
OBJ     += main.o

$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR))

.PHONY: all
all: $(BIN_DIR)/$(PROJECT_NAME)

$(BIN_DIR)/$(PROJECT_NAME): $(OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: install
install: $(BIN_DIR)/$(PROJECT_NAME)
	install -m 755 $< $(INSTALL_PREFIX)/bin

.PHONY: uninstall
uninstall:
	rm $(INSTALL_PREFIX)/bin/$(PROJECT_NAME)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

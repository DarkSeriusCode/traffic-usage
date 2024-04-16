PROJECT_NAME = traffic_usage
INSTALL_PREFIX = /usr/local

CC = clang
CFLAGS = -Wall -g
LDFLAGS = -lncurses

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
OBJ += main.o

$(shell mkdir -p bin obj)

.PHONY: all
all: $(OBJ)
	$(CC) -o bin/$(PROJECT_NAME) $^ $(CFLAGS) $(LDFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: install
install: bin/$(PROJECT_NAME)
	install -m 755 $< $(INSTALL_PREFIX)/bin

.PHONY: uninstall
uninstall:
	rm $(INSTALL_PREFIX)/bin/$(PROJECT_NAME)

.PHONY: clean
clean:
	rm -rf bin $(OBJ) obj

NAME := nothydrus
CC := gcc
LIB := $(shell pkg-config --libs sqlite3) $(shell pkg-config --libs libxxhash) $(shell pkg-config --libs notcurses)
CFLAGS := -Wall -Wextra $(shell pkg-config --cflags notcurses)
SOURCES := $(shell find src/ -type f -name *.c)
HEADERS := $(shell find include/ -type f -name *.h)
OBJECTS := $(patsubst src/%.c, build/%.o, $(SOURCES))
DEPS := $(patsubst src/%.c, build/%.d, $(SOURCES))


all: $(NAME)

$(NAME): $(OBJECTS)
	@mkdir -p build/util
	$(CC) $(OBJECTS) -o $@ $(LIB)

-include $(DEPS)

build/%.o: src/%.c
	@mkdir -p build/util
	$(CC) -MMD -I include -I src $(CFLAGS) -c $< -o $@

clean:
	rm -rd build/
	rm $(NAME)

install: all
	install -D -t $(DESTDIR)$(PREFIX)/bin $(NAME)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME)

.PHONY: clean all install uninstall

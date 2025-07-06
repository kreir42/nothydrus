NAME := nothydrus
CC := gcc
LIB := $(shell pkg-config --libs sqlite3) $(shell pkg-config --libs libxxhash) $(shell pkg-config --libs notcurses)
CFLAGS_BASE := -Wall -Wextra $(shell pkg-config --cflags notcurses)
SOURCES := $(shell find src/ -type f -name *.c)
HEADERS := $(shell find include/ -type f -name *.h)
OBJECTS := $(patsubst src/%.c, build/%.o, $(SOURCES))
DEPS := $(patsubst src/%.c, build/%.d, $(SOURCES))

CFLAGS := $(CFLAGS_BASE) -g -DDEBUG

all: $(NAME)

$(NAME): $(OBJECTS)
	@mkdir -p build/util
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIB)

-include $(DEPS)

build/%.o: src/%.c
	@mkdir -p build/util
	$(CC) -MMD -I include -I src $(CFLAGS) -c $< -o $@

release: CFLAGS := $(CFLAGS_BASE) -O2 -DNDEBUG
release: $(NAME)

clean:
	rm -rd build/
	rm $(NAME)

install: all
	install -D -t $(DESTDIR)$(PREFIX)/bin $(NAME)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME)

.PHONY: clean all install uninstall release

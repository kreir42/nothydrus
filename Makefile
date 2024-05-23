NAME := nothydrus
CC := gcc
LIB :=
CFLAGS := -Wall -Wextra
SOURCES := $(shell find src/ -type f -name *.c)
HEADERS := $(shell find include/ -type f -name *.h)
OBJECTS := $(patsubst src/%.c, build/%.o, $(SOURCES))
DEPS := $(patsubst src/%.c, build/%.d, $(SOURCES))


all: $(NAME)

$(NAME): $(OBJECTS)
	@mkdir -p build
	$(CC) $(OBJECTS) -o $@ $(LIB)

-include $(DEPS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) -MMD -I include -I src $(CFLAGS) -c $< -o $@

clean:
	rm -rd build/
	rm $(NAME)

.PHONY: clean all

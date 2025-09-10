.POSIX:

NAME    = lf
VERSION = 1.6

CC     = cc
CFLAGS = -std=c99 -D_DEFAULT_SOURCE -Wall -Wextra -pedantic -DVERSION=\"$(VERSION)\"

BIN_DIR = /usr/local/bin

SRC = lf.c
OBJ = lf.o

all: $(NAME)
$(NAME): $(OBJ)
install: all
	@mkdir -p $(BIN_DIR)
	@mv $(NAME) $(BIN_DIR)
	@rm -f $(OBJ)
uninstall:
	@rm -f $(BIN_DIR)/$(NAME)
clean:
	@rm -f $(OBJ) $(NAME)

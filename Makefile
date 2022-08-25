CC = cc
NAME = mare

OUT = $(NAME)

CFLAGS = -Wall -std=c99
LFLAGS = -ldl -lm

SRC = mare.c
OBJ = $(SRC:%.c=%.o)
DOBJ = $(SRC:%.c=%.do)

LIBNAME = lib$(NAME)
SLIBNAME = $(LIBNAME).a
DLIBNAME = $(LIBNAME).so

.PHONY: all test

test: $(OUT)

all: $(SLIBNAME) $(DLIBNAME)

$(OUT): main.c $(SLIBNAME) mare.h
	$(CC) main.c -o $@ $(CFLAGS) -L. -lmare $(LFLAGS) -lSDL2

$(SLIBNAME): $(OBJ)
	$(AR) rcs $@ $^

$(DLIBNAME): $(DOBJ)
	$(CC) $^ -o $@ -shared $(CFLAGS) $(LFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.do: %.c
	$(CC) -c $< -o $@ -fPIC $(CFLAGS)

clean:
	rm -f $(OUT)
	rm -f $(OBJ) $(DOBJ)
	rm -f $(SLIBNAME) $(DLIBNAME)

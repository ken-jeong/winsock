CC      = gcc
CFLAGS  = -Wall -O2
LDFLAGS = -lws2_32

MIDTERM_SRCS = $(wildcard 01_midterm/*.c)
FINAL_SRCS   = $(wildcard 02_final/*.c)

MIDTERM_BINS = $(MIDTERM_SRCS:.c=.exe)
FINAL_BINS   = $(FINAL_SRCS:.c=.exe)

.PHONY: all midterm final clean

all: midterm final

midterm: $(MIDTERM_BINS)

final: $(FINAL_BINS)

%.exe: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(MIDTERM_BINS) $(FINAL_BINS)

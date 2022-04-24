CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Werror

OBJS = main.o

all: ctrlhandlerdemo

ctrlhandlerdemo: $(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o ctrlhandlerdemo.exe

%.o: %.c
	$(CC) $(CFLAGS) -c $<

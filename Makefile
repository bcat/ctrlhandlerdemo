CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Werror

bin = ctrlhandlerdemo.exe
objs = main.o

all: $(bin)

$(bin): $(objs)
	$(CC) $(CFLAGS) $(objs) -o $(bin)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(bin) $(objs)

.PHONY: all clean

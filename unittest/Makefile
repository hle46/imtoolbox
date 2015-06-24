CC = clang++
CFLAGS = -Wall -Wextra -std=c++11 -g -DDEBUG
LDFLAGS = -I/usr/local/lib -ljpeg -lpng
LIBS = -stdlib=libc++
SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
EXES = $(SRCS:.cc=)

all: $(EXES)

%:%.o
	$(CC) $(LIBS) $(LDFLAGS) $< -o $@ 

%.o:%.cc
	$(CC) -c $(LIBS) $(CFLAGS) $< -o $@

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

include .depend

clean:
	rm -f *.o $(EXES) .depend
.PHONY: all clean



CC = g++-4.9
CFLAGS = -Wall -Wextra -std=c++11 -DDEBUG -O3 -ftree-vectorize -ffast-math
LDFLAGS = -L/usr/local/lib -ljpeg -lpng
#LIBS = -stdlib=libc++
SRCS = $(wildcard *.cc)
OBJS = $(SRCS:.cc=.o)
EXES = $(SRCS:.cc=)

all: $(EXES)

%:%.o
	$(CC) $(LIBS)  $< -o $@ $(LDFLAGS)

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



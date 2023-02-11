PREFIX=$(HOME)/usr

CC = gcc
CFLAGS = -Wall -fPIC -O3 -funroll-loops -I.
LD = ld
LDFLAGS = -shared

OBJ = jyp1.o

DLIB = libjyp1.so

TARGET = $(DLIB) sample

all: $(TARGET)
$(DLIB): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIB)
%.o: %.c
	$(CC) $(CFLAGS) -c $<
sample: sample.c $(DLIB)
	$(CC) $(CFLAGS) -L$(PREFIX)/lib -L. -o $@ $< -ljyp1
install:
	install -m 644 *.h $(PREFIX)/include
	install -m 755 $(DLIB) $(PREFIX)/lib
clean:
	-rm -f *.o *~ $(TARGET)

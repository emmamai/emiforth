OBJS	:= main.o
DEFINES	:=
CFLAGS	:= -Wall -O0 -g
CC  	:= gcc
OUT 	:= emiforth

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJS)

clean:
	-@rm -f *.o $(OUT)

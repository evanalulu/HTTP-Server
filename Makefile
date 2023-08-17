PROG = project05
OBJS = project05.o server.o network.o

CFLAGS = -g

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

all : $(PROG)

$(PROG) :  $(OBJS)
	gcc $(CFLAGS) $(LIBS) -o $@ $(OBJS)

clean :
	rm -rf $(PROG) $(OBJS)

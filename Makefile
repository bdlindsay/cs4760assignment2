CC = gcc
CFLAGS = -g
RM = rm

EXEM = master
EXES = slave
SRCSM = master.c
SRCSS = slave.c
OBJSM = ${SRCSM:.c=.o}
OBJSS = ${SRCSS:.c=.o}

.c:.o
	$(CC) $(CFLAGS) -c $<

all : $(EXEM) $(EXES)

$(EXEM) : $(OBJSM)
	$(CC) -o $@ $(OBJSM)

$(OBJSM) : master.h

$(EXES) : $(OBJSS)
	$(CC) -o $@ $(OBJSS)

$(OBJSS) : master.h

clean :
	$(RM) -f $(EXES) $(EXEM) $(OBJSS) $(OBJSM)







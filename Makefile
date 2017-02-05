all:
	gcc -o utmp-popper utmp-popper.c

debug:
	gcc -D_DEBUG -o utmp-popper -fbuiltin -O0 -g3 -ggdb3 utmp-popper.c

osxdebug:
	gcc -D_OSX -D_DEBUG -o utmp-popper -fbuiltin -O0 -g3 -ggdb3 utmp-popper.c



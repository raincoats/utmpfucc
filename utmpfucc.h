#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <paths.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <stdbool.h>
#include <err.h>
#include <time.h>
#include <assert.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <utmp.h>

#define mw(function) do { function; } while (false)
#define die(e)   mw( perror(e); exit(1) )

typedef struct utmp utmp;

char *format_record(utmp *);
void do_getopts(int, char **);
void print_record(struct utmp *);
char *divine_ip_from_environ(void);

int verbose    = false;
int only_dump  = false;
int dry_run    = false;
int file_given = false;
int divine_ip  = true;
char utmp_filename[512];

#ifndef UTMPFUCC_H
#define UTMPFUCC_H

#define PROG_VERSION "0.1"
#define PROG_NAME    "utmpfucc"

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
#include <fcntl.h>

#define mw(function) do { function; } while (false)
#define die(e)   mw( perror(e); exit(1) )

typedef struct utmp utmp;
typedef struct lastlog lastlog;

char *format_record(utmp *);
void do_getopts(int, char **);
void print_record(struct utmp *);
char *divine_ip_from_environ(void);
void do_llog_file(char *);
void *malloc_and_check(int);
void check_and_free(void *);
void _(char *, ...);
void _1(char *, ...);
void _2(char *, ...);
void _e(char *, ...);
void _w(char *, ...);
char *ip_to_string(void *);
char *time_to_string(time_t);
in_addr_t string_to_ip(char *);
int can_we_write_file(char *);
void usage();
void version();

// options
int verbose;
int dry_run;
int divine_ip;
int custom_file;
char *custom_file_path;
int skip_utmp;
int skip_wtmp;
int skip_llog;


// the target ip will go here
in_addr_t target;

// the name the program was called as. for use with --help
char *argv0;



#endif

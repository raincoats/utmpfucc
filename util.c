#include "utmpfucc.h"

/*
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 * util.c util.c uti    formatting and conversion functions   l.c util.c util.c
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 * util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c util.c
 */

/*
 *   variadic formatting functions
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *   
 *   _()  = general messages, always enabled
 *   _1() = verbose level 1, enabled with -v
 *   _2() = verbose level 2, enabled with -vv
 *   _w() = warning, always enabled
 *   _e() = error, always enable, exits program
 *
 *   i wish i could figure out how to make variadic funcs call other variadic
 *   funcs
 *
 */
void _(char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;082m[$]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

void _1(char *fmt, ...)
{
	va_list args;
	if (verbose < 1)
		return;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;121m[1]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

void _2(char *fmt, ...)
{
	va_list args;
	if (verbose < 2)
		return;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;192m[2]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

void _w(char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;220m[!]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

void _e(char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;196m[!]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
    exit(1);
}


/*
 *   format_record()
 *   ~~~~~~~~~~~~~~~
 *   format a utmp record in the style of who(1), with an added pid column
 */
char *format_record(utmp *r)
{
	char comment[32];
	char *timebuf;
	char *ip;

	timebuf = time_to_string((time_t)r->ut_tv.tv_sec);
	ip = ip_to_string(&(r->ut_addr));

	memset(comment, '\0', sizeof(comment));

	if (r->ut_addr != 0) {
		snprintf(comment, sizeof(comment), "(%s)", ip);
	}
	else if (strlen(r->ut_host) > 0) {
		snprintf(comment, sizeof(comment), "(%s)", r->ut_host);
	}

	char *ret;
	if (! asprintf(&ret, "%-8d %-8s %-12s %-16s %s",
		r->ut_pid, r->ut_user, r->ut_line, timebuf, comment)) {
		die("asprintf error");
	}

	return ret;
}

/*
 *    print_record()
 *    ~~~~~~~~~~~~~~
 *    print a record made in format_record(), and then free it because we used
 *    asprintf(3). we want valgrind to pat us on the head and give us an A+
 */
void print_record(utmp *r)
{
	char *line = format_record(r);
	printf("%s\n", line);
	free(line);
}

/*
 *   time_to_string()
 *   ~~~~~~~~~~~~~~~~
 *   it takes a time in seconds and converts it to like 2017-02-13 09:40
 */
char *time_to_string(time_t sec)
{
	_2("time_to_string(): sec=%d", sec);

	assert(sec > 0);

	int buflen = 512;
	char *buf = (char *)malloc(buflen);
	time_t *s = &sec;
	struct tm *tm = localtime(s);

	assert(tm != NULL);

	if (buf == NULL)
		die("malloc");

	if (! strftime(buf, buflen, "%F %H:%M", tm))
		_w("error in strftime");

	return buf;
}


/*
 *   ip_to_string()
 *   ~~~~~~~~~~~~~~
 *   this function exists to isolate the verbose ass error checking. maybe it
 *   should/could be inlined
 */
char *ip_to_string(void *ip)
{
	_2("ip_to_string(): ip=0x%08x", (int *)ip);

	char *ipbuf = malloc(INET_ADDRSTRLEN);
	struct in_addr *i = (struct in_addr *)ip;

	if (ipbuf == NULL)
		die("malloc");

	if (! inet_ntop(AF_INET, ip, ipbuf, INET_ADDRSTRLEN))
		_e("inet_ntop: could not unpack 0x%08x", i->s_addr);

	return ipbuf;
}


/*
 *   string_to_ip()
 *   ~~~~~~~~~~~~~~
 *   as above. let's avoid dicking around with casts and error checking anywhere
 *   else in the file
 */
in_addr_t string_to_ip(char *str)
{
	_2("string_to_ip(): str=%s", str);

	struct in_addr addr;

	if (! inet_pton(AF_INET, str, &addr))
		_e("inet_pton: could not make string '%s' into an ip", str);

	return addr.s_addr;
}

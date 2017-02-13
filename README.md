# utmpfucc

Automatically remove records based on an IP address from:

 - `/var/run/utmp`
 - `/var/log/wtmp`
 - `/var/log/lastlog`

## Example

    $ whoami
    weepy
    
    $ w
     13:07:41 up 9 days, 19:33,  2 users,  load average: 0.09, 0.39, 0.50
    USER     TTY        LOGIN@   IDLE   JCPU   PCPU WHAT
    root     pts/3     Sun12   22:15m  1.85s  1.85s -zsh
    weepy    pts/4     12:00   58:21   0.16s  0.16s w
    
    $ utmpfucc
    /var/run/utmp: 29342    weepy    pts/4        2017-02-13 12:00 (10.0.0.99)
    [$] /var/run/utmp: removed 1 records
    /var/log/wtmp: 29342    weepy    pts/4        2017-02-13 12:00 (10.0.0.99)
    [$] /var/log/wtmp: removed 1 records
    [$] /var/log/lastlog: removed 1 records
    
    $ w
     13:07:41 up 9 days, 19:33,  1 user,  load average: 0.08, 0.37, 0.50
    USER     TTY        LOGIN@   IDLE   JCPU   PCPU WHAT
    root     pts/3     Sun12   22:15m  1.85s  1.85s -zsh

    $ what now bitch
    sh: what: command not found


## Usage info

    usage: utmpfucc [OPTION]...
    remove entries from utmp/wtmp logs
       -i, --ip=IP         specify ip address to remove
       -n, --dry-run       don't modify log files
       -f, --file          utmp file to edit (default /var/run/utmp and
                           /var/log/wtmp)
           --skip-utmp     skip /var/run/utmp
           --skip-wtmp     skip /var/log/wtmp
           --skip-lastlog  skip /var/log/lastlog
       -v, --verbose       control verbosity (can be given twice)
       -h, --help          display this help and exit
       -V, --version       output version information and exit



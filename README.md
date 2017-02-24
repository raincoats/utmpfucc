# utmpfucc

Automatically remove records based on an IP address from:

 - `/var/run/utmp`
 - `/var/log/wtmp`
 - `/var/log/lastlog`

You must have root, or at least write access on those files, for this tool to work.

## Example

    $ whoami
    root
    
    $ w
     08:50:07 up 10:40,  2 users,  load average: 0.05, 0.13, 0.17
    USER     TTY        LOGIN@   IDLE   JCPU   PCPU WHAT
    root     tty1      08:49   33.00s  0.15s  0.15s -zsh
    root     pts/1     08:48    6.00s  0.33s  0.00s w
    
    $ utmpfucc
    /var/run/utmp: 7854     root     pts/1        2017-02-25 08:48 (10.0.0.99)
    [$] /var/run/utmp: removed 1 records
    /var/log/wtmp: 7854     root     pts/1        2017-02-25 08:48 (10.0.0.99)
    [$] /var/log/wtmp: removed 1 records
    [$] /var/log/lastlog: removed 1 records
    
    $ w
     08:50:22 up 10:40,  1 user,  load average: 0.11, 0.14, 0.18
    USER     TTY        LOGIN@   IDLE   JCPU   PCPU WHAT
    root     tty1      08:49   48.00s  0.15s  0.15s -zsh
    
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



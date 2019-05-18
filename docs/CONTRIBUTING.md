Contributing
============

First of all, thanks for contributing or considering to contribute.  

We have two high level development rules:  

-   Friendship, ideas and code are valued in said order  
-   There are no deadlines  

Issues and bugs
---------------

Reporting issues and bugs is a very helpful contribution. The preferred
route for reporting these is to raise github issues, but we are happy
with any method including forums threads and e-mails. If you are able
to help resolve them, that is also greatly appreciated.

### The basics of reporting issues

It would be useful if you could simply start by describing what you did
and what happened. It may help if you could also do the following:

1.  Run jgmenu from the terminal and copy/paste any output

2.  Provide the output of `jgmenu --version`

3.  Provide the output of `type jgmenu` (i.e. the location of the binary)

4.  Provide the command you used to run jgmenu

5.  Provide the config file

6.  Provide some basic information about your system (e.g. distribution, window
    manager, composite manager)

7.  Run `jgmenu init -i` then choose 'check' and report any warnings

### Debugging

This list is by no means exhaustive, but may provide some ideas for
things to try.

1. Make use of the modular design of jgmenu to isolate the root cause.
   For example, if you are using jgmenu with the 'lx' module, run
   `jgmenu_run lx > foo` and analyse the output. Then try `jgmenu
   --csv-file=foo`.  This approach might help find the root cause of the
   issue.

2. If the menu data is causing the issue (e.g. foo above), try to
   comment out a section at a time or write a script to cycle through
   individual items. The command line option --die-when-loaded can be
   useful for this.

3. If using the 'pmenu' module, `jgmenu_run pmenu` will output the
   .desktop file corresponding to each menu item. This can be useul to
   analyse specific items.

4. strace can be used to understand what what system calls are made.
   For example, to analyse which .desktop files were opened, try:
   `strace -f -o foo jgmenu_run lx ; grep '.desktop' foo`
   If you experience a crash, `strace` can also be particularly useful
   for analysing which system calls were made, including their return
   values.

5. Revert to a default configuration file and gradually change options
   until the bug occurs.

6. The file ./scripts/enable-error-messages.sh contains a number of
   environment variables which can be set for verbose output relating to
   specific topics.

### Stack traces

If jgmenu crashes with the message 'Segmentation fault' or 'Abort', a
stack trace (also known as backtrace) is often the most useful starting
point.  A stack trace tells us where the program stopped and how that
point was reached through all the functions up to main(). A useful stack
trace provides filenames and line numbers.

You can provide a stack trace using one of the methods listed below.

<b>Method 1 - ASAN</b>

AddressSanitizer (ASAN) is a tool that detects memory error bugs and has
been implemented in GCC since v4.8. It is invoked by simply compiling
and linking with `--fsanitize=address` flag.

This method is the easiest if you are prepared to compile jgmenu. You
might balk at this, but jgmenu is very fast and simple to build,
compared to many other software packages.  See INSTALL.md for details of
build dependencies and then take the following steps and report the
output.

    git clone https://github.com/johanmalm/jgmenu
    cd jgmenu
    make ASAN=1
    ./jgmenu

The `ASAN=1` variable enables ASAN and sets some flags for a useful stack
trace. Upon a crash, you will see something like this:

    #0 0x4a1ad2 in build_tree /home/johan/src/jgmenu/jgmenu.c:1099
    #1 0x4aa077 in main /home/johan/src/jgmenu/jgmenu.c:2441
    #2 0xb686d285 in __libc_start_main (/lib/i386-linux-gnu/libc.so.6+0x18285)
    #3 0x499a70  (/home/johan/src/jgmenu/jgmenu+0xba70)

<b>Method 2 - gdb</b>

In order to get a useful stack trace with gdb, you need binaries with
debug symbols. Release packages have generally been 'stripped' of these,
so you have two options:

-   Install a package with debug symbols (e.g. jgmenu-dbgsym on
    BunsenLabs Linux). This option has the advantage of not needing to
    build the binaries.

-   Compile with `-g` and `-Og' flags as a minimum.
    jgmenu CFLAGS are set in `Makefile.inc`

There are two ways of producing a stack trace with gdb. Either run the
programme in gdb and trigger the crash;

    gdb jgmenu
    (gdb) handle SIG64 noprint nostop  # or similar
    (gdb) run
    # wait for crash
    (gdb) backtrace

or analyse a core dump after a crash.

gdb <exec> <core>
(gdb) backtrace

When using the `gdb jgmenu` approach, be aware that jgmenu grabs the
keyboard and pointer, so interaction with gdb is not straight forward.
If you get it wrong, you may need to log into a different tty (e.g.
ctrl+alt+F2) and `killall jgmenu ; killall xterm` (or whatever terminal
you use).

With both approaches, remember that the jgmenu package contains many
binary files, so it's important to run gdb on the one with the issue.
For example, if the 'ob' modules caused a segfault and core dump, you
need to run

    gdb jgmenu-ob <core>

In order to use the `gdb <exec> <core>` method, your system needs to be
configured to core dump and you need the filename and location of the
core dump file. Consult your OS/distribution documentation for details
on this.

The segfault/abort error message will indicate if the core has been
dumped, e.g: `Segmentation fault (core dumped)`.

If you system has not been setup to core dump, you can use the command
`ulimit -c unlimited` to force a core dump in a specific shell session.
In this scenario, the core will be dumped in the current working
directory.

<b>Method 3 - coredumpctl</b>

If your system is set up to core dump and you have coredumpctl installed,
you can run the following and include the output in your bug report.

   sudo coredumpctl info jgmenu

Upversion
---------

-   update `default_version` in scripts/version-gen.sh  
-   update debian/changelog  
-   create docs/relnotes/X.Y.txt  
-   update NEWS.md  
-   add and commit the above files  
-   git tag -a 'vX.Y' (using the release notes as the commit message)  

Add new config variables
------------------------

Any new config variables need to be added to the following:  

-   config.c  
-   config.h  
-   src/jgmenu-config.c  
-   docs/manual/jgmenu.1.md  

Run ./scripts/jgmenurc-checker.sh

Testing
-------

`make ex` runs a few menus specifically designed to test the code.  

`make check` checks coding style.  
To just check a specific .c or .h file, do  
`./scripts/checkpatch-wrapper foo.c`  

`make test` runs unit tests  

APIs
----

list.h  
https://kernelnewbies.org/FAQ/LinkedLists  

hashmap.c  
https://www.kernel.org/pub/software/scm/git/docs/technical/api-hashmap.html  

Architecture
------------

`jgmenu_run` is a wrapper which call jgmenu-\<*command*> where `command`
is the first argument provided to `jgmenu_run`.  

`jgmenu_run` makes jgmenu easier to use by  

-   creating a simple interface for common commands  
-   saving the user having to remember script file extensions  
-   ensures the correct modules are called if multiple version of jgmenu are
    installed on the system.  

It also helps keep $prefix/bin tidier by putting all the other scripts in
$libexecdir.

Although it is recommended to do a `make install`, it is possible to run
`./jgmenu_run` from the source directory by setting `JGMENU_EXEC_PATH`.  

Grammar and Language
--------------------

jgmenu is always written with a lowercase "j". It should be obvious from
the context if we refer to the entire application or just the binary
file.  

The language used in documentation shall follow British English rules.
Although, for variable names and configuration options, US spelling is
used (e.g. color and center)  

Git
---

Keep commit messages lines to 74 characters.  

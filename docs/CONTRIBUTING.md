Contributing
============

First of all, thanks for contributing or considering to contribute.  

We have two high level development rules:  

  - Friendship, ideas and code are valued in said order  
  - There are no deadlines  

Upversion
---------

  - update `default_version` in scripts/version-gen.sh  
  - update debian/changelog  
  - create docs/relnotes/X.Y.txt  
  - git tag -a 'vX.Y'  

Add new config variables
---------------------------

Any new config variables need to be added to the following:  

  - config.c  
  - config.h  
  - noncore/config/jgmenurc  
  - docs/manual/jgmenu.1.md  

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

  - creating a simple interface for common commands  
  - saving the user having to remember script file extensions  
  - ensures the correct modules are called if multiple version of jgmenu  
    are installed on the system.  

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


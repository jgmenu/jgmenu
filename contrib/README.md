contrib/ software
=================

The tools and helpers in this directory:

- are stand-alone, hopefully interesting and sometimes experimental;

- are included in the git-repo to give users easier access to them;

- have sometimes been entirely contributed by others;

- are not installed by default, but can optionally be added to
  `make && make install` (those that are considered mature enough, can be
  given as options to the configure script - e.g. --with-lx)

- may have completely different dependencies to the core jgmenu application
  (e.g. ff-bookmarks depends on sqlite3 and arguably also firefox/iceweasel);

- do not always adhere to the jgmenu coding guidelines.

Although I will try to maintain this code, my main focus will remain on
the jgmenu app itself (the code in the src/ directory).

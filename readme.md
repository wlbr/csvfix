CSVfix
------

This is the source for CSVfix and the supporting alib library.
Building requires a GCC installation.  To build, enter the command 
"make win" for a Windows build, "make lin" for a Linux/Unix build, 
or "make mac" for a Mac build.

For further information, please see the CSVfix website 
at http://neilb.bitbucket.org/csvfix/

Neil Butterworth
13-Nov-2014


-----

The wonderful CSVfix tool was written by Neil Butterworth, but unfortunately it seems to be _orphaned_. There 
is an archived [repository on Google Code](https://code.google.com/archive/p/csvfix/) (version 1.5). Its readme 
points to a [version 1.6 on Bitbucket](http://neilb.bitbucket.org/csvfix), but that link is dead. Neil does not 
seem to be on Github.

I've seen a number of projects on [Github](https://github.com/search?q=csvfix&type=repositories) that autoimported
the code from the Google Code repo, but none of them compiles correctly on Linux or Mac. Finally I found a copy of 
the mentioned [1.6 on Sourceforce](https://sourceforge.net/projects/csvfix/) that was brought 
there by [Pedro Albanese](https://sourceforge.net/u/pedroalbanese/profile/). That one works!

UPDATE : A zip of version 1.7 was discovered on a backup drive by [@the-reverend](https://github.com/the-reverend) and it also builds!

I tried to install CSVfix using Homebrew on the Mac but that fails as well, as it tries to use the dead Bitbucket repo.

So the idea of this repository here is:
   * provide a working copy of 1.7 on Github (done, more or less a copy of [@the-reverend](https://github.com/the-reverend)'s backup)
   * provide a release using Github Actions (done)
   * point Homebrew to the new location (-not accepted by Homebrew-)

The docs are available in the `docs` subdirectory, that is being hosted as Github project pages at https://wlbr.de/csvfix
   
Michael Wolber


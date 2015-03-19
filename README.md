# Joe's Own Editor

[Editing Tasks](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/tasks.md) 

[Release Notes](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/NEWS.md)

[List of Commands](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/list.md)

[List of Options](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/options.md)

[Hacking](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/hacking.md)

[Man page](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/man.md)

[Project page](http://www.sourceforge.net/projects/joe-editor)

[Download source](http://prdownloads.sourceforge.net/joe-editor/joe-3.7.tar.gz?download)

[History](https://sourceforge.net/p/joe-editor/mercurial/ci/default/tree/docs/history.md)

<p></p>

<p>JOE is a full featured terminal-based screen editor which is distributed
under the GNU General Public License (GPL).  JOE has been around since 1988
and comes standard with many Linux distributions.</p>

<p>JOE is being maintained by its original author Joseph Allen, plus all of
the people who send bug reports, feature suggestions and patches to the
project web site.  JOE is hosted by SourceForge.net and its source code is
controlled under CVS.  Over the last few years there has been about one
major new release a year, usually in the April-May timeframe.</p>

<p>JOE is a blending of MicroPro's venerable microcomputer word processor
WordStar and Richard Stallman's famous LISP based text editor GNU-EMACS (but
it does not use code from either program): most of the basic editing keys
are the same as in WordStar as is the overall feel of the editor.  JOE also
has some of the key bindings and many of the powerful features of EMACS.</p>

<p>JOE is written in C and its only dependency is libc.  This makes JOE very
easy to build (just "configure" and "make install"), making it feasible to
include on small systems and recovery disks.  The compiled binary is about
300K in x86.  Note that JOE can use either the termcap or terminfo terminal
capabilities databases (or a built-in termcap entry for ANSI terminals).  The
choice is controlled by a "configure" option.  If terminfo is used, a
library is required to access the database (on some systems this library is
ncurses, but JOE does not use curses to control the terminal- it has its own
code for this).</p>

<p>Much of the look and feel of JOE is determined by its simple
configuration file "joerc".  Several variants of the editor are installed by
default in addition to "joe": "jmacs" (emulate GNU-EMACS), "jstar" emulate
WordStar, "jpico" emulate the Pine mailer editor PICO and "rjoe"- a
restricted version of JOE which allows the used to only edit the file given
on the command line.  JOE is linked to several names.  The name which is
used to invoke the editor with "rc" tacked on the end gives the name of
configuration file to use.  It is thus easy for you to make your own variant
if you want.  Also you can customize the editor by copying the system
"joerc" file to your home directory.</p>

<p>Here is a basic screen shot of JOE running in a Cygwin console:</p>
<img src="capture.gif" alt="screen capture">

<p>Here is a screen shot showing several windows- the first has some example
double-wide characters, the second is the same buffer as the first, but in
hex-dump view mode, the third is a shell window and the fourth shows a
selected rectangular block of numbers and their sum:</p>
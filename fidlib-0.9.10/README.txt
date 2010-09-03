Fidlib and Firun
----------------

This archive contains 'fidlib', my run-time filter design and
execution library, and 'firun', a tool for running fidlib filters from
the command-line.

  'Fidlib' is Copyright (c) 2002-2004 Jim Peters <http://uazu.net/>
  and is released under the GNU Lesser General Public License (LGPL)
  version 2.1 as published by the Free Software Foundation.  See the
  file COPYING_LIB for details, or visit: http://www.fsf.org/licenses/

  'Firun' is Copyright (c) 2004 Jim Peters <http://uazu.net/>
  and is released under the GNU General Public License (GPL)
  version 2 as published by the Free Software Foundation.  See the
  file COPYING for details, or visit: http://www.fsf.org/licenses/

To use 'fidlib' (one way at least):

  Copy fidlib.h, fidlib.c, fidmkf.h and fidrf_cmdlist.h to your
    project directory
  Build fidlib.c as part of your project
  #include "fidlib.h" in your source for prototypes and structures
  See fidlib.txt for full docs and API
  See also fidlib.c and fidlib.h for additional notes

To use 'firun':

  Type "firun" or "./firun" for an overview
  See firun.txt for full docs, and fidlib.txt for filter types
  
To build 'firun' (if included executables don't work):

  On Linux, run 'mk-firun' to build it
  On Windows, try 'mk-firun-mingw' using the MinGW MSYS environment

Jim

--
 Jim Peters                  (_)/=\~/_(_)                 jim@uazu.net
                          (_)  /=\  ~/_  (_)
 Uazú                  (_)    /=\    ~/_    (_)                http://
 Brighton, UK       (_) ____ /=\ ____ ~/_ ____ (_)            uazu.net

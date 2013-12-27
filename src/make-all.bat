cd Matheval
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS error.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS matheval.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS parser.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS symbol_table.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS xmath.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS g77_interface.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS node.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS scanner.c
gcc -c -mwindows -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS xmalloc.c
copy *.o ..
del *.o
cd ..
make


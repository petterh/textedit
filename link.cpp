/*
 * $Header: /Book/link.cpp 5     20.08.99 16:33 Oslph312 $
 *
 * Contains linker comments to spare us a long linker command line.
 * NOTE: If your compiler/linker doesn't support these pragmas, 
 * you should specify these libraries on the linker command line.
 */

#include "precomp.h"


#ifndef UNITTEST
#ifdef UNICODE
#pragma comment( linker, "/entry:wWinMainCRTStartup" )
#else
#pragma comment( linker, "/entry:WinMainCRTStartup" )
#endif
#endif // UNITTEST


#pragma comment( lib, "kernel32.lib" )
#pragma comment( lib, "gdi32.lib"    )
#pragma comment( lib, "user32.lib"   )
#pragma comment( lib, "shell32.lib"  )
#pragma comment( lib, "comctl32.lib" )
#pragma comment( lib, "comdlg32.lib" )
#pragma comment( lib, "shlwapi.lib"  )
#pragma comment( lib, "version.lib"  )
#pragma comment( lib, "advapi32.lib" )
#pragma comment( lib, "ole32.lib"    )

// end of file

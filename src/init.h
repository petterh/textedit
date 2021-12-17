/*
 * $Header: /Book/init.h 8     3.07.99 13:31 Oslph312 $
 *
 * Interface to the main TextEdit initialization function, 
 * defined in init.cpp.
 */

#pragma once

#include "String.h"

class Editor;
Editor *init( LPCTSTR pszCmdLine, int nShow );

// end of file

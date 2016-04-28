/*
 * $Header: /Book/precomp.h 23    16.07.04 10:42 Oslph312 $
 * 
 * General header file, used for precompiled headers.
 * See also common.h.
 */

#pragma once

#define USE_OWN_EXCEPTIONS

#ifndef WINVER
#define WINVER _WIN32_WINNT_WIN7
#endif

#ifndef _WIN32_IE
#define _WIN32_IE _WIN32_IE_IE90
#endif

//#define _WIN32_WINDOWS 0x0400
#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN7 // required for mouse wheel definitions
#endif

#include "unicode.h"

#define STRICT
#define OEMRESOURCE
#define NOMINMAX
#define NOCRYPT

#include "warnings.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wininet.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <process.h>
#include <direct.h>

// The STL header files generate scads of warnings:

#ifdef _MSC_VER
   #if 1200 <= _MSC_VER
      #pragma warning( push, 3 )
   #else
      #pragma warning( disable: 4018 4146 4244 4245 4663 )
   #endif
#endif

#include <string>
#include <memory>
#include <map>
#include <algorithm>

#ifdef _MSC_VER
   #if 1200 <= _MSC_VER
      #pragma warning( pop )
   #else
      #pragma warning( default: 4018 4146 4244 4245 4663 )
   #endif
#endif

#ifdef _MSC_VER
   #pragma warning( disable: 4711 ) // function 'xxx' selected for automatic inline expansion
#endif

#include "common.h" // TODO -- this is questionable

// end of file

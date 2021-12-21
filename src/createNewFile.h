/*
 * $Header: /Book/createNewFile.h 9     6-09-01 12:40 Oslph312 $
 * TODO: EmptyString?
 */

#pragma once

#include "String.h"

enum create_new_file_args {
   brand_new_file,
   file_for_stdin,
   file_for_copy,
};

String getDefaultPath( void );
String createNewFile( 
   HWND hwndParent = HWND_DESKTOP, 
   create_new_file_args what_kind = brand_new_file, 
   HANDLE hIn = 0,
   const String& path_for_new_file  = _T( "" ),
   const String& name_of_org_file = _T( "" ) );

// end of file

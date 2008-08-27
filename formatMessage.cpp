/*
 * $Header: /Book/formatMessage.cpp 11    5.09.99 13:07 Oslph312 $
 * 
 * Defines the formatMessage functions and their 
 * companion, the formatMessageV function.
 */

#include "precomp.h"
#include "formatMessage.h"
#include "utils.h"


#define MAX_LENGTH 1000


/**
 * Formats a message using the FormatMessage function, 
 * with a formatting string. Note that the formatting
 * string contains %1, %2, ... rather than printf-style
 * format specifiers. You can still specify a printf-style
 * format by enclosing it in a pair of exclamation marks
 * and putting it behind the number, e.g. %1!d! for a digit.
 * The default is !s!, signifying a string.
 */
String __cdecl formatMessage( const String strFmt, ... ) {
   
   va_list vl;
   va_start( vl, strFmt );
   const String strMessage = formatMessageV( strFmt, vl );
   va_end( vl );

   return strMessage;
}


/**
 * Formats a message using the FormatMessage function, 
 * with a formatting string loaded from the string table.
 * See the other formatMessage for further comments.
 */
String __cdecl formatMessage( UINT uiFmtID, ... ) {

   const String strFmt = loadString( uiFmtID );
   va_list vl;
   va_start( vl, uiFmtID );
   const String strMessage = formatMessageV( strFmt, vl );
   va_end( vl );

   return strMessage;
}


/**
 * Formats a message using the FormatMessage function, 
 * with a formatting string. This is the real workhorse,
 * used by both formatMessage functions.
 */
String formatMessageV( const String& strFmt, va_list vl ) {

   // This buffer *must* be empty in case strFmt is empty!
   TCHAR sz[ MAX_LENGTH ] = { 0 }; 
   FormatMessage( FORMAT_MESSAGE_FROM_STRING, 
      strFmt.c_str(), 0, 0, sz, dim( sz ), &vl );
   return sz;
}

// end of file

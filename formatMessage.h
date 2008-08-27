/*
 * $Header: /Book/formatMessage.h 8     5.09.99 13:07 Oslph312 $
 */

#pragma once

#include "String.h"

String __cdecl formatMessage( const String strFmt, ... );
String __cdecl formatMessage( UINT uiFmtID, ... );
String formatMessageV( const String& strFmt, va_list vl );

// end of file
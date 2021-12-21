/*
 * $Header: /Book/EditWordBreakProc.cpp 1     16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include <richedit.h>
#include "assert.h"
#include "trace.h"
#include "EditWordBreakProc.h"
#include "CharClasses.h"


inline int getClass( int ch ) {
	return ch < dim( CharClasses ) ? CharClasses[ ch ] : 0;
}


static int isDelimiter( int ch ) {
	return 1 == (WBF_CLASS & getClass( ch ) );
}


#if 0
static int moveWordLeft( LPSTR contents, int index, int length ) {

	if ( 0 < index ) {
		--index;
		while ( 0 < index && (WBF_ISWHITE & getClass( contents[ index ] ) ) ) {
			--index;
		}
		const int currClass = getClass( contents[ index ] );
		while ( 0 < index && currClass == getClass( contents[ index - 1 ] ) ) {
			--index;
		}
	}
	return index;
}


static int moveWordRight( LPSTR contents, int index, int length ) {

	if ( index < length ) {
		const int currClass = getClass( contents[ index ] );
		while ( index < length && currClass == getClass( contents[ index ] ) ) {
			++index;
		}
		while ( index < length && (WBF_ISWHITE & getClass( contents[ index ] ) ) ) {
			++index;
		}
	}
	return index;
}
#endif


static int moveWordLeft( LPWSTR contents, int index, int length ) {

	if ( 0 < index ) {
		--index;
		while ( 0 < index && (WBF_ISWHITE & getClass( contents[ index ] ) ) ) {
			--index;
		}
		const int currClass = getClass( contents[ index ] );
		while ( 0 < index && currClass == getClass( contents[ index - 1 ] ) ) {
			--index;
		}
	}
	return index;
}


static int moveWordRight( LPWSTR contents, int index, int length ) {

	if ( index < length ) {
		const int currClass = getClass( contents[ index ] );
		while ( index < length && currClass == getClass( contents[ index ] ) ) {
			++index;
		}
		while ( index < length && (WBF_ISWHITE & getClass( contents[ index ] ) ) ) {
			++index;
		}
	}
	return index;
}


#if 0
static int CALLBACK EditWordBreakProcA(
  LPSTR contents, // text string
  int index  , // index of starting point
  int length , // length of text string
  int code   ) // action to take
{
	int result = 0;
	switch ( code ) {
	case WB_LEFT       :
		trace( _T( "WB_LEFT" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_RIGHT      :
		trace( _T( "WB_RIGHT" ) );
		result = moveWordRight( contents, index, length );
		break;
	case WB_ISDELIMITER:
		trace( _T( "WB_ISDELIMITER" ) );
		result = isDelimiter( contents[ index ] );
		break;
	case WB_CLASSIFY:
		trace( _T( "WB_CLASSIFY" ) );
		result = getClass( contents[ index ] );
		break;
	case WB_MOVEWORDLEFT:
		trace( _T( "WB_MOVEWORDLEFT" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_MOVEWORDRIGHT:
		trace( _T( "WB_MOVEWORDRIGHT" ) );
		result = moveWordRight( contents, index, length );
		break;
	case WB_LEFTBREAK:
		trace( _T( "WB_LEFTBREAK" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_RIGHTBREAK:
		trace( _T( "WB_RIGHTBREAK" ) );
		result = moveWordRight( contents, index, length );
		break;
	}
	trace( _T( " result(%c, %d) = %d (0x%x)\n" ), 
		contents[ index ], index, result, result );

	return result;
}
#endif


static int CALLBACK EditWordBreakProcW(
  LPWSTR contents, // text string
  int index  , // index of starting point
  int length , // length of text string
  int code   ) // action to take
{
	int result = 0;
	switch ( code ) {
	case WB_LEFT       :
		trace( _T( "WB_LEFT" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_RIGHT      :
		trace( _T( "WB_RIGHT" ) );
		result = moveWordRight( contents, index, length );
		break;
	case WB_ISDELIMITER:
		trace( _T( "WB_ISDELIMITER" ) );
		result = isDelimiter( contents[ index ] );
		break;
	case WB_CLASSIFY:
		trace( _T( "WB_CLASSIFY" ) );
		result = getClass( contents[ index ] );
		break;
	case WB_MOVEWORDLEFT:
		trace( _T( "WB_MOVEWORDLEFT" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_MOVEWORDRIGHT:
		trace( _T( "WB_MOVEWORDRIGHT" ) );
		result = moveWordRight( contents, index, length );
		break;
	case WB_LEFTBREAK:
		trace( _T( "WB_LEFTBREAK" ) );
		result = moveWordLeft( contents, index, length );
		break;
	case WB_RIGHTBREAK:
		trace( _T( "WB_RIGHTBREAK" ) );
		result = moveWordRight( contents, index, length );
		break;
	}
	trace( _T( " result(%c, %d) = %d (0x%x)\n" ), 
		contents[ index ], index, result, result );

	return result;
}


void installEditWordBreakProcA( HWND hwnd ) {
	// TODO: Even the edit proc gets a Unicode string here.
	// May have something to do with the version?
	SendMessage( hwnd, EM_SETWORDBREAKPROC, 0,
		reinterpret_cast< LRESULT >( EditWordBreakProcW ) );
}


void installEditWordBreakProcW( HWND hwnd ) {
	SendMessage( hwnd, EM_SETWORDBREAKPROC, 0, 
		reinterpret_cast< LRESULT >( EditWordBreakProcW ) );
}

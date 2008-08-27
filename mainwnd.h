/*
 * $Header: /Book/mainwnd.h 10    6-09-01 12:49 Oslph312 $
 */

#pragma once

#include "Exception.h"
#include "Editor.h"

extern LRESULT CALLBACK mainWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define EDITOR_OFFSET 0
#define MAINWND_EXTRABYTES sizeof( Editor * )


/**
 * This function returns a null pointer during startup.
 * When called after setup, the hwnd is null, too,
 * but GetWindowLong fails gracefully in that case.
 */
inline Editor *getEditor( HWND hwnd ) {
   Editor *pEditor = reinterpret_cast< Editor * >( 
      GetWindowLong( hwnd, EDITOR_OFFSET ) );
   assert( 0 == pEditor || isGoodPtr( pEditor ) );
   if ( 0 == pEditor ) {
      trace( _T( "getEditor throws NullPointerException\n" ) );
      throw NullPointerException();
   }
   return pEditor;
}

// end of file

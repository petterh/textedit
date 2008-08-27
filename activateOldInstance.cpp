/*
 * $Header: /Book/activateOldInstance.cpp 17    16.07.04 10:42 Oslph312 $
 *
 * TODO:
 * activateOldInstance has one weakness -- it will only catch
 * "real" instances, which have an editing window open. If a
 * previous instance was started with the /p switch, we won't
 * catch it here.
 */

#include "precomp.h"
#include "activateOldInstance.h"
#include "main_class.h"
#include "winUtils.h"
#include "addAtom.h"
#include "resource.h"


/**
 * This struct is used to pass information to the enumProc below.
 * We use an ATOM rather than a string to identify the document,
 * as strings can't be sent across Win32 process boundaries.
 * Discussion: WM_COPYDATA?
 */
struct EnumStruct {
   HWND hwnd;
   ATOM aDocName;
};


/**
 * Callback function for EnumWindows.
 */
PRIVATE BOOL CALLBACK enumProc( HWND hwnd, LPARAM lParam ) {

   if ( isClass( hwnd, MAIN_CLASS ) ) {
      EnumStruct *pEnumStruct =
         reinterpret_cast< EnumStruct * >( lParam );
      DWORD dwResult = 0;
      const LRESULT lResult = SendMessageTimeout(
         hwnd, WM_APP, 0, pEnumStruct->aDocName,
         SMTO_ABORTIFHUNG, 3000, &dwResult );
      if ( 0 != lResult && 0 != dwResult ) {
         pEnumStruct->hwnd = hwnd;
         return FALSE; // Stop enumerating windows.
      }
   }
   return TRUE;        // Continue enumerating windows.
}


/**
 * Loop over all existing TextEdit instances 
 * and ask if they have this document:
 */
bool activateOldInstance( LPCTSTR pszPath, bool bPrinting ) {

	assert( 0 != pszPath );
	PATHNAME szPath = { 0 };
	SetLastError( NOERROR );
	const DWORD length = GetShortPathName( pszPath, szPath, dim( szPath ) );
	const DWORD win_error = GetLastError();
	if ( ERROR_FILE_NOT_FOUND == win_error ) {
		return false; //*** FUNCTION EXIT POINT
	}

	if ( NOERROR != win_error && ERROR_INVALID_NAME != win_error ) {
		debugBreak();
	}

	// TODO: New function getShortPathName with retry for ERROR_NOT_READY.
	if ( 0 == length || dim( szPath ) < length ) {
		return false; //*** FUNCTION EXIT POINT
	}
	assert( length == _tcsclen( szPath ) );
	ATOM aDocName = globalAddAtom( szPath );
	if ( 0 != aDocName ) {
		EnumStruct enumStruct = { 0, aDocName };
		EnumWindows( enumProc, reinterpret_cast< LPARAM >( &enumStruct ) );
		verify( 0 == GlobalDeleteAtom( aDocName ) );
		if ( IsWindow( enumStruct.hwnd ) ) {
			if ( bPrinting ) {
				FORWARD_WM_COMMAND( // TODO -- what if the win is minimized?
					enumStruct.hwnd, ID_FILE_PRINT, 0, 0, PostMessage );
			} else if ( IsIconic( enumStruct.hwnd ) ) {
				FLASHWINFO flash_info = { sizeof flash_info, enumStruct.hwnd, FLASHW_ALL, 3, 0 };
				FlashWindowEx( &flash_info );
			} else {
				HWND hwndToActivate = GetLastActivePopup( enumStruct.hwnd );
				verify( SetForegroundWindow( hwndToActivate ) );
			}
			return true; //*** FUNCTION EXIT POINT
		}
	}

	return false;        //*** FUNCTION EXIT POINT
}

// end of file

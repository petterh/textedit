/*
 * $Header: /Book/openDlgCommon.cpp 9     16.07.04 10:42 Oslph312 $
 *
 * Common stuff for all Open/Save dialogs:
 *
 * - Handles system color and font changes for the list view
 * - Sets Arial font in file name edit control 
 *   if FIX_EDIT_FONT is defined.
 */

#include "precomp.h"
#include "Exception.h"
#include "InstanceSubclasser.h"
#include "fileUtils.h"
#include "winUtils.h"
#include "utils.h"
#include "persistence.h"
#include "resource.h"

#define FIX_EDIT_FONT (defined( _UNICODE ) && 0)

#if FIX_EDIT_FONT
PRIVATE HFONT s_hfont = 0;
PRIVATE HFONT s_hfontSaved = 0;
#endif


#ifdef _DEBUG
PRIVATE int s_nRefCount = 0; // Assuming only one at a time
#endif


PRIVATE LRESULT CALLBACK commonDlgSubclassing( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

PRIVATE InstanceSubclasser 
   s_commonDlgSubclasser( commonDlgSubclassing );


PRIVATE inline void onSysColorChange( HWND hwnd ) {
   
   HWND hwndList = GetDlgItem( hwnd, lst2 );
   assert( IsWindow( hwndList ) );
   FORWARD_WM_SYSCOLORCHANGE( hwndList, SNDMSG );
}


PRIVATE inline void onSettingChange( HWND hwnd, LPCTSTR pszSection ) {

   HWND hwndList = GetDlgItem( hwnd, lst2 );
   assert( IsWindow( hwndList ) );
   FORWARD_WM_WININICHANGE( hwndList, 0, SNDMSG );
}


PRIVATE inline void onDestroy( HWND hwnd ) {
   
   assert( IsWindow( hwnd ) );
   UINT id = reinterpret_cast< UINT >( 
      s_commonDlgSubclasser.getUserData( hwnd ) );
   savePosition( hwnd, id );

#if FIX_EDIT_FONT
   
   HWND hwndEdit = GetDlgItem( hwnd, edt1 );
   assert( IsWindow( hwndEdit ) );
   if ( IsWindow( hwndEdit ) ) {
      if ( 0 != s_hfontSaved ) {
         SetWindowFont( hwndEdit, s_hfontSaved, false );
         if ( 0 != s_hfont ) {
            DeleteFont( s_hfont );
         }
      }
   }

#endif // FIX_EDIT_FONT

   assert( 0 == --s_nRefCount );
}


/**
 * This function exists merely to wrap the 
 * return statement in the HANDLE_MSG macro.
 */
PRIVATE LRESULT CALLBACK _commonDlgSubclassing( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   assert( IsWindow( hwnd ) );
   switch ( msg ) {
   HANDLE_MSG( hwnd, WM_SYSCOLORCHANGE, onSysColorChange );
   HANDLE_MSG( hwnd, WM_SETTINGCHANGE , onSettingChange  );
   HANDLE_MSG( hwnd, WM_DESTROY       , onDestroy        );
   }
   return 0;
}


PRIVATE LRESULT CALLBACK commonDlgSubclassing( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   assert( IsWindow( hwnd ) );
   _commonDlgSubclassing( hwnd, msg, wParam, lParam );
   return s_commonDlgSubclasser.callOldProc( 
      hwnd, msg, wParam, lParam );
}


/**
 * Call this on CDN_INITDONE.
 */
void subclassOpenDlgCommon( HWND hwndChildDlg, UINT id ) {

   assert( 1 == ++s_nRefCount );

   HWND hwndOpenDlg = GetParent( hwndChildDlg );
   assert( IsWindow( hwndOpenDlg ) );
   restorePosition( hwndOpenDlg, id );
   SNDMSG( hwndChildDlg, DM_SETDEFID, IDOK, 0 );

#if FIX_EDIT_FONT
   
   s_hfont = s_hfontSaved = 0;
   HWND hwndEdit = GetDlgItem( hwndOpenDlg, edt1 );
   assert( IsWindow( hwndEdit ) );
   if ( IsWindow( hwndEdit ) ) {
      s_hfontSaved = GetWindowFont( hwndEdit );
      if ( 0 != s_hfontSaved ) {
         LOGFONT logFont = { 0 };
         verify( GetObject( 
            s_hfontSaved, sizeof logFont, &logFont ) );
         _tcscpy( logFont.lfFaceName, _T( "Arial" ) );
         s_hfont = CreateFontIndirect( &logFont );
         if ( 0 != s_hfont ) {
            SetWindowFont( hwndEdit, s_hfont, false );
         }
      }
   }

#endif // FIX_EDIT_FONT

   // If this subclassing fails, we can live with the consequences:
   try {
      verify( s_commonDlgSubclasser.subclass( 
         hwndOpenDlg, reinterpret_cast< void * >( id ) ) );
   }
   catch ( const SubclassException& x ) {
      trace( _T( "%s\n" ), x.what() );
   }
}

PRIVATE LPCTSTR getFilterList( const bool save ) {
   
   const size_t MAX_FILTER_LENGTH = 900;
   static TCHAR szFilters[ 2 ][ MAX_FILTER_LENGTH ] = { 0 };
   if ( 0 != szFilters[ save ][ 0 ] ) {
      return szFilters[ save ]; //** FUNCTION EXIT POINT
   }

   LPTSTR filterStart = szFilters[ save ];
   LPTSTR pszPtr = filterStart;
   String strFilters = loadString( save ? IDS_FILEFILTERS2 : IDS_FILEFILTERS );

   for ( ;; ) {
      int iBreak = strFilters.find( _T( '|' ) );
      if ( iBreak <= 0 ) {
         break; //*** LOOP EXIT POINT
      }
      String strDescription = strFilters.substr( 0, iBreak );
      strFilters.erase( 0, iBreak + 1 );

      iBreak = strFilters.find( _T( '|' ) );
      assert( 0 < iBreak );
      if ( iBreak <= 0 ) {
         break; //*** LOOP EXIT POINT
      }
      String strExtensions = strFilters.substr( 0, iBreak );
      strFilters.erase( 0, iBreak + 1 );
      
      // Check if we would overflow the szFilters buffer.
      // If we would, just cut off at this point.
      const int nNewLength = 
         strDescription.length() + 2 * strExtensions.length() + 6;
      if ( szFilters[ save ] + MAX_FILTER_LENGTH <= pszPtr + nNewLength ) {
         trace( _T( "Filter string too long adding [%s]\n" ), strDescription.c_str() );
         break; //*** LOOP EXIT POINT
      }

      // We're home safe; append the filter:
      wsprintf( pszPtr, _T( "%s (%s)" ), strDescription.c_str(), strExtensions.c_str() );
      pszPtr += _tcsclen( pszPtr ) + 1;
	  const size_t charactersLeft = filterStart + MAX_FILTER_LENGTH - pszPtr;
      _tcscpy_s( pszPtr, charactersLeft, strExtensions.c_str() );
      pszPtr += _tcsclen( pszPtr ) + 1;
   }

   assert( pszPtr < szFilters[ save ] + MAX_FILTER_LENGTH );
   *pszPtr = 0;
   trace( _T( "Filter length = %d\n" ), pszPtr - szFilters[ save ] );
   return szFilters[ save ];
}

PRIVATE bool getOpenOrSaveFileName( 
   HWND hwndParent, UINT uiTitleString, LPOFNHOOKPROC fnHook,
   LPTSTR pszFileName, UINT cch,
   UINT uiChildDlg, DWORD dwFlags, bool bSave ) 
{
   assert( isGoodStringPtr( pszFileName ) );

   String strCustomFilter = getCustomFilter();
   TCHAR szCustomFilter[ MAX_PATH ] = { 0 };
   if ( !strCustomFilter.empty() ) {
      LPTSTR pszPtr = szCustomFilter;
      const String strFmt = loadString( IDS_CUSTOM );
      wsprintf( pszPtr, strFmt.c_str(), strCustomFilter.c_str() );
      pszPtr += _tcsclen( pszPtr ) + 1;
	  const int availableLength = szCustomFilter + dim( szCustomFilter ) - pszPtr;
      _tcscpy_s( pszPtr, availableLength, strCustomFilter.c_str() );
   }

   // TODO: Really ought to figure this out based on current file.
   int nFilterIndex = getFilterIndex();
   if ( nFilterIndex < 0 ) {
	   nFilterIndex = strCustomFilter.empty() ? 0 : 1;
   }

   PATHNAME szCurrPath = { 0 };
   verify( 0 != _tgetdcwd( 0, szCurrPath, dim( szCurrPath ) ) );

   const String strTitle = loadString( uiTitleString );
   OPENFILENAME openFileName = {
      sizeof( OPENFILENAME ),
      hwndParent,
      getModuleHandle(),
      getFilterList( bSave ),
      szCustomFilter,
      dim( szCustomFilter ),
      (DWORD) nFilterIndex,
      pszFileName,
      cch,
      0,
      0,
      szCurrPath,
      strTitle.c_str(),
      OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY | OFN_NOCHANGEDIR,
      0,
      0,
      getDefaultExtension() + 1, // Don't want dot...
      0,
      fnHook,
      MAKEINTRESOURCE( uiChildDlg ),
   };
   openFileName.Flags |= dwFlags;
   if ( 0 != fnHook ) {
      openFileName.Flags |= OFN_ENABLEHOOK;
   }
#if defined( _DEBUG ) && defined( OFN_ENABLESIZING )
   openFileName.Flags |= OFN_ENABLESIZING;
#endif

   if ( HWND_DESKTOP != hwndParent ) {
      FORWARD_WM_COMMAND( 
         hwndParent, ID_COMMAND_RESETSTATUSBAR, 0, 0, SNDMSG );
   }

   const BOOL bOK = bSave ? GetSaveFileName( &openFileName )
                          : GetOpenFileName( &openFileName );
   if ( bOK ) {
      LPTSTR pszNewCustomFilter = 
         szCustomFilter + _tcsclen( szCustomFilter ) + 1;
      if ( 0 != *pszNewCustomFilter ) {
         openFileName.nFilterIndex = 0;
         LPCTSTR pszPtr = getFilterList( bSave );
         for ( int iFilter = 0; ; ++iFilter ) {
            pszPtr += _tcsclen( pszPtr ) + 1;
            if ( 0 == *pszPtr ) {
               break; //*** LOOP EXIT POINT
            }
			// TODO -- each filter can have multiple parts, no?
            if ( 0 == _tcsicmp( pszNewCustomFilter, pszPtr ) ) {
               openFileName.nFilterIndex = iFilter + 1;
               *pszNewCustomFilter = 0;
               break; //*** LOOP EXIT POINT
            }
            pszPtr += _tcsclen( pszPtr ) + 1;
         }
      }
      setFilterIndex( openFileName.nFilterIndex );
      setCustomFilter( pszNewCustomFilter );
   } else {

      // Debugging tip:
      // This illustrates one way of forcing an exception (for testing):
#if 0
      const DWORD dwErr = CDERR_FINDRESFAILURE; 
#else
      const DWORD dwErr = CommDlgExtendedError();
#endif

      if ( ERROR_SUCCESS == dwErr ) {
         ; // The user hit cancel; OK.
      } else {
         throw CommonDialogException( dwErr );
      }
   }

   return 0 != bOK;
}


bool getOpenFileName( HWND hwndParent, UINT uiTitleString,
   LPOFNHOOKPROC fnHook, LPTSTR pszFileName, UINT cch, 
   UINT uiChildDlg, DWORD dwFlags )
{
   return getOpenOrSaveFileName( hwndParent, uiTitleString,
      fnHook, pszFileName, cch, uiChildDlg, dwFlags, false );
}


bool getSaveFileName(  HWND hwndParent, UINT uiTitleString,
   LPOFNHOOKPROC fnHook, LPTSTR pszFileName, UINT cch, 
   UINT uiChildDlg, DWORD dwFlags )
{
   return getOpenOrSaveFileName( hwndParent, uiTitleString,
      fnHook, pszFileName, cch, uiChildDlg, dwFlags, true );
}

// end of file

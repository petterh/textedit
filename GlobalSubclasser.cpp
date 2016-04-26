/*
 * $Header: /Book/GlobalSubclasser.cpp 7     16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "GlobalSubclasser.h"
#include "Exception.h"
#include "addAtom.h"
#include "winUtils.h"
#include "utils.h"
#include "trace.h"


GlobalSubclasser::GlobalSubclasser( 
   LPCTSTR pszWndClass, WNDPROC wndProcNew ) 

   : m_wndProc( wndProcNew )
   , m_wndProcSaved( 0 )
   , m_pszWndClass( pszWndClass )
{
	assert( isGoodStringPtrOrAtom( pszWndClass ) );
	assert( isGoodCodePtr( wndProcNew ) );

	trace( _T( "Creating global subclassing %s\n" ), 
		stringFromAtom( pszWndClass ) );  

	HWND hwnd = CreateWindow( pszWndClass, _T( "" ), 
		WS_POPUP, 0, 0, 0, 0, HWND_DESKTOP, 0, getModuleHandle(), 0 );
	if ( IsWindow( hwnd ) ) {
		m_wndProcSaved = (WNDPROC) SetClassLong( 
			hwnd, GCL_WNDPROC, reinterpret_cast< LONG >( wndProcNew ) );
//		DWORD realProc = GetClassLong( hwnd, GCL_WNDPROC );
		verify( DestroyWindow( hwnd ) );

//		hwnd = CreateWindow( m_pszWndClass, _T( "" ), WS_POPUP,
//			0, 0, 0, 0, HWND_DESKTOP, 0, getModuleHandle(), 0 );
//		if ( IsWindow( hwnd ) ) {
//			const DWORD realProc = GetClassLong( hwnd, GCL_WNDPROC );
//			const DWORD assumedProc = reinterpret_cast< DWORD >( m_wndProc );
//			assert( realProc == assumedProc );
//			verify( DestroyWindow( hwnd ) );
//		}

	} else {
		const DWORD dwErr = GetLastError();
		trace( _T( "Global subclassing failure %s\nReason: %s\n" ), 
			stringFromAtom( pszWndClass ),
			WinException( dwErr ).what() );  
	}
	if ( 0 == m_wndProcSaved ) {
		FatalAppExit( 0, _T( "Global subclassing failed" ) );
	}
}


GlobalSubclasser::~GlobalSubclasser() {

	trace( _T( "Destroying global subclasser %s\n" ), 
		stringFromAtom( m_pszWndClass ) );  

	try {
		assert( isGoodStringPtrOrAtom( m_pszWndClass ) );
		assert( 0 != m_wndProcSaved );
		if ( 0 != m_wndProcSaved ) {
			HWND hwnd = CreateWindow( m_pszWndClass, _T( "" ), WS_POPUP,
				0, 0, 0, 0, HWND_DESKTOP, 0, getModuleHandle(), 0 );
			if ( IsWindow( hwnd ) ) {
				SetClassLong( hwnd, GCL_WNDPROC, reinterpret_cast< LONG >( m_wndProcSaved ) );
				verify( DestroyWindow( hwnd ) );
			}
		}

	}

    // No exceptions may leave the destructor!
	// This one's always called at exit time anyway.
	catch ( ... ) {
		trace( _T( "*** Exception in Subclasser dtor ignored\n" ) );
	}
}

// end of file

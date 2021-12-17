/*
 * $Header: /Book/StatusBar.h 13    16.07.04 10:42 Oslph312 $
 */

#pragma once

#include "Window.h"
#include "AutoPtr.h"
#include "geometry.h"


class Statusbar : public Window {
private:
	enum StatusBarParts {
		message_part ,
		position_part,
		filetype_part,
		action_part  ,
	};

	HICON m_hicon;
	int   m_nIndex;

	Point position;
	int zoomPercentage;

	void __cdecl setMessageV( LPCTSTR pszFmt, va_list vl );
	void setText( int nIndex, LPCTSTR pszText );
	void update( void );

public:
	Statusbar( HWND hwndParent, UINT uiID );
	virtual ~Statusbar();

	void __cdecl setMessage( LPCTSTR pszFmt, ... );
	void __cdecl setMessage( UINT idFmt, ... );
	void __cdecl setHighlightMessage( UINT idFmt, ... );
	void __cdecl setErrorMessage( UINT idFlags, UINT idFmt, ... );
	void update( const Point& position );
	void update( const int zoomPercentage );
	void setFileType( const bool isUnicode );
	void setIcon( int nIndex = 0 );
	void setIcon( HIMAGELIST hImageList, int nIndex );

	virtual void onSettingChange( LPCTSTR pszSection );
};


inline void Statusbar::setText( int nIndex, LPCTSTR pszText ) {

   sendMessage( 
      SB_SETTEXT, nIndex, reinterpret_cast< LPARAM >( pszText ) );
}


class TemporaryStatusIcon {
private:
   Statusbar *m_pStatusbar;

public:
   TemporaryStatusIcon( Statusbar *, HIMAGELIST, int );
   TemporaryStatusIcon( Statusbar *, int );
   ~TemporaryStatusIcon();
};


inline TemporaryStatusIcon::TemporaryStatusIcon( 
   Statusbar *pStatusbar, HIMAGELIST hImageList, int nIcon )
   : m_pStatusbar( pStatusbar )
{
   assert( isGoodPtr( m_pStatusbar ) );
   m_pStatusbar->setIcon( hImageList, nIcon );
}


inline TemporaryStatusIcon::TemporaryStatusIcon( 
   Statusbar *pStatusbar, int nIcon ) 
   : m_pStatusbar( pStatusbar )
{
   assert( isGoodPtr( m_pStatusbar ) );
   m_pStatusbar->setIcon( nIcon );
}


inline TemporaryStatusIcon::~TemporaryStatusIcon() {
   assert( isGoodPtr( m_pStatusbar ) );
   m_pStatusbar->setIcon();
}


#if 0
class TemporaryStatusMessage {
private:
   Statusbar& m_Statusbar;

public:
   TemporaryStatusMessage( Statusbar& statusbar, int nIcon );
   ~TemporaryStatusMessage();
};


inline TemporaryStatusMessage::TemporaryStatusMessage( 
   Statusbar& statusbar, int nIcon ) 
   : m_Statusbar( statusbar )
{
   m_Statusbar.setIcon( nIcon );
}


inline TemporaryStatusMessage::~TemporaryStatusMessage() {
   m_Statusbar.setIcon();
}
#endif

// end of file

/*
 * $Header: /Book/Editor.h 18    16.07.04 10:42 Oslph312 $
 */

#pragma once

#include "AbstractEditWnd.h"
#include "String.h"
#include "Document.h"
#include "Toolbar.h"
#include "Statusbar.h"
#include "utils.h"


typedef AutoPtr< AbstractEditWnd > AutoEditWnd  ;
typedef AutoPtr< Document        > AutoDocument ;
typedef AutoPtr< Toolbar         > AutoToolbar  ;
typedef AutoPtr< Statusbar       > AutoStatusbar;


class Editor : public EditListener {

private:
	HWND          m_hwndMain;
	HACCEL        m_haccel;
	AutoEditWnd   m_pEditWnd;
	AutoDocument  m_pDocument;
	AutoToolbar   m_pToolbar;
	AutoStatusbar m_pStatusbar;

	HFONT m_hfontFixed;
	HFONT m_hfontProportional;

	LOGFONT m_logFontFixed;
	LOGFONT m_logFontProportional;

	bool m_hasFixedFont;
	bool m_isClean;
	bool m_isWhistleClean;

	LPTSTR getContents( void );
	void updateToolbar( void );

	static void CALLBACK autoSaveTimerProc( HWND, UINT, UINT, DWORD );

	void assertValid( void );
	void assertValid( void ) const;

public:
	Editor( HWND hwndParent, AutoDocument *ppDocument );
	~Editor();

	HWND getMainWnd( void );
	void detach( HWND hwndMain );
	AbstractEditWnd *getEditWnd( void );
	Toolbar *getToolbar( void );
	Statusbar *getStatusbar( void );
	Document *getDocument( void );
	void setDocument( Document *pNewDocument );

	void setTitle( void );
	void refreshToolbar( void );
	void setSettings( void );
	void setWordWrap( bool bOn );

	void loadAcceleratorTable( HINSTANCE = getModuleHandle() );
	int run();

	void openFile( const String& strPath );
	void reload();
	void copyFile( void );
	void printFile( void );
	bool save( void ) throw();
	bool saveIfNecessary( void );
	void restoreOriginal( void );
	bool searchAndSelect( 
		const String& strSearchPattern, bool *pbWrapped = 0 );
	int getAutoSaveTime( void ) const;
	void saveState( void );

	HFONT getFont( void ) const;
	HFONT getFont( bool bFixedFont ) const;
	void setFont( bool isFixedFont );
	bool hasFixedFont( void ) const;
	const LOGFONT *getLogFont( void ) const;
	const LOGFONT *getLogFont( bool bFixed ) const;
	void setLogFont( const LOGFONT *pLogFont, bool bFixed );

	String getMenuDescription( HMENU hmnuPopup ) const;
	String getMenuItemDescription( 
		int nItem, const String& strLast ) const;

	bool isToolbarDialogMessage( MSG *pMsg );

	void clean( void );
	bool isClean( void ) const;
	bool isWhistleClean( void ) const;
	void setReadOnly( bool bReadOnly );
	bool getReadOnly( void ) const;

	friend Editor *getEditor( HWND hwnd );

protected: // implementation of EditListener
	virtual void onChange   ( void );
	virtual void onMaxText  ( void );
	virtual void onErrSpace ( void );
	virtual void onPosChange( const Point& position );
	virtual void onZoomChange( const int percent );
};


typedef AutoPtr< Editor > AutoEditor;


inline HWND Editor::getMainWnd( void ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( m_hwndMain ) );
   return m_hwndMain;
}


inline void Editor::detach( HWND hwndMain ) {
   
   assert( IsWindow( m_hwndMain ) );
   assert( m_hwndMain == hwndMain  );
   m_hwndMain = 0;
}


inline AbstractEditWnd *Editor::getEditWnd( void ) {

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pEditWnd ) );
   return m_pEditWnd;
}


inline Toolbar *Editor::getToolbar( void ) {
   
   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pToolbar ) );
   return m_pToolbar;
}


inline Statusbar *Editor::getStatusbar( void ) {
   
   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pStatusbar) );
   return m_pStatusbar;
}


inline Document *Editor::getDocument( void ) {

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pDocument ) );
   return m_pDocument;
}


inline bool Editor::hasFixedFont( void ) const {
   
   assert( isGoodConstPtr( this ) );
   return m_hasFixedFont;
}


inline const LOGFONT *Editor::getLogFont() const {
   
   assert( isGoodConstPtr( this ) );
   return getLogFont( hasFixedFont() );
}


inline const LOGFONT *Editor::getLogFont( bool bFixed ) const {
   
   assert( isGoodConstPtr( this ) );
   return bFixed ? &m_logFontFixed : & m_logFontProportional;
}


inline bool Editor::isToolbarDialogMessage( MSG *pMsg ) {
   
   assert( isGoodHeapPtr( this ) );
   assert( isGoodPtr( pMsg ) );
   const HWND hwndToolbar = *getToolbar();
   return 0 != IsDialogMessage( hwndToolbar, pMsg );
}


inline void Editor::clean( void ) {
   m_isClean = true;
}


inline bool Editor::isClean( void ) const {
   
   assert( isGoodConstPtr( this ) );
   return m_isClean && m_pDocument->isClean();
}


inline bool Editor::isWhistleClean( void ) const {
   
   assert( isGoodConstPtr( this ) );
   return m_isWhistleClean;
}


inline void Editor::assertValid( void ) {
   
   assert( isGoodPtr( this ) );
   static_cast< const Editor * >( this )->assertValid();
}


inline void Editor::assertValid( void ) const {
   
   assert( isGoodConstPtr( this ) );
   assert( 0 == m_hwndMain || IsWindow( m_hwndMain ) );
}

// end of file

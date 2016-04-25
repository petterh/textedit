/*
 * $Header: /Book/AbstractEditWnd.h 18    16.07.04 10:42 Oslph312 $
 *
 * Defines AbstractEditWnd and the pure interface EditListener.
 */

#pragma once

#include "String.h"
#include "Window.h"
#include "AutoPtr.h"
#include "geometry.h"

class EditListener;

/**
 * This is an _almost_ pure interface to an actual edit window. 
 */
class AbstractEditWnd : public Window {
private:
	int getVisibleLineCount( HFONT hfont ) const;
	int moveInsertionPoint( int nLine, int nColumn );

	EditListener *m_pEditListener;
	int m_nCurLine  ;
	int m_nCurColumn;
	int m_nLastEnd  ;
	int m_numerator ;
	int m_denominator;

protected:
	bool wordWrap;

	AbstractEditWnd( void );

public:
	virtual LRESULT dispatch( 
		UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );

	void resetPosition( void );

#ifdef _DEBUG
	static bool bForceEdit;
#endif

	virtual int getSearchText( LPTSTR psz, UINT cb ) const = 0;
	virtual void getText( LPTSTR psz, UINT cb ) const = 0;
	virtual void setText( LPCTSTR psz ) = 0; // WM_SETTEXT

	virtual int getTextLength( bool bCurPosCompatible = false ) const;
	virtual int   getLineCount ( void ) const;
	virtual Point getCurPos    ( void ) const = 0;

	/** 
	* getSel returns true if there actually is a selection,
	* i.e., start is less than end.
	*/
	virtual bool getSel( int *pnStart = 0, int *pnEnd = 0 ) const = 0;
	virtual bool getSel( String *pstrSelection )            const = 0;
	virtual void setSel( int nStart = 0, int nEnd = -1 )          = 0;
	virtual void replaceSel( LPCTSTR psz )                        = 0;
	virtual bool getWord( String *pstrWord )                const = 0;

	virtual int lineFromChar( UINT ich )          const = 0;
	virtual int getFirstVisibleLine( void )       const = 0;
	virtual void setFirstVisibleLine( int nLine ) const = 0;

	virtual bool canUndo( void )       const = 0;
	virtual bool undo( void )                = 0;
	virtual String getUndoName( void ) const = 0;

	virtual bool hasRedo( void )       const = 0;
	virtual bool canRedo( void )       const = 0;
	virtual bool redo( void                ) = 0;
	virtual String getRedoName( void ) const = 0;

	virtual bool canSetTabs( void ) const = 0;

	virtual bool isDirty( void ) const = 0;
	virtual void clean( bool bEmptyUndo = true ) = 0;

	virtual void cutSelection( void ) = 0;
	virtual void copySelection( void ) = 0;
	virtual void paste( void ) = 0;
	virtual void deleteSelection( void ) = 0;

	virtual void setReadOnly( bool bReadOnly ) = 0;
	virtual bool isReadOnly( void ) const = 0;

	virtual void setSpacesPerTab( int nSpaces ) = 0;
	virtual int getSpacesPerTab( void ) = 0;
	virtual void setWordWrap( bool bWordWrap ) {
		this->wordWrap = bWordWrap;
	}

	virtual bool searchAndSelect( 
		const String& strSearchPattern, 
		const bool bMatchWholeWord, 
		const bool bMatchCase,
		const int  nDirection );
	virtual int replaceInSelection( 
		const String& strSearchPattern, 
		const String& strReplacePattern,
		const bool bMatchWholeWord, 
		const bool bMatchCase );

	void scroll( UINT nWhat );
	void bringCaretToWindow( HFONT hfont );
	EditListener *getEditListener( void );
	bool indentSelection( const bool execute );

	// Factory method:
	static AbstractEditWnd *create( HWND hwndParent, 
		LPCTSTR pszText, EditListener *pEditListener, bool bWordWrap );
};


/**
 * This is a pure interface. 
 * Use it to listen to AbstractEditWnd events.
 */
class EditListener {
public:
   virtual void onChange   ( void ) = 0;
   virtual void onMaxText  ( void ) = 0;
   virtual void onErrSpace ( void ) = 0;
   virtual void onPosChange( const Point& position ) = 0;
   virtual void onZoomChange( const int percent ) = 0;
};


inline AbstractEditWnd::AbstractEditWnd( void ) 
	: m_pEditListener(  0 )
	, m_nCurLine     ( -1 )
	, m_nCurColumn   ( -1 )
	, m_nLastEnd     ( -1 )
	, m_numerator    (  0 )
	, m_denominator  (  0 )
{
}


inline void AbstractEditWnd::resetPosition( void ) {
   m_nCurLine = m_nCurColumn = m_nLastEnd = -1;
   m_numerator = m_denominator = 0;
}


inline EditListener *AbstractEditWnd::getEditListener( void ) {
   return m_pEditListener;
}

// end of file

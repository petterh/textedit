/*
 * $Header: /Book/Toolbar.h 17    16.07.04 10:42 Oslph312 $
 */

#pragma once


#include "Window.h"
#include "AutoPtr.h"
#include "geometry.h"


enum {
   num_std_icons = STD_PRINT - STD_CUT + 1, // see commctrl.h
   fixed_icon    = num_std_icons + 0,
   prop_icon     = num_std_icons + 1,
   wordwrap_icon = num_std_icons + 2,
   find_icon     = num_std_icons + 3,
};


class Editor;
class Toolbar : public Window {
private: // TBSTYLE_WRAPABLE | CCS_NODIVIDER
   enum { WS_TOOLBAR = TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_TOP | TBSTYLE_TRANSPARENT };

   void adjust( bool bRepaint, bool hasRedo, bool canSetTabs );
   void setFonts( void );
   void setButtons( bool hasRedo );
   int commandFromIndex( int nIndex );

   virtual void onCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual LRESULT onNotify( const int id, const LPNMHDR pHdr );
   virtual void onLButtonDown(
      BOOL fDoubleClick, int x, int y, UINT keyFlags );
   virtual void onSettingChange( LPCTSTR pszSection );
   //virtual void onSysColorChange( void );

   void onDeltaPos   ( NMUPDOWN *pNmUpDown );
   bool onGetDispInfo( NMTTDISPINFO *pDispInfo );

   HWND getChild( UINT uiID ) const;

protected:
   void assertValid( void );
   void assertValid( void ) const;

public:
   Toolbar( Editor *pEditor, UINT uiID, bool hasRedo, bool canSetTabs );
   virtual ~Toolbar();

   virtual LRESULT dispatch(
      UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );

   bool isEnabled( UINT uiID ) const;
   void setEnabled( UINT uiID, bool bEnable );
   void check( UINT uiID, bool bCheck );

   // This retrieves a child window (*not* an image button):
   HWND getChild( UINT uiID );

   int getTabs( void );
   void setReadOnly( bool bReadOnly, bool bAccessDenied = false );
   bool getReadOnly( void ) const;
   void setSpacesPerTab( int nSpacesPerTab );
};


inline bool Toolbar::isEnabled( UINT uiID ) const {

   assertValid();
   return 0 != sendMessage( TB_ISBUTTONENABLED, uiID, 0 );
}


inline void Toolbar::setEnabled( UINT uiID, bool bEnable ) {

   assertValid();
   sendMessage( TB_ENABLEBUTTON, uiID, bEnable );
}


inline void Toolbar::check( UINT uiID, bool bCheck ) {

   assertValid();
   sendMessage( TB_CHECKBUTTON, uiID, bCheck );
}


inline HWND Toolbar::getChild( UINT uiID ) const {

   assertValid();
   assert( (UINT) -1 != uiID && 0 != uiID );

   HWND hwndChild = GetDlgItem( m_hwnd, uiID );
   assert( IsWindow( hwndChild ) );
   return hwndChild;
}


inline HWND Toolbar::getChild( UINT uiID ) {

   return const_cast< const Toolbar * >( this )->getChild( uiID );
}


inline void Toolbar::assertValid( void ) {
   assert( isGoodPtr( this ) );
   Window::assertValid();
}


inline void Toolbar::assertValid( void ) const {
   assert( isGoodConstPtr( this ) );
   Window::assertValid();
}


#ifndef TBDDRET_DEFAULT
#define TBDDRET_DEFAULT 0
#endif

// end of file

/*
 * $Header: /Book/WaitCursor.h 6     28.11.99 22:18 Oslph312 $
 *
 * The WaitCursor class displays hourglass cursors during lengthy 
 * operations. It uses named alternatives to the standard wait
 * cursors if they are available, e.g. for printing.
 *
 * The WaitCursor class has become more advanced since the text of
 * PISW went to print. Instead of displaying the wait cursor right
 * away, it starts a thread that waits a bit. This avoids a quick
 * wait-cursor flicker in the operation turns out not to be so
 * time-consuming after all.
 * TODO: Make one MT, one ST version.
 */

#pragma once


class WaitCursor {
private:
   int     _nTimeIn;
   HANDLE  _hThread;
   HANDLE  _hEvent;
   DWORD   _dwParentThreadId;
   HCURSOR _hcur;
   bool    _isFromFile;

   void _attachThreadInput( bool bAttach ) const;
   void _finishThread( void );
   void _restore( void ) const;
   HCURSOR _loadCursor( LPCTSTR pszName );

   void _threadFunc( void ) const;
   static DWORD WINAPI _threadFunc( void *pData );

public:
   WaitCursor( LPCTSTR pszName = 0, int nTimeIn = 150 ); // ms
   ~WaitCursor() throw();
   void restore( void );
};

// end of file

### Programming Industrial Strength Windows
[« Previous: Customization and Persistence](Chapter-10-Customization-and-Persistence.md) — [Next: File I/O »](Chapter-12-File-I-O.md)
# Chapter 11: Wait a Moment

Before I get into potentially time-consuming operations such as File I/O and printing, I’ll digress into the subject of the wait cursor. I’ll describe a problem with the typical implementation, and discuss enhancements to the standard hourglass.

Most Windows applications display an hourglass cursor during lengthy operations. The implementation usually looks something like this:

{code:C#}
// Set hourglass cursor, saving the current one:
const HCURSOR hcurSave = SetCursor( LoadCursor( 0, IDC_WAIT ) );
// ...perform lengthy operation here...
// Restore old cursor:
SetCursor( hcurSave );
{code:C#}
SetCursor returns a handle to the previous cursor; this suggests such usage. Most Windows books I’ve seen do the above, and the MFC class CWaitCursor encapsulates the same functionality.  

What’s wrong with this picture? Consider the following sequence of events:

# When the lengthy operation starts, the cursor is an IDC{"_"}ARROW, a fact remembered by hcurSave.
# During the operation, the user moves the mouse, and the hourglass cursor comes to rest on a split bar.
# The operation ends, and the cursor is changed back to an arrow.

The problem is that the cursor isn't supposed to be an arrow when it’s on the split bar. As soon as the mouse pointer is moved, things will be dandy again, but in the meantime, there’s this tiny crack in the smooth surface of illusion we strive to create for our users.  

This particular crack is easily mended, and the result is actually simpler to use than the standard example above. What we need is a function analogous to InvalidateRect that operates on the cursor. We no longer need to save the existing cursor, and the above example becomes:

{code:C#}
// Set hourglass cursor:
SetCursor( LoadCursor( 0, IDC_WAIT ) );
// ...perform lengthy operation here...
// Restore normal cursor:
InvalidateCursor();
{code:C#}
The implementation of InvalidateCursor is simple:

{code:C#}
void InvalidateCursor ( void ) {
   POINT pt; // Screen coordinates!
   GetCursorPos( &pt );
   SetCursorPos( pt.x, pt.y );
}
{code:C#}
This moves the cursor to its current location (i.e., the cursor does not actually move anywhere), forcing a WM{"_"}SETCURSOR message in the process. Doing it this way is a lot simpler than synthesizing and dispatching a WM{"_"}SETCURSOR message. 

GetCursorPos and SetCursorPos have changed somewhat during the migration from Win16 to Win32. In Win16, these were void functions; in Win32, they each return TRUE for success and FALSE for failure. As is its wont, however, Microsoft’s documentation is vague about possible causes of failure. 

An improved implementation of MFC’s CWaitCursor class might have the following declaration:

{code:C#}
class WaitCursor {
public:
   WaitCursor();
   ~WaitCursor();
   void restore();
};
{code:C#}
The constructor and the restore method are both trivial; the interesting part is the destructor:

{code:C#}
WaitCursor::WaitCursor() {
   SetCursor( LoadCursor( 0, IDC_WAIT ) ); // or call restore...
}

void WaitCursor::restore() {
   SetCursor( LoadCursor( 0, IDC_WAIT ) );
}

/**
 * Forces a WM_SETCURSOR message.
 */ 
WaitCursor::~WaitCursor() {
   POINT pt; // Screen coordinates!

#ifdef _WIN32
   if ( !GetCursorPos( &pt ) ) {
      trace( _T( "GetCursorPos Failed\n" ) );
   } else if ( !SetCursorPos( pt.x, pt.y ) ) {
      trace( _T( "SetCursorPos(%d,%d) Failed\n" ), pt.x, pt.y ) );
   }
#else // Win16
   GetCursorPos( &pt );
   SetCursorPos( pt.x, pt.y );
#endif
}
{code:C#}

## Changing the Cursor Image

The Microsoft Office suite comes with a nifty set of animated wait cursors that indicate just what kind of lengthy operation is going on – opening, saving or printing a file, for example. This is more than just a gimmick; it gives useful feedback that it would be nice to provide for TextEdit.

It obviously wouldn’t do to steal Microsoft’s cursors and ship them with TextEdit, but what about users that already have these cursors installed – surely it is OK for them to use the cursors?

**Listing 48: WaitCursor.cpp**

{code:C#}
#define CURSOR_PATH _T( "Cursors\\" )


PRIVATE inline HCURSOR loadCursor( LPCTSTR pszName ) {
  return static_cast< HCURSOR >( LoadImage( 0, pszName, 
     IMAGE_CURSOR, 0, 0, 
     LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_LOADFROMFILE ) );
}


/*
* If we don't call AttachThreadInput, SetCursor has 
* no effect when called from the worker thread.
*/
inline void WaitCursor::_attachThreadInput( bool bAttach ) const {

  verify( AttachThreadInput( 
     GetCurrentThreadId(), _dwParentThreadId, bAttach ) );
}


void WaitCursor::_threadFunc( void ) const {

  const DWORD dwRet = WaitForSingleObject( _hEvent, _nTimeIn );
  if ( WAIT_TIMEOUT == dwRet ) {
     _attachThreadInput( true );
     _restore();
     _attachThreadInput( false ); // Works OK withouth this...
     // ...but without this, no animation unless in debugger.
     verify( WAIT_OBJECT_0 == WaitForSingleObject( _hEvent, INFINITE ) );
  }
  assert( (DWORD) -1 != dwRet );
}


DWORD WINAPI WaitCursor::_threadFunc( void *pData ) {

  const WaitCursor *pThis = 
     reinterpret_cast< WaitCursor * >( pData );
  assert( isGoodConstPtr( pThis ) );
  pThis->_threadFunc();
  return 0;
}


void WaitCursor::_finishThread( void ) {

  if ( 0 != _hEvent ) {
     // Signal event, if necessary, to wake the thread.
     // If the thread has finished already, no matter.
     verify( SetEvent( _hEvent ) );
  }

  if ( 0 != _hThread ) {
     verify( WAIT_OBJECT_0 == WaitForSingleObject( _hThread, INFINITE ) );
     verify( CloseHandle( _hThread ) );
     _hThread = 0;
  }

  if ( 0 != _hEvent ) {
     verify( CloseHandle( _hEvent ) );
     _hEvent  = 0;
  }
}


void WaitCursor::_restore( void ) const {

  // Actually, 0 is an acceptable value for SetCursor,
  // but _hcur is nevertheless not **supposed** to be 0 here:
  assert( 0 != _hcur );
  SetCursor( _hcur );
}


HCURSOR WaitCursor::_loadCursor( LPCTSTR pszName ) {

  if ( 0 != pszName ) {
     assert( isGoodStringPtr( pszName ) );

     String strCursor( CURSOR_PATH );
     strCursor += pszName;

     PATHNAME szWindowsDirectory = { 0 };
     const int nChars = GetWindowsDirectory( 
        szWindowsDirectory, dim( szWindowsDirectory ) );
     if ( 0 < nChars && nChars < dim( szWindowsDirectory ) ) {
        // We now have something like C:\\WINNT or C:\\WINDOWS. 
        // There's a terminating \\ if it is a root directory.
        PATHNAME szCursorPath = { 0 };
        _tmakepath( szCursorPath, 0, szWindowsDirectory, 
           strCursor.c_str() , 0 );
        
        // Sample szCursorPath = "C:\\WINNT\\Cursors\\load.ani"
        return loadCursor( szCursorPath );
     }
  }

  return 0;
}


/**
* The constructor attempts to load the named cursor.
* The pszName parameter should be relative to the Windows directory.
*/
WaitCursor::WaitCursor( LPCTSTR pszName, int nTimeIn )
  : _nTimeIn( nTimeIn )
  , _hThread( 0 )
  , _hEvent( 0 )
  , _dwParentThreadId( GetCurrentThreadId() )
  , _hcur( _loadCursor( pszName ) )
  , _isFromFile( 0 != _hcur ) 
{
  if ( !_isFromFile ) {
     _hcur = LoadCursor( 0, IDC_WAIT ); // NOTE: LR_SHARED is undoc!
  }
  if ( 0 < _nTimeIn ) {
     _hEvent = CreateEvent( 0, true, false, 0 );
     DWORD dwThreadId = 0; // Required under Win9x, not under NT.
     _hThread = CreateThread( 0, 0, _threadFunc, this, 0, &dwThreadId );
  }
  if ( 0 == _hThread ) {
     _restore();
  }
}

 
WaitCursor::~WaitCursor() throw() {
  _finishThread();
  assert( 0 != _hcur );
  if ( _isFromFile ) {
     //verify( DestroyCursor( _hcur ) ); // NOTE: This fails.
     DestroyCursor( _hcur );
  }
  reset_pointer( _hcur );
 
  POINT pt; // Screen coordinates.
 
  // SetCursorPos forces a WM_SETCURSOR message.
  if ( !GetCursorPos( &pt ) ) {
     trace( _T( "GetCursorPos failed!" ) );
     assert( false );
  } else if ( !SetCursorPos( pt.x, pt.y ) ) {
     trace( _T( "SetCursorPos( %d, %d ) failed!" ), pt.x, pt.y );
     assert( false );
  }
}

 
void WaitCursor::restore( void ) {
  _finishThread();
  _restore();
}
{code:C#}
### Programming Industrial Strength Windows
[« Previous: The Bare Bones](Chapter-5-—-The-Bare-Bones) — [Next: Off the Launch Pad »](Chapter-7-—-Off-the-Launch-Pad)
# Chapter 6: Exceptions

When your application calls a function, there’s always a chance that the function fails to carry out its assigned duties. Possible causes are multifarious – a file was not found, a network file server crashed, the system could not allocate memory, a computation overflowed, the system is corrupt, your application is corrupt, the government is corrupt – the list is endless. 

The occasional failure is unavoidable. Failing gracefully is, however, infinitely preferable to falling flat on your face.

Sometimes you don’t care if a function failed. Perhaps it wasn’t terribly important, perhaps the function has already reported the problem to the user, or perhaps no reasonable course of action exists. The WaitCursor class (see [Chapter 11](Chapter-11-—-Wait-a-Moment)) is an example of no-fail software. Its methods don’t need to communicate success or failure to the caller, because the caller doesn’t care one way or another. 

Other times, you do care, if for no other reason that you must tell the user that you failed to print his file. Failure can be reported through two basic mechanisms: return codes and exceptions. From modern computer literature, one often gets the impression that exceptions are in some way “better” than return codes, but that distinction is meaningless. The goal is to reduce total complexity to a minimum. Often enough, exceptions support that goal best, but by no means always. 

There is one truly bad way, and it occurs all too often in real life. Programmers new to exceptions will often just treat them as a complicated form of return codes, enclosing each function call in a try block, with one or more catch block for each function call. This is a lot more wordy and unreadable than just using return codes – when used properly, exceptions separate functional code from error handling, making it much easier to follow the logic of the nominal path.


Your application lives in a hostile environment, and a certain amount of paranoia is fully justified. If you design error handling as an integral part of your application architecture, you stand a chance of protecting yourself. If you don’t, you’ll get yourself into trouble, sooner rather than later.

## Return Codes

When you do use return codes, keep the following design principle in mind:

**“Never mix return values with return codes.”**

Why not? The standard library function getchar is a good example. It reads a character from the standard input, returning a byte-sized value. Since all possible byte-sized values are permissible character codes, none are left over to signal conditions such as end-of-file. This problem has been “solved” by having the function return not a char, but an int, thus allowing the special return value EOF, equal to –1.

**EOF** falls outside the byte range. If you stuff the return value from getchar into a variable of type char, seemingly a reasonable thing to do, only the low byte is retained, and EOF becomes undistinguishable from 0xff. The char variable never compares equal to EOF.

Consider **atoi**, which converts a string to an integer. The return value of this function is the desired integer. Unfortunately, since all the possible return values are permissible integers, none are left over to signal errors. This problem has been “solved” by designating zero as an error return. Thus, applying atoi to the string “garbage” returns zero. Applying it to the string “0” also returns zero. 

If the conversion overflows, the result is “undefined.” Why not return zero in this case, too? Because the implementation does not even know that the result has overflowed! I would imagine that the quest for the Holy Grail of Efficiency is more to blame than laziness. “Efficiency” is the root of, if not all, then at least the occasional evil: Programmers’ obsession with efficiency sometimes overshadows more important goals, such as correctness. 

In one sense, atoi doesn’t have “errors.” It all depends on how you define its behavior, and atoi’s behavior is defined in terms of its implementation. This is totally backwards; as a result, atoi is unusable without additional error checking. This kind of programming is shaky under the best of circumstances, but if you apply atoi directly to user input, you’re really courting disaster.

Consider malloc. This function also uses a special value – zero, again – to signal failure. In contrast to atoi’s error value, malloc’s error value is outside the range of valid pointer values – the null pointer is a well-defined concept in C and {"C++"} programming. But no attention is drawn to the possibility of failure, and it is therefore less likely that error checking will actually be performed. I was once involved with a large Unix application in which all calls to malloc were simply assumed to succeed. When I asked why, the programmer said, in essence, “if memory allocation fails, we’re already in shit so deep that we’ve no chance of bailing out.” 

The standard C library is a notorious violator of the principle of separating return values from return codes, and I shudder when I contemplate the debugging man-years that have been wasted on this. If you look at the evolution of the Windows API, you’ll find a distinct trend towards better separation of return values and return codes. Once upon a time, for example, there was a function named **GetWindowOrg**, declared like this:

{code:C#}
DWORD GetWindowOrg( HDC );
{code:C#}
The return value stuffed two 16-bit coordinates into a DWORD. This left no good way to signal errors, and created a tight coupling between the function and the size of GDI coordinates. Contrast this with its successor, GetWindowOrgEx:

{code:C#}
BOOL GetWindowOrgEx( HDC, LPPOINT );
{code:C#}
Here we have a clean separation between return code and return value, and changing the coordinate type from 16 to 32 to any number of bits becomes a simple matter of a recompile.

## Exceptions

The C language doesn’t have exception handling, except in the limited form provided by setjmp and longjmp, and in the form of language extensions to deal with Structured Exception Handling (SEH) under 32-bit Windows. 

The {"C++"} language does have exception handling. Aside from syntax, one major difference from setjmp/longjmp is the “unwinding of the stack.” This means that the destructors of all stack-allocated objects are called before the exception handler is invoked, giving each stack frame a chance to clean up after itself. Another difference is that throwing a real exception invokes the first applicable handler it finds on the stack, rather than a specifically identified handler, as does longjmp.

## Two-stage Construction

{"C++"} constructors lack return values. This is no accident. If they did, you’d have a problem with allocating arrays of objects – which element’s constructor should be responsible for the return value?

The natural way to signal errors in a {"C++"} constructor is by means of exceptions. This can be an inconvenience, though – sometimes you actually want return codes. 

A common solution to this conundrum is called two-stage construction. The constructor simply does nothing that might cause problems; instead, some method is called after construction to do the “real” initialization. Consider the MFC CFile class, for example. CFile is designed to work both ways, depending on which constructor you use. One way is to use the default (i.e., parameterless) constructor, which does not open any files, followed by a call to the Open method, which returns an error code if it fails to open the specified file. The other way is to use a constructor that takes a file name; this constructor throws an exception if it fails to open the file.

There is no “right” or “wrong” here; the guiding principle to follow is this:

**“Keep total program complexity to a minimum.”**
 
The VersionInfo class demonstrates yet another option. If the constructor fails, it sets an internal flag to indicate this, and the application must call the auxiliary method isValid before actually using the instance.

## Global Object Constructor Exceptions

What happens if you throw an exception from the construction of a global object – one at file scope? You can only catch it by getting compiler-dependent. 

In the case of a Visual {"C++"} program, your real entry point is not WinMain, but an extern “C” function called WinMainCRTStartup (or wWinMainCRTStartup, if you are a Unicode application). This function performs all the initialization required and then calls WinMain (or wWinMain, as the case may be). Among other things, it calls a function named _initterm, which walks a list of function pointers and calls them. The _initterm function is first called with a list of run-time library initializers, for example __initstdio, and then with a list of {"C++"} constructors for global objects.

To catch an exception thrown during the construction of a global object, you must override WinMainCRTStartup, or its equivalent in other environments. If you don’t need the C Runtime Library (CRT) and have no static instances of classes, this may actually be a good idea, since it reduces the size of the executable and speeds up loading. Otherwise, it is a bad idea; you’d have to copy all the initialization code, and you’d have no guarantee that the next compiler release wouldn’t break your code. As for porting to other compilers, that’s out.

## The TextEdit Exception Classes

### Listing 21: Exception.h

{code:C#}
...
#define INTERCEPT_SEH 1
 

/**
* Functional code should not handle the generic Exception class,
* only its descendants. 
*/
class Exception {
public:

#ifdef _DEBUG
  Exception() { trace( _T( "Exception ctor\n" ) ); }
#endif
  virtual ~Exception() { trace( _T( "Exception dtor\n" ) ); }

  virtual String getDescr( void ) const = 0;
  //virtual LPCTSTR what( void ) const;
};
 

#if 1
#define what() getDescr().c_str()
#else
inline LPCTSTR Exception::what( void ) const {
  return getDescr().c_str();
}
#endif
 

class CancelException : public Exception {
public:
  virtual String getDescr( void ) const { 
     return _T( "CancelException" ); 
  }
};
 

class NullPointerException : public Exception {
public:
  virtual String getDescr( void ) const { 
     return _T( "NullPointerException" ); 
  }
};

 
#if INTERCEPT_SEH

class sehException : public Exception {
public: 
  virtual String getDescr( void ) const;
};


class InvalidHandleException : public sehException {
public: 
  virtual String getDescr( void ) const;
};


class AccessViolationException : public sehException {
public: 
  virtual String getDescr( void ) const;
};


class DivideByZeroException : public sehException {
public: 
  virtual String getDescr( void ) const;
};


class StackOverflowException : public sehException {
public: 
  virtual String getDescr( void ) const;
  static StackOverflowException& getStackOverflowException();
};

#endif // INTERCEPT_SEH


class MemoryException : public Exception {
public:
  virtual String getDescr( void ) const;
  static MemoryException& getMemoryException( void );
};


class WinException : public Exception {
protected:
  const DWORD m_dwErr; // Error code from winerror.h

protected:
  String m_strDescr;

public:
  WinException( const String& strDescr, DWORD dwErr = GetLastError() );
  WinException( DWORD dwErr = GetLastError() );

  void resetLastError( void ) const;

  virtual String getDescr( void ) const;
};


inline void WinException::resetLastError( void ) const {
  SetLastError( m_dwErr );
}


String getError( 
  const String& strDescr, DWORD dwErr = GetLastError() );


inline String getError( DWORD dwErr = GetLastError() ) {
  return getError( _T( "" ), dwErr );
}


class ComException : public WinException {
public:
  ComException( HRESULT hres );
  virtual String getDescr( void ) const;
};


inline ComException::ComException( HRESULT hres ) 
  : WinException( hres )
{
}


class SubclassException : public WinException {
public:
  virtual String getDescr( void ) const;
};


#define DECLARE_SIMPLE( name, err ) \
  class name : public WinException {         \
  public:                                    \
     name( LPCTSTR pszDescr ) : WinException( pszDescr, err ) {};  \
     name( const String& strDescr ) : WinException( strDescr, err ) {};  \
     virtual String getDescr( void ) const { return WinException::getDescr(); }   \
  };

DECLARE_SIMPLE( FileNotFoundException    , ERROR_FILE_NOT_FOUND    );
DECLARE_SIMPLE( PathNotFoundException    , ERROR_PATH_NOT_FOUND    );
DECLARE_SIMPLE( AccessDeniedException    , ERROR_ACCESS_DENIED     );
DECLARE_SIMPLE( SharingViolationException, ERROR_SHARING_VIOLATION );


class CommonDialogException : public WinException {
public:
  CommonDialogException( DWORD dwErr = CommDlgExtendedError() );
  virtual String getDescr( void ) const;
};


void throwException( 
  const String& strDescr, DWORD dwErr = GetLastError() );


inline void throwException( DWORD dwErr = GetLastError() ) {
  throwException( _T( "" ), dwErr );
}


#define throwMemoryException()  throw MemoryException::getMemoryException()
{code:C#}

### Listing 22: Exception.cpp

{code:C#}
#if INTERCEPT_SEH

String sehException::getDescr( void ) const {
  return _T( "Unknown System Exception" );
};


String DivideByZeroException::getDescr( void ) const {
  return _T( "Division by zero" );
};


String StackOverflowException::getDescr( void ) const {
  return _T( "Stack overflow" );
};


String InvalidHandleException::getDescr( void ) const {
  return _T( "Invalid handle" );
};


String AccessViolationException::getDescr( void ) const {
  debugBreak();
  return _T( "Access violation" );
};


static StackOverflowException theStackOverflowException;

StackOverflowException& StackOverflowException::getStackOverflowException( void ) {
  return theStackOverflowException;
}

#endif // INTERCEPT_SEH


String getError( const String& strDescr, DWORD dwErr ) {
  
  String strError( strDescr );
  if ( !strDescr.empty() ) {
     strError += _T( "\n" );
  }

  LPTSTR pszMsgBuf = 0;
  
#ifdef _DEBUG
  const DWORD dwLen = 
#endif
     
     FormatMessage ( 
        FORMAT_MESSAGE_FROM_SYSTEM     | 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_IGNORE_INSERTS  ,
        0, dwErr,
        // LANGIDFROMLCID( GetThreadLocale() ),
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        reinterpret_cast< LPTSTR >( &pszMsgBuf ),
        0, 0 );
  
  if ( 0 == pszMsgBuf ) {
     pszMsgBuf = reinterpret_cast< LPTSTR >( LocalAlloc( LPTR, 50 * sizeof( TCHAR ) ) );
     assert( 0 != pszMsgBuf );
     wsprintf( pszMsgBuf, _T( "Unknown API error (%lu)" ), dwErr );
  }
  
  assert( 0 == dwLen || _tcsclen( pszMsgBuf ) == dwLen);
  strError += pszMsgBuf;
  LocalFree( pszMsgBuf );
  reset_pointer( pszMsgBuf );
  
#ifdef _DEBUG
  strError += formatMessage( _T( " [%1!u!](%1!u!)" ), dwErr );
#endif

  return strError;
}


WinException::WinException( const String& strDescr, DWORD dwErr )
  : m_dwErr( dwErr )
  , m_strDescr( strDescr )
{
}


WinException::WinException( DWORD dwErr )
  : m_dwErr( dwErr )
  , m_strDescr( _T( "" ) )
{
}


String WinException::getDescr( void ) const {
  return getError( m_strDescr, m_dwErr );
}


String ComException::getDescr( void ) const {
  switch ( m_dwErr ) {
  case S_OK: 
     return _T( "Success." );

  case S_FALSE:
     return _T( "The COM library is already initialized." );

  case REGDB_E_CLASSNOTREG:
     return
        _T( "A specified class is not registered in the " )
        _T( "registration database. Also can indicate that the " )
        _T( "type of server you requested in the CLSCTX " )
        _T( "enumeration is not registered or the values for " )
        _T( "the server types in the registry are corrupt." );

  case CLASS_E_NOAGGREGATION:
     return 
        _T( "This class can't be created as part of an aggregate." );
  }

  return getError( _T( "Com Error" ), m_dwErr );
}


String SubclassException::getDescr( void ) const {
  String strDescr( _T( "Internal error: Window subclassing failed\n\n" ) );
  strDescr += WinException::getDescr();
  return strDescr;
}


static MemoryException theMemoryException; // ERROR_OUTOFMEMORY

MemoryException& MemoryException::getMemoryException( void ) {

  return theMemoryException;
}


String MemoryException::getDescr( void ) const {
  return _T( "Out of memory" );
}
 

CommonDialogException::CommonDialogException( DWORD dwErr ) 
  : WinException( dwErr )
{
}


String CommonDialogException::getDescr( void ) const {

  static struct {
     DWORD   dwErr;
     LPCTSTR pszErr;
  } const aErrorTable[]() = {
     { CDERR_FINDRESFAILURE , _T( "Unable to find resource"   ) },
     { CDERR_NOHINSTANCE    , _T( "No HINSTANCE"              ) },
     { CDERR_INITIALIZATION , _T( "Initialization error"      ) },
     { CDERR_NOHOOK         , _T( "No hook"                   ) },
     { CDERR_LOCKRESFAILURE , _T( "Unable to lock resource"   ) },
     { CDERR_NOTEMPLATE     , _T( "No template"               ) },
     { CDERR_LOADRESFAILURE , _T( "Unable to load resource"   ) },
     { CDERR_STRUCTSIZE     , _T( "Wrong structure size"      ) },
     { CDERR_LOADSTRFAILURE , _T( "Unable to load string"     ) },
     { FNERR_BUFFERTOOSMALL , _T( "Buffer too small"          ) },
     { CDERR_MEMALLOCFAILURE, _T( "Memory allocation failure" ) },
     { FNERR_INVALIDFILENAME, _T( "Invalid file name"         ) },
     { CDERR_MEMLOCKFAILURE , _T( "Unable to lock memory"     ) },
     { FNERR_SUBCLASSFAILURE, _T( "Unable to subclass"        ) },
  };

  for ( int iErr = 0; iErr < dim( aErrorTable ); ++iErr ) {
     if ( aErrorTable[ iErr ](-iErr-).dwErr == m_dwErr ) {
        return aErrorTable[ iErr ](-iErr-).pszErr; //*** METHOD EXIT POINT
     }
  }

  return formatMessage( 
     _T( "Unknown common dialog error %1!lu!" ), m_dwErr );
}


void throwException( const String& strDescr, DWORD dwErr ) {
  if ( ERROR_NOT_ENOUGH_MEMORY == dwErr || ERROR_OUTOFMEMORY == dwErr ) 
  {
     throw MemoryException::getMemoryException();
  }

  switch ( dwErr ) {
  case ERROR_FILE_NOT_FOUND: throw FileNotFoundException( strDescr );
  case ERROR_PATH_NOT_FOUND: throw PathNotFoundException( strDescr );
  case ERROR_ACCESS_DENIED : throw AccessDeniedException( strDescr );
#if 0
  case ERROR_SHARING_VIOLATION: throw SharingViolationException( strDescr );
#endif
  }

  throw WinException( strDescr, dwErr );
}
{code:C#}

## Converting Allocation Failures to Exceptions

{"C++"} programs usually call operator new rather than malloc. Behind the scenes, though, the typical operator new implementation calls malloc to actually allocate memory; malloc, in turn, may call on the operating system for help. At any rate, while the {"C++"} standard mandates that an exception be thrown on failure, Microsoft’s compiler returns a null pointer if the allocation request fails. As a result, much code is littered with checks such as this:

{code:C#}
LPTSTR pszMyString = new TCHAR[ MAX_PATH ](-MAX_PATH-);
if ( 0 == pszMyString ) {
   // handle error
}
{code:C#}
Luckily, you can make even Microsoft’s operator new throw an exception on failure. The _set_new_mode function lets you decide whether an allocation failure should return zero or call an application-defined function; the _set_new_handler allows you to specify the function to call if an allocation fails. In this callback, you can do just about anything you like, such as throwing an exception. Just keep in mind that you may not have a lot of memory to play with! 

## Recovering from Errors

Some things you just can’t do much about. If your stack is corrupted, for example, your chances of bailing out are slim indeed; this is the software equivalent of taking a sledgehammer to the computer. Most errors aren’t that bad, though, and your chances of a graceful recovery depend on your error-handling architecture.

Guiding principle in imperative form:

**“Don’t lose the user’s data!”**
 
If TextEdit has problems getting started, it doesn’t matter too much. If we can inform the user of the cause before we call it quits, that’s sufficient. If, however, we run into problems in the middle of saving the text file, we have to bend over backwards to recover.

The error handling architecture in TextEdit is somewhat akin to an onion. The outer skin of the onion is the try/catch block in WinMain. Exceptions that make it this far are fatal; the user is informed of the problem’s cause via a message box, and the application dies.

Going deeper into the onion, we find the main message loop, in the Editor::run method. Each message is dispatched in a try/catch block; everything caught here is recoverable. Whenever an exception makes it here, some operation has failed; perhaps the user should be informed.

As we approach the core of the onion, we come to the handling of individual messages and commands. Some messages are relatively unimportant; it doesn’t really matter whether the onMenuSelect handler failed to set the text in the status bar. Others, such as the onCommandSave handler, are so critical that they handle all irregularities deep in their guts, and only let exceptions float to the surface under the most unusual circumstances.

The onion has comparatively few layers. With exception-safe objects, it turns out that you don’t need to catch exceptions all over the place; most are allowed to propagate through many levels. 

Guiding principle for exception handling:

**“Don’t handle general exceptions in functional code!”**

That’s the purview of architectural code. A corollary is that we need different exceptions for different problems; it’s insufficient merely to use error codes. This is difficult in practice, since winerror.h defines thousands of different error codes. Thus, in handling the WinException, functional code is allowed to catch it, check the code, and pass it on if the shoe didn’t fit. Mostly, though, code that can actually recover from the problem is so close to it as to use GetLastError directly, and WinException is caught only to provide error reporting.

This rule has one exception: You may catch and re-throw an exception, provided you use it as you would a final clause. The onion contains such emergency aid stations in various places – in the Dialog class, for example. It catches all exceptions, enables the parent window, and then rethrows the exception. (An emergency aid station in an onion? I think my vegetable metaphor is breaking down…)

Microsoft PowerPoint got this right. My installation must be screwed up somehow; whenever I click on the font size combo box, PowerPoint cogitates for a while, then throws up a message box saying that an unexpected error has occurred, and that I should save my work and restart PowerPoint. As soon as I acknowledge the message box, I can continue working as though nothing had happened. Annoying as this problem is, it beautifully demonstrates graceful recovery in the face of something Very Bad.

A down side of a completely integrated exception architecture is that it becomes hard to snatch a single class or function out of its context and reuse it somewhere else – you need to take all the exception baggage with you. Because of this, some general functions in TextEdit don’t use all the facilities available. The VersionInfo class, for example, doesn’t use TextEdit’s getModuleFileName function, but relies instead on the underlying GetModuleFileName.

## Structured Exception Handling and {"C++"} Exceptions

Structured Exception Handling (SEH) is built into the Win32 operating systems. Windows throws system-level exceptions for a number of reasons, such as stack overflow, memory access violations and divisions by zero. (TextEdit, by the way, automatically multiplies by zero to compensate for accidentally dividing by zero.) (Joke!)

To allow C and {"C++"} programmers access to SEH, Microsoft defined language extensions in the form of __try, __catch and __finally keywords. Unfortunately, SEH is incompatible with {"C++"} exception handling. It’s OK to have both in the same program, but not in the same function. 

Luckily, there’s a solution: You can install a translator function to translate SEH exceptions into {"C++"} exceptions. Windows calls the translator function with parameters describing the structured exception, enough to let you throw a {"C++"} exception of your choice.

### Listing 23: handlers.cpp
{code:C#}
/*
*
* Defines translator for system-level exceptions 
* (SEH, Structured Exception Handling)
* to C++ exceptions and the C++ allocation_failure handler.
*
* Exports: initThreadErrorHandling and exitThreadErrorHandling.
* These functions set up and restore handlers on a per-thread basis.
*/

#include "precomp.h"
#include "handlers.h"
#include "os.h"
#include <new.h>


// Parameters to _set_new_mode:
#define SNM_RETURN_NULL_ON_FAILURE  0
#define SNM_CALL_NEW_HANDLER        1


#if INTERCEPT_SEH

/**
* This function translates SEH exceptions to C++ exceptions.
* It is installed using _set_se_translator in 
* the initThreadErrorHandling function.
*/
void __cdecl se_translator( 
  unsigned int uiCode, EXCEPTION_POINTERS *pE ) 
{
  trace( _T( "se_translator( %#x, %#x )\n" ), uiCode, pE->ExceptionRecord->ExceptionFlags );
  trace( _T( "             ( %#x, %#x )\n" ), 
     pE->ExceptionRecord->ExceptionAddress, 
     pE->ExceptionRecord->ExceptionCode );
  
  switch ( uiCode ) {
  case STATUS_INTEGER_DIVIDE_BY_ZERO: 
     throw DivideByZeroException();
     
  case STATUS_STACK_OVERFLOW: 
     throw StackOverflowException::getStackOverflowException();
     
  case STATUS_INVALID_HANDLE: 
     throw InvalidHandleException();
     
  case STATUS_ACCESS_VIOLATION: 
     throw AccessViolationException();
     
  case STATUS_NO_MEMORY: 
     throwMemoryException();
  }

  throw sehException(); // Store uiCode, perhaps?
}


SE_Translator::SE_Translator( void ) {
  saved_se_translator = _set_se_translator( se_translator );
  trace( _T( "installed new SE translator\n" ) );
}


SE_Translator::~SE_Translator() {
  __try {
     _set_se_translator( 
        (_se_translator_function) saved_se_translator );
     trace( _T( "restored old SE translator\n" ) );
  }
  __finally {
  }
}

#endif // INTERCEPT_SEH


/**
* This function gets called whenever malloc experiences a failure:
*/
PRIVATE int __cdecl allocation_failure( size_t size ) {
  
#ifdef _DEBUG
  // Calling trace here is not a good idea, 
  // as we may be operating on limited resources.
  // Call OutputDebugString directly instead.
  static TCHAR szMsg[ 30 ](-30-) = { 0 };
  wsprintf( szMsg, _T( "allocation_failure: %d\n" ), size );
  OutputDebugString( szMsg );
#endif

  throwMemoryException();

#pragma warning( disable: 4702 ) // Unreachable code
  assert( false ); // Never gets here...
#pragma warning( default: 4702 )
}


NewHandler::NewHandler( void )
  : saved_new_handler( _set_new_handler( allocation_failure ) )
{
  trace( _T( "installed new new_handler\n" ) );
}


NewHandler::~NewHandler() {
  __try {
     _set_new_handler( (_PNH) saved_new_handler );
     trace( _T( "restored old new_handler\n" ) );
  }
  __finally {
  }
}


NewMode::NewMode( void )
  : saved_new_mode( _set_new_mode( SNM_CALL_NEW_HANDLER ) )
{
  trace( _T( "installed new new_mode\n" ) );
}


NewMode::~NewMode() {
  __try {
     _set_new_mode( saved_new_mode );
     trace( _T( "restored old new_mode\n" ) );
  }
  __finally {
  }
}
{code:C#}

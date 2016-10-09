/*
 * $Header: /Book/Exception.h 21    16.07.04 10:42 Oslph312 $
 *
 * Declares the Exception class and its descendants.
 */

#pragma once

#include <winerror.h>
#include "String.h"
#include "trace.h"


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
   WinException( 
      const String& strDescr, DWORD dwErr = GetLastError() );
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
      name( LPCTSTR pszDescr )                \
         : WinException( pszDescr, err ) {};  \
      name( const String& strDescr )          \
         : WinException( strDescr, err ) {};  \
      virtual String getDescr( void ) const { \
         return WinException::getDescr(); }   \
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


#define throwMemoryException() \
   throw MemoryException::getMemoryException()

// end of file

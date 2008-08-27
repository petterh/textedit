/*
 * $Header: /Book/AutoLibrary.h 3     3.06.02 10:44 Oslph312 $
 *
 * HINT: To see the HMODULE wrapped by AutoLibrary during Visual C++
 * debugging, add the following line to the AUTOEXP.DAT file:
 * AutoLibrary=<_hLib>
 */

#pragma once

class AutoLibrary {
private:
   HMODULE _hLib;
   
public:
   explicit AutoLibrary( LPCTSTR pszLibName );
   ~AutoLibrary();
   operator HMODULE();
};

inline AutoLibrary::AutoLibrary( LPCTSTR pszLibName )
   : _hLib( LoadLibrary( pszLibName ) )
{
#if defined( _DEBUG ) && defined( trace )
   if ( 0 == _hLib ) {
      const DWORD win_error = GetLastError();
      trace( _T( "AutoLibrary ctor: LoadLibrary( \"%s\" ) failed with error code %d (%#x)" ),
         pszLibName, win_error, win_error );
   }
#endif
}


inline AutoLibrary::~AutoLibrary() {

   FreeLibrary( _hLib );
}


inline AutoLibrary::operator HMODULE() {
   return _hLib;
}

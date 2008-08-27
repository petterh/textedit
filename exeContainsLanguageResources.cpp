#include "precomp.h"
#include "exeContainsLanguageResources.h"
#include "VersionInfo.h"
#include "utils.h"

bool exeContainsLanguageResources( WORD language_id ) {

   const VersionInfo vi( getModuleHandle() );
   if ( vi.isValid() ) {
      const int nLanguages = vi.getLanguageCount();
      for ( int iLanguage = 0; iLanguage < nLanguages; ++iLanguage ) {
         const UINT uiLangID = vi.getLanguage( iLanguage );
         if ( PRIMARYLANGID( uiLangID ) == PRIMARYLANGID( language_id ) ) {
            return true; //*** FUNCTION EXIT POINT
         }
      }
   }

   return false; //*** FUNCTION EXIT POINT
}

// MAKELANGID

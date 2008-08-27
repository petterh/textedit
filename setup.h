/*
 * $Header: /Book/setup.h 8     5.09.99 13:07 Oslph312 $
 */

#pragma once

#include "String.h"
#include "Registry.h"


#define INSTALL_PATH   _T( "InstallPath" )
#define WIN_PATH _T( "Software\\Microsoft\\Windows\\CurrentVersion" )
#define UNINSTALL_PATH WIN_PATH _T( "\\Uninstall\\TextEdit" )
#define RUNONCE_PATH   WIN_PATH _T( "\\RunOnce" )

inline void setInstallPath( const String& str ) {
   Registry::setString(
      HKEY_LOCAL_MACHINE, _T( "" ), INSTALL_PATH, str.c_str() );
}


inline String getInstallPath( void ) {
   return Registry::getString(
      HKEY_LOCAL_MACHINE, _T( "" ), INSTALL_PATH );
}


void appendProgramName( String *pstrPath );

void setup( bool bSilent );

// end of file

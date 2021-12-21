/*
 * $Header: /Book/FileType.cpp 9     16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "FileType.h"
#include "Registry.h"
#include "formatMessage.h"
#include "utils.h"
#include "resource.h"


FileType::FileType( LPCTSTR pszExt, 
   bool isEditDefault, bool bEdit, bool bPrint )
   : m_strExtension( pszExt )
   , m_strClass( Registry::getString( HKEY_CLASSES_ROOT, pszExt ) )
   , m_isEditDefault( isEditDefault )
   , m_bEdit( bEdit )
   , m_bInclude( true )
   , m_bPrint( bPrint )
{
   const int nNumEntries = getNumFileTypes();
   for ( int iType = 0; iType < nNumEntries; ++iType ) {
      FileType *pOther = getFileType( iType );
      if ( pOther == this || !exists() ) {
         break; //*** LOOP EXIT POINT
      }
      if ( 0 == m_strClass.compare( pOther->m_strClass ) ) {
         pOther->m_strExtension += _T( " " );
         pOther->m_strExtension += m_strExtension.substr( 1 );
         m_strExtension.erase();
         m_strClass    .erase();
      }
   }
}


String FileType::sm_strCommand;

FileType FileType::sm_aFileType[] = {
   //               isEditDefault  bEdit  bPrint
   FileType( _T( ".txt"  ), true , true , true  ),
   FileType( _T( ".bat"  ), false, true , true  ),
   FileType( _T( ".cmd"  ), false, true , true  ),
   FileType( _T( ".ini"  ), true , true , true  ),
   FileType( _T( ".inf"  ), false, true , true  ),
   FileType( _T( ".asp"  ), false, false, false ),
   FileType( _T( ".jsp"  ), false, false, false ),
   FileType( _T( ".htm"  ), false, false, false ),
   FileType( _T( ".html" ), false, false, false ),
   FileType( _T( ".xml"  ), false, true , true  ),
   FileType( _T( ".xsl"  ), false, true , true  ),
   FileType( _T( ".css"  ), true , true , true  ),
   FileType( _T( ".log"  ), true , true , true  ),
// FileType( _T( ".cpp"  ), true , true , true  ),
   FileType( _T( ".cxx"  ), true , true , true  ),
   FileType( _T( ".c"    ), true , true , true  ),
   FileType( _T( ".cpp"  ), true , true , true  ),
   FileType( _T( ".cs"   ), true , true , true  ),
   FileType( _T( ".h"    ), true , true , true  ),
   FileType( _T( ".hxx"  ), true , true , true  ),
   FileType( _T( ".hpp"  ), true , true , true  ),
   FileType( _T( ".java" ), true , true , true  ),
   FileType( _T( ".reg"  ), false, true , true  ),
   FileType( _T( ".asc"  ), true , true , true  ),
   FileType( _T( ".asm"  ), true , true , true  ),
};


int FileType::getNumFileTypes( void ) {

   return dim( sm_aFileType );
}


FileType *FileType::getFileType( int nIndex ) {

   assert( 0 <= nIndex && nIndex < dim( sm_aFileType ) );
   return &sm_aFileType[ nIndex ];
}


bool FileType::exists( void ) const {

   return !m_strClass.empty();
}


void FileType::include( LPCTSTR pszExt, bool bInclude ) {

   for ( int iType = 0; iType < dim( sm_aFileType ); ++iType ) {
      const LPCTSTR pszThis = sm_aFileType[ iType ].getExtension();
      if ( 0 == _tcsicmp( pszThis + 1, pszExt ) ) {
         sm_aFileType[ iType ].m_bInclude = bInclude;
         break; //*** LOOP EXIT POINT
      }
   }
}


void FileType::setCommand( const String& strCommand ) {

   sm_strCommand = strCommand;
}


#define SHELL_ROOT       _T( "%1\\shell" )
#define SHELL_PATH       SHELL_ROOT _T( "\\%2" )
#define EDITWITHTEXTEDIT _T( "EditWithTextEdit" )


void FileType::setCommand( 
   LPCTSTR pszPath, LPCTSTR pszFmt, LPCTSTR pszName )
{
   assert( !sm_strCommand.empty() );
   const LPCTSTR pszClass = getClass();
   String strPath =
      formatMessage( SHELL_PATH, pszClass, pszPath );
   const String strCurrent = Registry::getString( HKEY_CLASSES_ROOT,
      strPath.c_str(), _T( "" ), _T( "" ) );
   if ( strCurrent.empty() ) {
      if ( 0 == pszName || 0 == _tcslen( pszName ) ) {
         Registry::deleteEntry(
            HKEY_CLASSES_ROOT, strPath.c_str(), _T( "" ) );
      } else {
         Registry::setString( 
            HKEY_CLASSES_ROOT, strPath.c_str(), _T( "" ), pszName );
      }
   }
   strPath += _T( "\\command" );
   Registry::setString( HKEY_CLASSES_ROOT, strPath.c_str(),
      _T( "" ), pszFmt, sm_strCommand.c_str() );
}

void FileType::removeCommand( LPCTSTR pszPath, LPCTSTR pszFmt ) {
   
   assert( !sm_strCommand.empty() );
   const LPCTSTR pszClass = getClass();
   String strPath = formatMessage( 
      SHELL_PATH _T( "\\command" ), pszClass, pszPath );
   const String strCurrent = Registry::getString( HKEY_CLASSES_ROOT,
      strPath.c_str(), _T( "" ) );
   if ( strCurrent.find( sm_strCommand ) < strCurrent.length() ) {
      Registry::setString( HKEY_CLASSES_ROOT, strPath.c_str(),
         _T( "" ), pszFmt, _T( "NOTEPAD.EXE" ) );
   }

#ifdef trace
   trace( _T( "%s\n" ), strCurrent.c_str() );
#endif
}


void FileType::registerType( void ) {
   
   if ( shouldInclude() ) {
      if ( isEditDefault() ) {
         setCommand( _T( "open" ), _T( "\"%1\" \"%%1\"" ) );
      } else if ( useEdit() ) {
         setCommand( _T( "edit" ), _T( "\"%1\" \"%%1\"" ) );
      }
      if ( shouldPrint() ) {
         setCommand( _T( "print" ), _T( "\"%1\" -p \"%%1\"" ) );
         setCommand( _T( "printto" ), 
            _T( "\"%1\" -pt \"%%1\" \"%%2\" \"%%3\" \"%%4\"" ) );
      }
   } else {
      unregisterType();
   }

   const String strMenuCommand = loadString( IDS_EDIT_WITH_TEXTEDIT );
   setCommand( EDITWITHTEXTEDIT, 
      _T( "\"%1\" \"%%1\"" ), strMenuCommand.c_str() );
}


void FileType::unregisterType( void ) {

   
   String strPath = 
      formatMessage( SHELL_PATH, getClass(), EDITWITHTEXTEDIT );
   Registry::deleteEntry( HKEY_CLASSES_ROOT, strPath.c_str() );

   strPath = 
      formatMessage( SHELL_ROOT, getClass(), EDITWITHTEXTEDIT );
   const String strCurrent = Registry::getString( HKEY_CLASSES_ROOT,
      strPath.c_str(), _T( "" ) );
   if ( strCurrent.find( EDITWITHTEXTEDIT ) < strCurrent.length() ) {
      Registry::deleteEntry( 
         HKEY_CLASSES_ROOT, strPath.c_str(), _T( "" ) );
   }

   removeCommand( _T( "open" ), _T( "\"%1\" \"%%1\"" ) );
   removeCommand( _T( "edit" ), _T( "\"%1\" \"%%1\"" ) );
   removeCommand( _T( "print" ), _T( "\"%1\" -p \"%%1\"" ) );
   removeCommand( _T( "printto" ), 
      _T( "\"%1\" -pt \"%%1\" \"%%2\" \"%%3\" \"%%4\"" ) );
}

// end of file

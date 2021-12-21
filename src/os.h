/*
 * $Header: /Book/os.h 6     5.09.99 13:07 Oslph312 $
 * 
 * Encapculates differences between operating systems.
 */

#pragma once

DWORD getGoodIOBufferSize( void );
HRESULT coInitialize( void );
bool getCompressedFileSize( LPCTSTR pszFile, DWORD *pdwSize );
bool setThreadLocale( LCID locale );
bool isTextUnicode( CONST LPVOID lpBuffer, int cb, LPINT lpi );
bool isDebuggerPresent( void );

// end of file

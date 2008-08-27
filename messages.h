/*
 * $Header: /Book/messages.h 4     5.09.99 13:07 Oslph312 $
 *
 * LATER: Broadcast font changes to all instances.
 * This is not used for anything at the moment.
 */

#pragma once

#define TEM_FONTCHANGE (WM_APP + 1)

void broadcast( UINT msg, WPARAM wParam, LPARAM lParam );

// end of file

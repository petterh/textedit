### Programming Industrial Strength Windows
[« Previous: Child Windows](Chapter-8-Child-Windows.md) — [Next: Customization and Persistence »](Chapter-10-Customization-and-Persistence.md)
# Chapter 9: The Main Window

The **mainWndProc** function (in mainwnd.cpp) implements the window function for TextEdit’s main window, and is the central switchboard of TextEdit. To do the switching, it employs the message cracker macros in windowsx.h. Even though mainWndProc contains the big switch statement of traditional window functions, the {"HANDLE_MSG"} macro delegates each message to an appropriate handler function in a – sort of – type-safe manner. At least the programmer doesn’t need to worry about the parameter packing for the various messages. Furthermore, the macros are portable between Win16 and Win32. This is particularly important for messages such as {"WM_COMMAND"}, where the parameter packing changed.

## Handling {"WM_COMMAND"} messages

The onCommand handler function is a switchboard within a switchboard. In the same way that mainWndProc farms out message handling to individual functions, onCommand farms out command handling to individual functions. The only exception is the range of commands {"ID_MRU_1"} through {"ID_MRU_9"}, which is handled directly in onCommand. The command handlers could have been extended to handle ranges of commands, of course, but it hardly seems worth the bother.

One surprise is the existence of both {"ID_EDIT_FINDNEXT"}, which is on the menu, and {"ID_ACCEL_FINDNEXT"}, which is the command sent by the accelerator key F3. Yet both commands map to the onEditFindNext function. How come?

The reason is that the menu command “Find Next” is disabled when no previous search exists. If F3 mapped to {"ID_EDIT_FINDNEXT"}, this accelerator key would also be disabled. I do want it to do something, though: behave as though {"ID_EDIT_FIND"} had been selected. This is handled easily enough in onEditFindNext; the whole point of using two command IDs is to avoid disabling the accelerator merely because the menu item is disabled.


Some command handlers are pure debug scaffolding: onDivideByZero, onAccessViolation, onOutOfMemory and onStackOverflow. Their only purpose is to exercise the exception handling; they are excluded from release builds.

TestClass is used to verify proper stack unwinding when an exception is thrown; all it does is trace its construction and its destruction:

```C++
class TestClass {
public:
   TestClass () { trace( _T( "TestClass ctor\n" ) ); }
   ~TestClass() { trace( _T( "TestClass dtor\n" ) ); }
};

PRIVATE void onAccessViolation( HWND ) {
   
   TestClass testClass;

   lstrcpy( 0, _T( "uh-oh!" ) ); // No exception 
   _tcscpy( 0, _T( "uh-oh!" ) ); // Exception
}
```
My first implementation of onAccessViolation used lstrcpy to force an access violation. This failed though, in the sense that it didn’t fail. The access violation is caught in the bowels of lstrcpy, which just fails quietly. This kind of “error handling” is an abomination; while it certainly protects the program from crashing, it also protects the programmer from noticing what is certain to be a bug in the program. It certainly doesn’t protect the program from working incorrectly.

## The Clipboard User Interface

The implementation of the clipboard commands – cut, copy and paste – is straightforward enough. Representing them properly in the user interface is a little trickier:

* Copy should be enabled iff text is selected.
* Cut should be enabled iff text is selected and the file is writeable.
* Paste should be enabled iff the clipboard contains text and the file is writeable.

(Iff, by the way, is shorthand for “if and only if.”) The delete command is grouped with the clipboard commands on the Edit menu; it should be enabled iff the file is writeable and non-empty.

Enabling and disabling menu items is easy. We receive a {"WM_INITMENU"} message just before the menu opens, and can enable and disable to our heart’s content. The toolbar is another matter. Since it is always visible, we must enable and disable buttons synchronously in response to outside events.

Enabling and disabling of the cut and copy commands is done in Editor::onPosChange, which is called whenever something happens in the edit control, including when the selection changes.

To properly enable and disable the paste command, the TextEdit main window hooks itself into the clipboard viewer chain. Each member of the clipboard viewer chain is responsible for holding the next viewer in the chain; if any application fails to uphold this responsibility, the viewer chain collapses. This design stems from the early days of Windows and is not very robust. Today, this would probably (?) have been designed differently, as it is really asking for trouble.

At any rate, mainwnd.cpp has a static variable {"s_hwndNextClipboardViewer"} to do the job; it is initialized in onCreate:

```C++
s_hwndNextClipboardViewer = SetClipboardViewer( hwnd );
```
The window is unhooked from the chain again in onDestroy:

```C++
ChangeClipboardChain( hwnd, s_hwndNextClipboardViewer );
```
Once registered as a clipboard viewer, the window receives {"WM_DRAWCLIPBOARD"} messages whenever the clipboard contents change:

```C++
PRIVATE void onDrawClipboard( HWND hwnd ) {

   if ( IsWindow( s_hwndNextClipboardViewer ) ) {
      FORWARD_WM_DRAWCLIPBOARD( s_hwndNextClipboardViewer, SNDMSG );
   }
   enablePaste( hwnd );
}
```
The **enablePaste** function is responsible for actually enabling and disabling the paste command. Its implementation is trivial; the trick lies in calling it at the right time. The onDrawClipboard function takes care of this.

The hairiest part of clipboard viewerhood is handling someone else’s unhooking from the chain. If the window just below us in the chain – {"s_hwndNextClipboardViewer"} – is bailing out, we must update the {"s_hwndNextClipboardViewer"} variable with the next window down the chain. If not, we just forward the {"WM_CHANGECBCHAIN"} message to {"s_hwndNextClipboardViewer"}:

```C++
PRIVATE void onChangeCBChain(
	HWND hwnd, HWND hwndRemove, HWND hwndNext ) 
{
   if ( s_hwndNextClipboardViewer == hwndRemove ) {
      s_hwndNextClipboardViewer = hwndNext;
   } else {
      FORWARD_WM_CHANGECBCHAIN( s_hwndNextClipboardViewer, 
         hwndRemove, hwndNext, SNDMSG );
   }
}
```
An interesting experiment is to remove the forwarding of {"WM_DRAWCLIPBOARD"}, then start the clipboard viewer, then start TextEdit. The clipboard viewer is now essentially blind to changes of the clipboard contents! As I said, the design of the clipboard viewer chain is asking for trouble.

## Persistence in the Main Window

TextEdit stores information about the position and size of the window used to edit a given file. That way, we can restore the window to the same position and size the next time that file is edited. The Editor::saveState method is responsible for – you guessed it – saving the state.

Many applications save state information of one kind or another, and a common approach is to save the state when the application exits. If the application (or the whole system) crashes, the state information is, unfortunately, lost. 

TextEdit saves state information for the window whenever said state information has changed. When that happens, we get {"WM_SIZE"}, {"WM_MOVE"} or {"WM_SYSCOMMAND"} messages. The natural thing to do would be to call saveState in response to these messages, but with full-window dragging, this can slow things down so much that the dragging gets noticeably more jerky. To avoid this problem, we start a half-second timer instead, and call saveState in response to the firing of the timer. The timer is reset on every new state-changing message, so state is only saved when the user stops dragging the window or pauses for more than half a second.

Why, I hear you ask, not use {"WM_EXITSIZEMOVE"} instead of this timer nonsense? This is due to an implementation oddity in Windows. Perhaps I’m even justified in calling it a bug, as I’m fairly sure it was unintentional: If you select one of the window tiling commands from the task bar context menu, {"WM_EXITSIZEMOVE"} is never sent. I know this to be true under Windows NT 4.0; it may not be true on all versions of Windows. That’s irrelevant in any case; as long as one supported platform misbehaves, we need to handle the problem.

A final detail about full-window dragging: This can cause unsightly flashing of the caret in the edit window. To avoid this, the caret is hidden in response to {"WM_ENTERSIZEMOVE"} and shown again in response to {"WM_EXITSIZEMOVE"}. The problem noted above doesn’t apply; when you move or size the window, both {"WM_ENTERSIZEMOVE"} and {"WM_EXITSIZEMOVE"} are sent.

Chapter 10 explains how to create persistent variables using the registry. The Document class allows you to create persistent variables on a per-document basis. Window position and window size, for example, are both stored per document.

## Drag and Drop

To support file drag and drop, a window must do three things:

# Call DragAcceptFiles( hwnd, TRUE ) at startup time
# Call DragAcceptFiles( hwnd, FALSE ) at destruction time
# Handle the {"WM_DROPFILES"} message.

Handling the message is a matter of calling DragQueryFile (perhaps several times), then calling DragFinish. Since TextEdit is an SDI application, we must settle the question of how to handle dropping of multiple files – we obviously can’t open them all in the same application instance. TextEdit solves this in a manner similar to how it opens multiple files on the command line: It takes the first file for itself, and sends the rest to startInstance:

```C++
PRIVATE void onDropFiles( HWND hwnd, HDROP hdrop ) {
   const UINT DRAGQUERY_NUMFILES = (UINT) -1;
   const int nFiles = DragQueryFile( hdrop, DRAGQUERY_NUMFILES, 0, 0 );
   for ( int iFile = nFiles - 1; 0 <= iFile; --iFile ) {
      PATHNAME szDragFile = { 0 };
      verify( 0 < DragQueryFile( hdrop, iFile, szDragFile, dim( szDragFile ) ) );
      if ( 0 == iFile ) {
         Editor *pEditor = getEditor( hwnd );
         pEditor->saveIfNecessary();
         pEditor->openFile( szDragFile );
      } else {
         const String strArg = formatMessage( _T( "\"%1\"" ), szDragFile );
         startInstance( strArg );
      }
   }
   DragFinish( hdrop );
}
```

## Menu Management

TextEdit handles two messages that properly fall under the heading of menu management: {"WM_INITMENU"} and {"WM_MENUSELECT"}. 

The {"WM_INITMENU"} message is sent just before a menu drops down, and is intended to let you adjust menu items – enabling, disabling, checking and unchecking are typical chores. Since drop-down menus aren’t visible all the time, it is sufficient to do this just before the menu opens, and usually easier, too. The toolbar is more of a problem. It is always visible, and demands immediate gratification.

In mainwnd.cpp, {"WM_INITMENU"} is handled by onInitMenu. Aside from enabling and disabling this and that, it also refreshes the MRU (Most Recently Used) file list. The {"WM_INITMENU"} message is sent for popup menus as well as for window menus, so the exact same function serves to append the MRU file list to the toolbar’s File Open popup menu.

The {"WM_MENUSELECT"} message is sent when a menu or menu item is highlighted. Note that the {"HANDLE_WM_MENUSELECT"} macro in windowsx.h has a bug – it doesn’t handle the {"MENU_CLOSING"} case correctly. For that reason, the macro is redefined in **mainwnd.cpp** (this time correctly).

In mainwnd.cpp, {"WM_MENUSELECT"} is handled by **onMenuSelect**. The message is handled to update the status bar with a description of the currently highlighted menu item. The Windows function MenuHelp can be used to take the drudgery out of this, but TextEdit doesn’t use it. In the first place, MenuHelp can’t display text for popup menus; in the second place, it doesn’t give you much control over the string to display.

Popup menus don’t, unfortunately, have useful IDs, so when a popup menu comes along, you must somehow figure out which menu it is. The most obvious and simple way to do this is to check the first menu item on the menu. If the first menu item is {"ID_FILE_NEW"}, for example, you have hold of the File menu. The problem with this approach is that it breaks if you insert a new item at the top of a menu, and it doesn’t handle cascading menus without extra work.

TextEdit uses a different approach; it checks to see if the menu contains – anywhere – a given menu item. This approach is more robust than the first one. Both approaches can run into problems if a menu hierarchy has duplicate entries of the key IDs. One may hope that Microsoft adds IDs to popup menus some day; this feature has always been present in OS/2’s Presentation Manager.

The Editor class has two helper functions that produce menu description strings. One is getMenuDescription, which handles submenus; the other is getMenuItemDescription, which handles menu items. The latter tries its best to be clever, and changes the description depending on whether an item is enabled or disabled, what the current file name is or what the current selection is. This requires some hand coding, but the effect in the user interface is nice.

The menuUtils module defines utility functions for use with menus and menu items. For the most part, these are simple wrappers for Windows functions. The only exception is containsMenuItem, which checks whether a menu or any of its submenus contains a command with a given ID.

< Listing 36: menuUtils.h>
< Listing 38: menuUtils.cpp>

[Sidebar: Messy Menus, Accelerator Alignment](Sidebar-Messy-Menus.md)

## Communication Between TextEdit Instances

Different instances of TextEdit communicate using the {"WM_APP"} message. During startup, TextEdit checks to see if an already running instance has a given file loaded; it does this using {"WM_APP"}. (see the **activateOldInstance** module). The lParam is a global atom representing the file name. {"WM_APP"} returns 1 if the atom represents the currently loaded file, otherwise 0.

## Changing User Settings

Windows broadcasts {"WM_SYSCOLORCHANGE"} or {"WM_SETTINGCHANGE"} messages whenever the user changes the customization of the Windows GUI. Handling {"WM_SYSCOLORCHANGE"} is straightforward; {"WM_SETTINGCHANGE"} is more complex. The language may have changed, so the menu bar must be redrawn. Font sizes may have changed; this may require recalculating the layout. The status bar uses the currently defined menu font (see the MenuFont class), and the toolbar uses large icons if the size of the menu font is above a certain threshold. 

Many applications, such as the Windows Explorer, don’t change the font in the status bar to reflect changes in the user’s preferences. I find this rather strange.

Even TextEdit doesn’t change the font in the dialogs, sad to say.

## Notifications

The main window does not handle {"WM_NOTIFY"}. Yet, it does have a toolbar, complete with ToolTips and drop-down menu. Both these features are handled through {"WM_NOTIFY"}, so what’s going on here?

The toolbar subclasses the main window, that’s what’s going on. It does this a) in order to spy on {"WM_SIZE"} messages, and b) in order to steal the {"WM_NOTIFY"} messages. That way, more functionality is encapsulated within the toolbar itself.

## The Editor Connection

The main window’s reference to the Editor object is stored as a window long. The registration of the main window class reserves {"MAINWND_EXTRABYTES"} extra bytes of storage for each instance.

Given the handle to the main window, the getEditor function retrieves the corresponding Editor object.

< Listing 38: mainwnd.h>
< Listing 39: mainwnd.cpp>

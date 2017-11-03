### Programming Industrial Strength Windows
[« Previous: Designing for Programmers](Chapter-3-—-Designing-for-Programmers) — [Next: The Bare Bones »](Chapter-5-—-The-Bare-Bones)
# Chapter 4: The Mechanics of Subclassing

In an ideal world, at least for C++ programmers, Windows would be C++ throughout. Subclassing a window would be a matter of subclassing a C++ class, creating a window would be a matter of invoking the window’s constructor, message handling would be a matter of overriding virtual functions.

Unfortunately, Windows is not an ideal world for C++ programmers; the world turns out to be made of HWNDs rather than classes. There are two distinct paths to follow: Windows-type subclassing chains and polymorphic inheritance in the OO sense. We shall meet the challenge of creating a C++ wrapper for the HWND towards the end of this chapter. First, let’s look at subclassing from a Windows (i.e., plain C) point of view, since this forms the foundation for everything else.

## Subclassing Defined

Window subclassing is subversion, pure and simple. When you subclass a window, you replace its window function with your own, and the original gets to see messages only at your whim. Usually you want to be good about this, and interfere no more than necessary. Otherwise, the whole exercise is pointless; if you greedily keep all messages to yourself, you would have been just as well off developing a new window class from scratch.

There are many reasons for subclassing. The most obvious reason is that you want to modify the behavior of existing windows, as I do in this chapter. Less obvious, perhaps, is subclassing in order to listen in on the message traffic. A toolbar might subclass its parent, for example, in order to listen for WM_SIZE messages. The TextEdit toolbar subclasses the main TextEdit window for this reason, and to let it catch its own notification messages. This makes it more of a self-contained widget.

The ToolTip control will subclass its parent if you ask it to, in order to spy on its mouse messages. This saves your window from having to forward all mouse messages to the ToolTip window. 

Subclassing comes in several variants: Instance subclassing (a.k.a. local subclassing), global subclassing and class cloning (a.k.a. superclassing).

## Instance subclassing

**Instance subclassing** means to replace the window function of an existing window. A typical example is the subclassing of a specific edit control in a dialog box – to filter out illegal characters, perhaps. This used to be a popular technique for creating numeric input fields before the advent of the ES_NUMBER window style.

When a dialog’s **WM_INITDIALOG** message handler is invoked, all the edit controls exist already. The InstSub program is a complete example of instance subclassing an edit control to create a numeric input field. The resulting dialog is depicted in Figure 4:

![](Chapter 4 — The Mechanics of Subclassing_Figure4.bmp)

**Figure 4: Instance Subclassing in Action.** The upper edit field accepts only digits.

Building the InstSub example is a two-step process:

# rc InstSub.rc
# cl InstSub.cpp InstSub.res user32.lib

In spite of its overwhelming simplicity, InstSub illustrates several points:

* The actual subclassing is performed in the **{"WM_INITDIALOG"}** handler, by calling **SetWindowLong** with the **{"GWL_WNDPROC"}** parameter. (It is more convenient to use the **SubclassWindow** macro defined in **windowsx.h**, but in this example, I want to expose everything.)
* The address of the old window function is stored in the static variable **{"s_savedEditWndProc"}**. This works well enough in this simple example, and will work as long as you subclass a single control, or a set of identical controls with identical subclassing histories. Real life is usually less accommodating – perhaps the controls are of different classes, or perhaps one of them was already subclassed by somebody else.
* The invocation of the old window function is done through **CallWindowProc**. Don’t call **{"s_savedEditWndProc"}** directly; it may not be a function address! This happens under Windows NT if you subclass a Unicode window with a non-Unicode window function or vice versa. In such cases, the system must translate the parameters of text-related messages, and the alleged window function is actually a pointer to some black-box data structure cooked up by SetWindowLong.
* This point has nothing to do with subclassing, but it is nevertheless worth noting: Even though the sample dialog has no buttons, the Enter and Escape keys still serve up WM_COMMAND messages with IDs IDOK and IDCANCEL, respectively. Starting in Chapter13, I’ll have more to say about dialog boxes and the dialog keyboard interface.
* Final point –newEditWndProc is simpleminded. It fulfills the goal of allowing only numeric input, but in so doing, it disables accelerators such as Ctrl-C and Ctrl-Z.

InstSub is the simplest working example I can think of; it is far from complete. Consider the clipboard accelerators: If you change the code to let them by without further ado, there’s nothing to stop the user from pasting non-digits into the edit control. And, believe me, some users will. Me, for example.

**Listing 5: InstSub.rc** (used for all three subclassing examples)

{code:C#}
// InstSub.rc – used for all three subclassing examples

#include <windows.h>

#define IDC_NUMBERS 1000

InstSubDlg DIALOG DISCARDABLE  0, 0, 139, 46
STYLE DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Instance Subclassing"
FONT 8, "MS Sans Serif"
{
   LTEXT    "&Numbers:",-1,7,9,31,8
   EDITTEXT IDC_NUMBERS,44,7,88,14,ES_AUTOHSCROLL | WS_GROUP
   LTEXT    "&Anything:",-1,7,27,30,8
   EDITTEXT -1,44,25,88,14,ES_AUTOHSCROLL | WS_GROUP
}


GlobSubDlg DIALOG DISCARDABLE  0, 0, 139, 46
STYLE DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Global Subclassing"
FONT 8, "MS Sans Serif"
{
   LTEXT    "&Numbers:",-1,7,9,31,8
   EDITTEXT IDC_NUMBERS,44,7,88,14,ES_AUTOHSCROLL | WS_GROUP
   LTEXT    "&Anything:",-1,7,27,30,8
   EDITTEXT -1,44,25,88,14,ES_AUTOHSCROLL | WS_GROUP
}

CloneDlg DIALOG DISCARDABLE  0, 0, 139, 46
STYLE DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Class Cloning"
FONT 8, "MS Sans Serif"
{
   LTEXT    "&Numbers:",-1,7,9,31,8
   CONTROL  "",IDC_NUMBERS,"NumericEdit", 
            WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | WS_GROUP,
            44,7,88,14
   LTEXT    "&Anything:",-1,7,27,30,8
   EDITTEXT -1,44,25,88,14,ES_AUTOHSCROLL | WS_GROUP
}
{code:C#}

**Listing 6: InstSub.cpp**

{code:C#}
#include <windows.h>

#define IDC_NUMBERS 1000

static WNDPROC s_savedEditWndProc;

static LRESULT CALLBACK newEditWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   case WM_CHAR:
      if ( wParam < '0' || '9' < wParam ) {
         return 0; //*** EAT THE KEYSTROKE!
      }
      break;

   case WM_NCDESTROY:
      SetWindowLong( hwnd, GWL_WNDPROC, (LONG) s_savedEditWndProc );
      break;
   }

   return CallWindowProc( 
      s_savedEditWndProc, hwnd, msg, wParam, lParam );
}

static BOOL CALLBACK dlgFunc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   case WM_INITDIALOG:
      s_savedEditWndProc = (WNDPROC) SetWindowLong( 
         GetDlgItem( hwnd, IDC_NUMBERS ), 
         GWL_WNDPROC, (LONG) newEditWndProc );
      break;
   case WM_COMMAND:
      if ( HIWORD( wParam ) == BN_CLICKED ) {
         EndDialog( hwnd, 0 );
      }
      break;
   }
   return FALSE;
}

int APIENTRY WinMain( HINSTANCE hinst, HINSTANCE, LPSTR, int ) {

   return DialogBox( hinst, "InstSubDlg" , HWND_DESKTOP, dlgFunc );
}
{code:C#}

## Global Subclassing

**Global subclassing** means changing elements of a window class structure using SetClassLong. In the GlobSub sample, we replace the window function. A new window’s window function is initialized from the class structure when the window is created. Accordingly, existing windows are unaffected by global subclassing, but all new windows of that particular class are affected. 

The GlobSub example in Listing 7 uses the same resource file as the InstSub example.

To set the class window function, we need an instance of the class. WinMain creates an edit control for that purpose, and discards it afterwards.

The dialog box function no longer handles WM_INITDIALOG. The numeric input edit control is nevertheless subclassed, and so is the other edit control. In fact, all edit controls are affected. The call to GetOpenFileName is a final demonstration of the power of global subclassing; it proves that our subversion of edit controls works fine even for dialogs we don’t create ourselves.

**Listing 7: GlobSub.cpp**

{code:C#}
#include <windows.h>

#define IDC_NUMBERS 1000

static WNDPROC s_savedEditWndProc;

static LRESULT CALLBACK newEditWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   case WM_CHAR:
      if ( wParam < '0' || '9' < wParam ) {
         return 0; //*** EAT THE KEYSTROKE!
      }
      break;
   }

   return CallWindowProc( 
      s_savedEditWndProc, hwnd, msg, wParam, lParam );
}

static BOOL CALLBACK dlgFunc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   if ( WM_COMMAND == msg && HIWORD( wParam ) == BN_CLICKED ) {
      EndDialog( hwnd, 0 );
   }
   return FALSE;
}

int APIENTRY WinMain( HINSTANCE hinst, HINSTANCE, LPSTR, int ) {

   HWND hwndEdit = CreateWindow( "edit", "", 
      WS_POPUP, 0, 0, 0, 0, HWND_DESKTOP, (HMENU) 0, hinst, 0 );
   s_savedEditWndProc = (WNDPROC) SetClassLong( 
      hwndEdit, GCL_WNDPROC, (LONG) newEditWndProc );
   DestroyWindow( hwndEdit );

   DialogBox( hinst, "GlobSubDlg" , HWND_DESKTOP, dlgFunc );

   OPENFILENAME openFileName = {
      sizeof( OPENFILENAME ), HWND_DESKTOP, hinst,
   };
   GetOpenFileName( &openFileName );

   hwndEdit = CreateWindow( "edit", "", 
      WS_POPUP, 0, 0, 0, 0, HWND_DESKTOP, (HMENU) 0, hinst, 0 );
   SetClassLong( hwndEdit, GCL_WNDPROC, (LONG) s_savedEditWndProc );
   DestroyWindow( hwndEdit );

   return 0;
}
{code:C#}

## Class Cloning

**Class cloning** means to register a new window class based on an existing class. Functionally, it is a variant of global subclassing, but since you must register the cloned class under a new name, you have some control over which windows are subclassed. The Clone example clones the edit control to create a class named NumericEdit. In the resource file, I explicitly set the control class of the numeric entry field to NumericEdit rather than edit (see Listing 5). 

If we fail to register a class named NumericEdit before instantiating the dialog box, said dialog box will fail to load, and the DialogBox function will return -1. (This is the default behavior, which can be overridden by applying the **{"DS_NOFAILCREATE"}** dialog style.)

It isn’t mandatory to change the window function during class cloning. Sometimes you clone a class just to change the class style bits.

**Listing 8: Clone.cpp**

{code:C#}
#include <windows.h>

#define IDC_NUMBERS 1000

static WNDPROC s_savedEditWndProc;

static LRESULT CALLBACK newEditWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   case WM_CHAR:
      if ( wParam < '0' || '9' < wParam ) {
         return 0; //*** EAT THE KEYSTROKE!
      }
      break;
   }

   return CallWindowProc( 
      s_savedEditWndProc, hwnd, msg, wParam, lParam );
}

static BOOL CALLBACK dlgFunc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   if ( WM_COMMAND == msg && HIWORD( wParam ) == BN_CLICKED ) {
      EndDialog( hwnd, 0 );
   }
   return FALSE;
}

int APIENTRY WinMain( HINSTANCE hinst, HINSTANCE, LPSTR, int ) {

   WNDCLASS wndClass = { 0 };
   GetClassInfo( hinst, "edit", &wndClass );
   s_savedEditWndProc = wndClass.lpfnWndProc;
   wndClass.lpfnWndProc = newEditWndProc;
   wndClass.lpszClassName = "NumericEdit";
   RegisterClass( &wndClass );

   DialogBox( hinst, "CloneDlg", HWND_DESKTOP, dlgFunc );
}
{code:C#}

Thus endeth the subclassing tutorial. That’s all there is to it, really, except for various esoteric techniques used to subclass windows in other processes, which I won’t go into. 

The next section presents the code that TextEdit uses to accomplish instance and global subclassing. The code uses other classes that I haven’t described yet. For now, I’ll just use the String and Exception classes without further ado; I’ll go into more detail about them in Chapters [5](Chapter-5-—-The-Bare-Bones) and [6](Chapter-6-—-Exceptions).

## General Mechanism for Global Subclassing

It’s in the nature of global subclassing that there is only one old window function to store, so the approach used in GlobSub is actually fine, except that it is bothersome to create a window of the usurped class merely in order to call SetClassLong. The main purpose of GlobalSubclasser is to hide the bother.

Curiously, SetClassLong and GetClassLong are both prototyped as returning DWORDs, even though the dwNewLong argument to SetClassLong is prototyped as LONG, i.e., a signed value.

< Listing 9: GlobalSubclasser .h >
< Listing 10: GlobalSubclasser.cpp >

## General Mechanism for Instance Subclassing

The biggest problem with the InstSub example was the static storage of the old window function. A subclassing function that applies yellow backgrounds, for example, could conceivably be used both for edit controls and list boxes. Then you’d have to store two different pointers to two old window functions – rather difficult to do in a single static variable, and even if you used two, how would you know them apart? In a general solution, the pointer to the old window function must somehow be associated with the window handle itself. 

In a general solution, other properties are desirable as well:

* It should be possible to apply multiple subclassings to a window
* It should be possible to subclass and unsubclass in any order
* The programming interface to a general mechanism must be easy to use, protect against programming mistakes and be robust at runtime
* It is sometimes convenient to associate subclassing-specific data with a window; a general solution should offer this feature.

Another problem with InstSub is that it unsubclasses unconditionally. If somebody else had subclassed the window after InstSub did, that somebody else would be hosed. A careful solution should check whether the current window function is the expected one, and unhook only if it is.

## Associating the Old Window Function with the Window Handle

Data can be associated with windows in two basic ways: You can maintain a static list of associations (array, linked list, binary tree, hash table, post-it notes, engraved stone tablets) or you can associate the data with the window itself (using either window properties or SetWindowLong).

If you were to use SetWindowLong, the **{"GWL_USERDATA"}** offset would be the only possible choice, given the requirement to subclass windows under somebody else’s control. This might be OK within a single application, but hardly in a generally reusable library. True, you could reinstate the old value of **{"GWL_USERDATA"}** before calling window functions down the chain, but you would have no defense against subclasssings above you in the chain, or against application programmers ignorant of this little implementation detail using **{"GWL_USERDATA"}** for their own purposes.

According to conventional wisdom, window properties are less efficient than window words. Benchmarking

{code:C#}
GetWindowLong( hwnd, GWL_USERDATA );
{code:C#}

against

{code:C#}
GetProp( hwnd, “test” ); 
{code:C#}

verifies this. When I used an atom instead of the string for the property name, however, the tables were turned. Now GetProp was actually faster than GetWindowLong, at least on a Windows NT 4.0 window with a single property. In truth, though, this is a rather moot point – I’ve implemented subclassing using all these techniques, and efficiency has never been a noticeable problem.

One solution to the problem of multiple subclassings is to store the old window function as a property, using a unique property name for each subclassing. With multiple subclassings, this suggests an image of saved window functions sticking out all over the window like pins from a pincushion. While I’ve used this porcupine technique successfully, it does have one disadvantage: the subclassings are isolated from one another, and unhooking must be done in reverse order of subclassing. To remedy this, the InstanceSubclasser maintains a linked list of subclassing descriptors:

{code:C#}
class Node {
public:
   WNDPROC m_wndProc;
    WNDPROC m_wndProcSaved;
    void    *m_pUserData;
    Node    *m_pNext;
    ...
};
{code:C#}

This allows subclassing and unsubclassing with gay abandon, provided you stick to using InstanceSubclasser. You’re still vulnerable to “foreign” subclassings, though, and no perfect solution exists to deal with this particular problem.

The head of the list can now be attached to the window using a single window property, or it can be maintained in a static table. 

One advantage of using static storage is encapsulation – window properties are exposed to the rest of the world, and it is within the realm of possibility that some Bad Person will steal our property. Another advantage is that we won’t leak window properties if something gets screwed up. One example of a screw-up is when somebody else subclasses after us, then fails to get out of the way before WM_DESTROY or WM_NCDESTROY. 

In spite of the above, InstanceSubclasser uses a window property to store the head of the subclassing list. Perhaps this is only old habit on my part, but it feels more “right” in some sense.

## Unhooking

The InstanceSubclasser class lets you unsubclass at any time, provided nobody else is in the way. A subclassing using InstanceSubclasser will unhook automatically on either WM_DESTROY or WM_NCDESTROY, again provided nobody else is in the way. Both of these messages are used for unhooking in common subclassing schemes; and by getting out of the way at WM_DESTROY time, we allow subclassings below us to unhook on WM_DESTROY, if they so desire. If we have a subclassing above us that doesn’t unhook until WM_NCDESTROY, our first attempt to unhook (on WM_DESTROY) will fail, but our second attempt (on WM_NCDESTROY) will succeed.

The tool tip doesn’t unhook at all, as far as I can tell, and thus blocks every prior subclassing from unhooking. For that reason, the Toolbar class most carefully lets the tool tip subclass before subclassing its edit child.

Sadly, there is no way to create a subclassing scheme that’s guaranteed to work under all circumstances. InstanceSubclasser covers 99% of what you’ll ever need, though.

< Listing 11: InstanceSubclasser.h >
< Listing 12: InstanceSubclasser.cpp >

## The Window Class: Wrapping the HWND

As I mentioned at the start of this chapter, creating a C++ wrapper for a HWND is a challenge. The ideal solution would hide the HWND completely; subclassing a window would be done by subclassing the C++ way and messages would be handled by overriding virtual functions. We do, however, face a number of obstacles. Here are a few things to consider:

* C++ member functions – methods – have an implicit “this” pointer, a pointer to the object instance. The Windows API, unfortunately, understands nothing of “this” pointers, so the callback function can’t be a member function. It must be static.
* The WNDPROC actually does have an explicit “this” pointer – the HWND parameter. Any solution must have some way of mapping HWNDs to instances of C++ classes. Vice versa too, of course, but that’s trivial.
* Windows has a huge number of predefined messages. Creating virtual functions for them all is expensive.
* Extending the base class – adding new virtual methods to deal with new Windows messages, for example – would require you to recompile the entire world. The Windows subclassing chain is much more dynamic, and requires nothing of the kind.
* The process of creating a window is unrelated to the C++ constructor. You can ask Windows to create a HWND for you, but it will be more difficult to persuade Windows to instantiate your corresponding C++ object when a Window is created outside your control.
* The process of destroying a window is unrelated to the C++ destructor. You can ask Windows to destroy a HWND for you, but you can’t, in general, invoke a C++ destructor from the WM_DESTROY handler. The C++ object must be able to exist without an attached HWND.
* You must deal with system-defined windows such as controls. If you don’t use any of those, you can create your own little OO world, but you will also throw away many of the benefits of using Windows in the first place. In such a universe, there would be only one window function in the traditional Windows sense.

Mapping a C++ object to a HWND is trivial; all you have to do is include a HWND member variable in the class definition. Going in the opposite direction, from a HWND to a C++ object, you have the same options that we had with InstanceSubclasser: Some kind of static storage, window words or window properties. 

(Actually, there is another option: You can dynamically generate a code stub that calls a member function, then install this code stub as the window function. Microsoft’s Active Template Library (ATL) uses a mechanism based on this elegant principle. The solution’s main drawback is that it is necessarily processor-dependent. As a curious side effect, all window instances have different window functions, even if they are of the same class.)
 
Microsoft Foundation Classes (MFC) maintains a mapping table between HWNDs and CWnds. Something like this is probably necessary, given that MFC attempts to hide the HWND completely. If you call CWnd::FromHandle, for example, you receive a pointer to a CWnd object. The function searches the mapping table, and if it finds the HWND, it returns the address of the corresponding CWnd object. If not, it creates a new CWnd object for you. Since you don’t know whence it came, you a) cannot delete it, and b) cannot store it for later use. MFC itself keeps track of which CWnds are temporary and deletes them on idle cycles. This is either weird and wonderful, or it’s just weird; take your pick.

The main problem with global mapping tables is something else, though. If you maintain a single global table, you need to provide thread-safe access, with the overhead that entails. If you use Thread Local Storage (TLS) to maintain one table per thread, as MFC does, you run into problems if you share CWnd objects between threads.

The TextEdit Window class uses (you guessed it) the InstanceSubclasser to do the subclassing. Since InstanceSubclasser allows us to store arbitrary data per subclassing per window, it gives us not only subclassing, but a reference to the C++ object as well.

All windows wrapped by Window objects are subclassed using the same window function, Window::wndProc. Derived classes may override any of the virtual functions defined in the Window class, including dispatch, which in one sense is the “real” window function. The whole point of wndProc is that we need a static function as our ambassador from C++-land to Windows; all wndProc really does is to get hold of the attached Window object so that we can call methods on it.

The set of messages selected for the virtual function treatment is rather arbitrary. To handle a new message, you have the choice between adding a new virtual method and overriding the dispatch method. The quest for overall simplicity and convenience should guide your choice.

The Window class defines default handlers for all messages, either through the virtual methods or through the dispatch method. These invariably pass the messages on to the original window function.

< Listing 13: Window.h >
< Listing 14: Window.cpp >

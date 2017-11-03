### Programming Industrial Strength Windows
[« Previous: Designing for Users](Chapter-2-Designing-for-Users) — [Next: The Mechanics of Subclassing »](Chapter-4-The-Mechanics-of-Subclassing)
# Chapter 3: Designing for Programmers

In the previous chapter, I discussed software usability from the user’s point of view. Programmers are users, too, albeit with a different worldview. Programmers, including yourself, are users of your source code. 

Is your source code usable? How easily can I extend your program, how easy are your APIs to use – correctly? Do your APIs encourage robust programming, or do they actively court programming mistakes? Do you remember what you did two years ago? Two months ago? I know I don’t.

This chapter examines a few issues that help make programmers’ lives easier; more are scattered throughout the rest of the book. I’ll also throw in a few comments on Java.

## Programming Guiding Principles

A guiding principle is a general rule to guide design decisions; it usually includes the rationale behind the rule. We already encountered a few guiding principles in the previous chapter, and you’ll find guiding principles for programming scattered through this book. They’re all formatted like this one:

**“Let the compiler catch the bugs.”**

This means compiling at the highest warning level available. It means defining STRICT before including any Windows header files. It means avoiding casts when you can (which is not all that often, in Windows programming). It means using little tricks such as putting the constant first in all comparisons. Consider this code fragment:

{code:C#}
if ( uiRetCode = IDOK ) {
   // ...
}
{code:C#}

This example contains a bug. The controlling expression has one equal sign instead of two, so it is a (perfectly legal) assignment, rather than the intended comparison. Moreover, IDOK being non-zero, the expression is constant and always evaluates to true – the controlled clause is always executed.

If you code the statement like this instead, the compiler will catch the bug:

{code:C#}
if ( IDOK = uiRetCode ) {
   // ...
}
{code:C#}

Actually, most compilers will warn about “assignment in comparison” on the first example. This warning is one reason why many programmers fail to use all the help the compiler can give them – they like writing assignments in comparisons, and disable the warning to avoid complaints about constructs such as this:

{code:C#}
if ( hdc = GetDC( hwnd ) ) {
   // ...
   ReleaseDC( hwnd, hdc );
}
{code:C#}

Don’t do it! The last time I was burned by an assignment in a comparison was in 1996. It cost me five hours of debugging, all of them unnecessary, and I vowed that it would never happen again. 

If you absolutely _must_ do the assignment on the fly, this formulation generates equivalent code on any decent compiler:

{code:C#}
if ( 0 != (hdc = GetDC( hwnd )) ) {
   // ...
   ReleaseDC( hwnd, hdc );
}
{code:C#}

Another reason to avoid assignments in comparisons is that it makes the code harder to read. Not unreadable, by any means, but harder. Furthermore, if you put each statement on a line by itself, the number of code lines gives a more accurate indication of the complexity of the code. Obfuscated coding may be fun, but it has no place in production code. Program as clearly as possible; there is no such thing as being too clear.

This is better, and easier to step through when debugging, because it allows you to change the value of hdc before reaching the test:

{code:C#}
hdc = GetDC( hwnd );
if ( 0 != hdc ) {
   // ...
   ReleaseDC( hwnd, hdc );
}
{code:C#}

The one place where an assignment in a comparison is defensible is in the controlling expression of a while statement. Lifting the assignment may require its duplication – once before the statement, and once at the end of the controlled body. This cure is worse than the disease; code duplication fits firmly in the Bad category.

## Getting Your Priorities Straight

What is most important in your current programming project? Should the program be as fast as possible? As small as possible? Does it have to be portable? 

The answers to these and related questions vary from project to project, and, indeed, from module to module and from function to function. You will, however, do well to consider the questions up front, and come up with a prioritized list of what’s important. Steve Maguire discusses this in his book Writing Solid Code. Experience shows that nobody can satisfy all requirements at the same time, which makes conscious prioritization all the more important.

Item	Definition
Correctness	The code works correctly. This item might seem superfluous, but experience tells me something else. Too often, I’ve seen programmers more concerned with speed than correctness.
Size	This does not refer to the number of source code lines, but to the total size of compiled code. It may also include run-time memory usage and overhead imposed by non-functional data such as strings used internally in the program.
Speed	This includes both execution speed (as measured by CPU usage) and perceived responsiveness from the user’s point of view. These are not necessarily the same.
A guideline is to make the code fast enough, but not to waste time making it faster than that. If you need to sort 5 records, choose bubble sort. If you are sorting a million records, choose Quicksort.
Speed bottlenecks are rarely obvious. Before you decide that an operation or a subsystem needs optimization, try to get hard data on where the real bottleneck is.
Robustness	Tolerance towards erroneous input and other error conditions. This does not mean that a program or routine should accept garbage, but that it should handle it gracefully. 
Safety	Choose the implementation that you are most likely to develop without bugs. Bubble sort is safer than Quicksort, but Quicksort is faster than bubble sort. Tradeoffs between speed and safety are common.
Testability	A program should be easy to test.
Maintainability	Code that is easy to maintain typically has several characteristics: 
•	It is easy to read and understand.
•	It is well encapsulated. This allows changes (updates or fixes) to be made with some confidence that it won’t blow up something else.
•	Documentation, including comments in the code, is in agreement with the code.
Reusability	This can mean class or function reuse in the same project, or it can mean preparing for reuse in later projects. Designing for reuse typically has an overhead of around 50%, split among additional design time (particularly finding good generalizations), additional documentation requirements and additional testing. 
A good compromise is often just to choose a design that does not preclude reuse. The best tool for this is known as encapsulation.

## Portability

The code is reusable across platforms. Coding for portability may entail 
* Using a cross-platform library
* Using a subset of a language or library that is common and consistent across platforms
* Isolating platform dependencies (see os.cpp).

To illustrate what I mean by testability, here’s a code fragment from getPathFromIdListA in utils.cpp:

{code:C#}
const BOOL bOK = SHGetPathFromIDListA( pidl, szAnsiPath );
if ( bOK ) {
   multiByteToWideChar( szAnsiPath, pwszPath, MAX_PATH );
}
{code:C#}

I could have saved one line and one variable as follows:

{code:C#}
if ( SHGetPathFromIDListA( pidl, szAnsiPath ) ) {
   MultiByteToWideChar( szAnsiPath, pwszPath, MAX_PATH );
}
{code:C#}

These two versions are functionally equivalent, but the first is easier to debug. You can see the value of bOK, and when you’re single stepping through the code, you can even change the value before the test. This provides you with an opportunity to cover both branches. (Actually, you may have to remove the const keyword to get the debugger’s permission to change the value.)

## Global Variables Considered Harmful

Global Variables Are Bad. We all learned that on our mothers’ knees. TextEdit has no global variables, although it has several that are “global” to a single compilation unit. 

A common strategy to avoid global variables is to create a structure containing all global data, create an instance of that structure at some point, and pass the structure to all functions requiring access to the globals.

This cure is worse than the disease; you still have fairly uncontrolled access to the variables, and additional parameters to boot. It helps to keep in mind why global variables are bad: It’s because they break encapsulation. 

## Assert Your Sanity

The assert macro evaluates its argument; if the expression evaluates to non-zero, it does nothing; if it evaluates to zero, it displays a diagnostic message and gives you a choice of whether to abort, debug or continue. The assert macro is grossly underused.

The whole idea behind the assert macro is to identify logical errors during program development. An assertion is a sanity check on your assumptions. Here’s an example from the ArgumentList class:

{code:C#}
inline LPCTSTR ArgumentList::getArg( int nArg ) const {
   ...
   assert( 0 <= nArg && nArg < m_argc );
   return m_argv[ nArg ](-nArg-);
}
{code:C#}

The m_argc member denotes the number of arguments present in the argument list; the assertion documents the assumption that we never call the getArg method with an argument that’s out of range. If that happens, there’s a bug in the program. With the assertion in place, you’re much more likely to catch that bug during development, and to do it in a way that lets you fix it cleanly and cheaply.

The assert macro is also a documentation tool. Consider the isWindowsNT function from **os.cpp**:

{code:C#}
bool isWindowsNT( void ) {

   OSVERSIONINFO osvi = { sizeof osvi };
   assert( 0 == offsetof( OSVERSIONINFO, dwOSVersionInfoSize ) );

   return GetVersionEx( &osvi ) && 
      VER_PLATFORM_WIN32_NT == osvi.dwPlatformId;
}
{code:C#}

The assertion serves to document what the initialization of the osvi variable is all about, namely initializing the size member, dwOSVersionInfoSize. It will also catch the problem whenever the size member turns out not to be the first member of the structure.

The assert macro is active only in debug code. Release builds define the NDEBUG preprocessor identifier, which causes assert to evaluate to nothing. In some cases, such as when creating a robust library, you may want to retain parameter validation code in release builds. You should still keep the assertions, though, since they alert you at run-time while debugging. It is much easier to catch and fix a live bug, when you can see the life-blood of data flowing through the veins of your code, than it is to do a post mortem on a dead program.

One common mistake is to put production code into the assert macro. For example:

{code:C#}
assert( FindClose( hFind ) );
{code:C#}

This documents that I expect FindClose to succeed, or that I see the likelihood of failure small enough that I won’t bother with error handling, or that there is little to do in the way of recovery. If the function fails, it’s probably because of a programming error (such as forgetting to call FindFirstFile), and the assertion will alert me to the problem.

Putting FindClose into the assert will work fine in a debug build. In a release build, however, FindClose won’t be called. If you want to combine assertion with release-build execution, use the verify macro instead, which evaluates its argument even when NDEBUG is defined:

{code:C#}
verify( FindClose( hFind ) );
{code:C#}

The assert macro is defined in the standard header file assert.h; the verify macro is defined in the TextEdit include file common.h. I’ll get back to this header file towards the end of Chapter 5.

## Constantly on Guard

The !const! keyword is as underused as the assert macro. When the const keyword modifies a variable, it says that the value of that variable may not be changed:

{code:C#}
const int i = 5;
i = 6; // Compilation error!
{code:C#}

The benefits of const are many:

* It serves to document the programmer’s intentions, i.e., “this variable is const, so it is not expected to change.”
* It lets the compiler object whenever you do violate the constness, either due to a programming mistake or due to faulty assumptions.
* When applied to parameters, const lets you pass references and pointers with full confidence that they will return unscathed.
* The const keyword is a hint to the compiler that the variable will not change. This hint can be used for optimization, or for placing variables in some form of read-only memory.
* The const keyword can be used instead of the preprocessor’s #define statement for manifest constants. This allows typed constants under full control of the compiler proper, an improvement over the mindless textual substitution performed by the preprocessor.

All kinds of data may be declared const, including variables that are class instances. Classes may also have methods declared const. If an object is declared const, it is illegal to call non-const methods on that object. Methods that are callable on const objects must themselves be declared const.

The lack of const is a shortcoming of the Java programming language. Java does have the final keyword, which means the same as const when applied to simple types, but not when applied to object references. (A reference, in Java as in C++, is a controlled pointer.) It’s the reference that’s final, not the object – Java does not have const objects in the sense that C++ does.

There are really two types of constness. There’s the strict language-lawyer interpretation, and there’s conceptual constness. Consider an object that tries to be smart: A polygon object with a getArea method might, in the interest of optimization, cache the results from the area calculation, storing the result in the private instance variable m_area. Thus, getArea will immediately return the value of m_area when it believes this to be valid; otherwise, it will perform the calculation and update the cache variable for later use.

What happens if you call getArea on a const polygon object? A violation of strict constness, is what happens, and the compiler will object strongly to the updating of m_area. To get around this, declare m_area to be mutable. This lets you assign a value to it even from a method declared const. 

The const_cast is another way of getting around this sort of thing. TextEdit uses this to get around parameters in the Windows API that should have been declared const, but aren’t.

To see conceptual constness from another angle, consider the TextEdit EditWnd class, which wraps a standard edit control. Its setText method does not change the C++ object, but it does change the wrapped HWND. Therefore, I’ve chosen to consider setText as non-const. Some programmers see this differently, and, again, there is no “right” answer.

## Use Destruction Constructively

The destructor of a C++ object is called when the object goes out of scope (if it was allocated on the stack) or when it is deleted (if it was allocated on the heap). The constructor/destructor sandwich is one of the most elegant parts of the C++ language; it lets objects clean up after themselves without explicitly coding for it at the point of use.

Java does not have destructors, relying instead on automatic garbage collection. This is fine as long as the only resource you deal with is memory; it’s less fine if an object allocates other resources, such as file handles, network connections and GDI objects. The lack of destructors is one of my two main gripes with the Java language definition, the other being the lack of a preprocessor.

Destructors are intimately connected to C++ exception handling, a topic I’ll return to in Chapter 6. When an exception propagates through the stack frames, each frame cleans up after itself by calling the destructors of all automatic (that is, stack-based) objects. This is known as “stack unwinding.”

What if a destructor throws an exception? If this happens during stack unwinding, as well it might, the program dies horribly. Accordingly, the following design principle is really an ironclad rule that you should never break if you can possibly help it:

**“Don’t throw exceptions from destructors.”**

## Automatic Pointers

Objects allocated on the stack are destroyed automatically. Not so with objects allocated on the heap; in their case you must invoke the delete operator, or, in the case of arrays of objects, the delete[]() operator.

Consider the **getWindowText** method from the **Window** class, which we’ll look at in Chapter 4:

{code:C#}
String Window::getWindowText( void ) const {

   const int nLength = GetWindowTextLength( *this );
   LPTSTR pszWindowText = new TCHAR[ nLength  1 ](-nLength--1-);    //x1
   GetWindowText( *this, pszWindowText, nLength  1 );
   const String strWindowText( pszWindowText );        //x2
   delete[]() pszWindowText;
   return strWindowText;                               //x3
}
{code:C#}

If the program follows the nominal execution path, this code works correctly. Problems appear when things begin to fail, though. The statements marked x1, x2 and x3 may throw exceptions:

* x1 will throw a memory exception if the allocation fails. This is not a problem, since there is nothing to clean up. 
* x3 invokes the String copy constructor, which, depending on the definition of the String class, may throw an exception for a variety of reasons. This is not a problem either, since we’ve already cleaned up by deleting the memory that pszWindowText points to. 
* x2 also invokes a String constructor, and this time we do have a problem. If the constructor throws an exception, we never get to delete the memory that pszWindowText points to.

In this example, the problem results in a “mere” memory leak. Sometimes it is much worse.

(The example assumes that an allocation failure in operator new throws an exception rather than returning a null pointer. Notwithstanding the C++ standard, we’re getting into compiler-dependent territory here, and I’ll return to the subject in Chapter 6.)

One way of handling such situations is to catch the exception, do whatever cleanup is necessary, and then rethrow the exception:

{code:C#}
String Window::getWindowText( void ) const {
   const int nLength = GetWindowTextLength( *this );
   LPTSTR pszWindowText = new TCHAR[ nLength  1 ](-nLength--1-);
   String strWindowText;
   try {
      GetWindowText( *this, pszWindowText, nLength  1 );
      strWindowText.assign( pszWindowText );
   }
   catch ( ... ) {
      delete[]() pszWindowText;
      throw; // pass exception to next handler in chain
   }
   delete[]() pszWindowText;
   return strWindowText;
}
{code:C#}

While this works correctly, it is verbose and difficult to read. Even worse, the clean-up code is duplicated, a guarantee for maintenance headaches. (Java has the edge on C++ when it comes to exception handling semantics; it allows a finally clause that is executed both during the normal flow of control and after an exception has been thrown.)

A better solution is to wrap the pointer to the heap-allocated memory in an object, making it a smart pointer. The essential part of a smart pointer class is its destructor, which (in simple cases) deletes the pointer that it wraps. (More complex cases may involve, say, reference counting.) 

{code:C#}
String Window::getWindowText( void ) const {
   const int nLength = GetWindowTextLength( *this );
   AutoString pszWindowText( nLength  1 );
   GetWindowText( *this, pszWindowText, nLength  1 );
   return pszWindowText;
}
{code:C#}

The result is considerably more readable than either of the previous examples. We get rid of the delete statement; this saves us one line. As a fringe benefit, we also get rid of the temporary String object. The return statement invokes the String(LPCTSTR) constructor, and this happens before the AutoString object goes out of scope. The important part, though, is that the destructor is called even if an exception is thrown, as part of stack unwinding:

The C++ Standard Template Library (STL) defines a smart pointer: a template class named auto_ptr. I don’t use this, relying instead on AutoPtr (in AutoPtr.h). Why create a replacement for a perfectly good standard template? The reason is that auto_ptr is not perfectly good. In particular, it lacks a conversion operator to the pointer type it wraps, forcing the tedious use of the get method instead. I also added the reset method.

Both auto_ptr and AutoPtr have one little problem with respect to the getWindowText example above – their destructors invoke delete rather than the required delete[](). For this reason, I created a variant called AutoArray, defined in AutoArray.h. This class is not completely general; it has exactly the functionality I needed for TextEdit, no more.

(I’m being a bit of a purist here, as the difference between delete and delete[]()() only becomes important in the case of arrays of objects. Before freeing the memory, delete[]()() invokes the destructor of each object in the array. Plain delete does not. Since the elements of an AutoString array are of simple type, using auto_ptr or AutoPtr would actually work fine on any reasonable compiler.)

<Listing 1: AutoPtr.h>

**Listing 2: AutoArray.h**

{code:C#}
/*
 * HINT: To see the string wrapped by AutoString during Visual C++
 * debugging, add the following line to the AUTOEXP.DAT file:
 * AutoArray<*>=m_ptr=<m_ptr>
 */

template< class T >
class AutoArray {
private:
   T *m_ptr;

public:
   explicit AutoArray( int n ) throw() : m_ptr( new T[ n ](-n-) ) {
      assert( 0 != m_ptr );
   }
   explicit AutoArray( T *_ptr = 0 ) throw() : m_ptr( _ptr ) {
   }
   ~AutoArray() {
      delete[]() m_ptr; // NOTE: It's OK to delete a null pointer.
      reset_pointer( m_ptr );
   }
   void alloc( int n ) {
      assert( 0 == m_ptr );
      m_ptr = new T[ n ](-n-);
   }
   T ** operator &() {
      return &m_ptr;
   }
   operator T *() throw() {
      return m_ptr;
   }
   operator const T *() const throw() {
      return m_ptr;
   }
   operator void *() throw() {
      return m_ptr;
   }
   operator const void *() const throw() {
      return m_ptr;
   }
};

typedef AutoArray< CHAR  > AutoStringA;
typedef AutoArray< WCHAR > AutoStringW;

#ifdef UNICODE
typedef AutoStringW AutoString;
#else
typedef AutoStringA AutoString;
#endif
{code:C#}

The problem addressed by AutoPtr and AutoArray is not limited to memory; it applies to other resources as well – file handles, display contexts, network connections, et cetera ad nauseam. I include two examples here: The AutoHandle class wraps a Win32 HANDLE, while the PaintStruct class wraps – you guessed it – a PAINTSTRUCT (or, rather, the sandwich-layer calls to BeginPaint and EndPaint that wrap the use of the PAINTSTRUCT). Such wrappers are usually trivial to implement.

To sum up: 

* The wrappers are a programming convenience, since you save the close/free/cleanup statement – the bottom layer of the sandwich. 
* They help make the code more robust, since you cannot forget to free the resource in question. 
* Most importantly, they make the wrapped resource exception-safe. 

Other examples of self-destructing wrapper classes are FileMapping (defined in FileMapping.h, described in Chapter 12), TemporaryStatusIcon (Statusbar.h) and ClientDC (ClientDC.h). In TextEdit, you will find many more.

< Listing 3: AutoHandle.h>
< Listing 4: PaintStruct.h>

## Programming Defensively

When you drive a car, you will have fewer accidents if you don’t assume that other drivers always follow the rules of the road. When you program a computer, you will have fewer bugs if you don’t assume that the functions you call are bug-free, or that code that calls your function is bug-free. When you’re working on a particular function, assume that any code that calls it is buggy.

The File Properties dialog (in PropertiesDlg.cpp) demonstrates one of the principles of defensive programming. This dialog has an Apply button that, when invoked, applies any changes to file name or other file attributes. The button is initially disabled; it is automatically enabled whenever something changes. The dialog is notified of changes through WM_COMMAND messages from its controls.

If a file resides on a CD-ROM or a write-protected floppy, you can’t change any of its attributes. For such files, all fields (except the Cancel button) are disabled, which means that the Apply button can never get enabled, right? Right. Nevertheless, I always check for “access denied” before I enable the Apply button, and also before I invoke the code behind the button.

This suspenders-and-belt approach might seem like overkill. After all, the extra checks only come into play if there’s a bug in the code. But who of us writes bug-free code consistently? I don’t think the Properties dialog has any bugs at the moment, but that’s beside the point. Defensive programming is more of an attitude than anything else.

How far to take this kind of double-checking is a judgement call. Its most valuable property is that it makes the program more robust in the face of changes. If you add a new check box to the dialog, for example, you might easily forget to disable it in case file access is denied.

## Is Windows Object-oriented?

Many years ago, an introductory text on Windows programming told me that Windows is object-oriented. The argument went like this: If you drag a window by its title bar, the rest of the window tags along. Therefore, a window is an object, and Windows is object-oriented.

This, of course, is rubbish. But the question remains: Is Windows object-oriented?

Object-oriented programming rests on three pillars: Encapsulation, inheritance and polymorphism. We should also add abstraction, which in this case means something quite concrete: the ability to create abstract data types such as C structs. This abstraction business is sometimes misunderstood to mean “programming at a higher level of abstraction,” i.e., that you operate on objects closer to the problem domain. While this tends to be true of object-oriented programming in general, it begs the point. This kind of abstraction emerges as a property of the system as a whole; it’s not a building block that you plug in at the bottom.

The core object of Windows programming is the window. Externally, to you as an application programmer, a window manifests itself as an opaque HWND handle. It’s a black box, the exact details of which are – or should be – of no interest. Internally, to the Windows system programmer, the HWND is a reference to a struct.

Some details of the internal HWND structure can be inferred, though, as they define the properties of the window. A particularly important member is the address of the window callback function. This window function determines the behavior of the window.

A window instance is created from a template called a window class. The “class” name is significant; it suggests that the original designers at least thought they were doing something object-oriented. Were they?

Encapsulation: You can store private data with a window in several ways. When you register a new window class, you can specify that additional bytes should be allocated for each window instance, tacked on to the end of the HWND structure. Since those extra bytes must be accessed by special API calls in 32-bit chunks, the facility is somewhat cumbersome to use and hardly what you would call an abstract data type. Its redeeming feature is that you can use these extra DWORDs to store pointers to heap-allocated structures. 

In addition, you can store data as window properties; this is a more dynamic method that doesn’t require you to reserve extra words in the HWND structure. This makes it especially suitable for use with windows of classes that you don’t control.

Inheritance: One of the members of the HWND structure is the address of the callback function, the WNDPROC. To subclass a window, you replace this address with the address of your own function, while stashing away the original address. Your replacement window function handles any messages you care to override, and sends the rest to the original function. You may also do pre– and post-processing relative to invocation of the original function.

Windows even has a primordial base object. As Java and Smalltalk have their respective Object classes, Windows has the DefWindowProc. The following program demonstrates this:

{code:C#}
int WINAPI WinMain( 
   HINSTANCE hinst, HINSTANCE, LPTSTR, int ) 
{
   WNDCLASS wndClass = {
      0, DefWindowProc, 0, 0, hinst, 
      0, 0, 0, 0, "Object",
   };
   RegisterClass( &wndClass );
   HWND hwnd = CreateWindow( "Object", "Object", 
      WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
      10, 10, 200, 100, HWND_DESKTOP, 0, hinst, 0 );

   MSG msg;
   while ( GetMessage( &msg, 0, 0, 0 ) ) { 
      DispatchMessage( &msg );
   }
   return 0;
}
{code:C#}
The resulting window lacks even the sense to paint its own client area, but it does have a title bar, and it responds correctly to resizing, maximizing and minimizing (see Figure 3).

![](Chapter 3 — Designing for Programmers_Figure3.bmp)

**Figure 3: The DefWindowProc window, with its client area full of garbage.**

Only one thing is missing for this to be an almost perfectly well behaved window: Even though DefWindowProc handles WM_CLOSE by calling DestroyWindow, neither the WM_CLOSE handler nor the WM_DESTROY handler calls PostQuitMessage. After the window has been destroyed, the application hangs forever on GetMessage. (It couldn’t be otherwise, of course, or all applications would close shop every time one of its windows was destroyed.)

Polymorphism: This is really at the heart of windows programming, since everything is message-based. Take WM_GETTEXT as an example. Many window types understand this message, but they handle it differently, according to their needs. A top-level window understands the message as referring to its title, while an edit control understands it as referring to its contents.

(If you like bizarre experiments, try to create a top-level edit window, and watch the ensuing confusion.)

## Yes!

The answer to our question is a resounding yes; Windows programming is indeed object-oriented programming. The bad news is that the mechanics of Windows programming is totally out of whack with any object-oriented programming language. You need to do all the plumbing manually, and this is so cumbersome as to obliterate many of the benefits of the object-orientation. In particular, all message parameters are stuffed into the WPARAM and LPARAM parameters to the window function, and the packing and unpacking of parameters is a nasty business that the word “type-safe” does not even begin to describe.

_Programming Windows is easy,_
_Provided you do the right cast._
_Of course, this technique is sleazy,_
_It’s really a blast from the past._

In the next couple of chapters, we’ll look at ways to deal with this.


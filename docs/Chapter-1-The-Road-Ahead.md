### Programming Industrial Strength Windows
[« Previous: Introduction](Introduction.md) — [Next: Designing for Users »](Chapter-2-Designing-for-Users.md)
# Chapter 1: The Road Ahead

This book is the story of a Windows application. I considered several alternatives for this application, and eventually landed on TextEdit, a Notepad replacement with a twist in the usability department. This chapter explains the how, the why and the wherefore.

## Destination: The Application

Why did I end up with TextEdit as my sample program? One reason is its convenient size. On the one hand, it’s large enough to illustrate how to build a non-trivial Windows application. On the other hand, it is small enough to fit in a reasonably sized book. On the third hand, I wanted a Notepad replacement for my own use. For some editing tasks, speed is more important than power, and Notepad does start a lot faster than Word or Visual Studio does. Every time I use it, however, I end up thinking, “I could do better than this!”

The actual editing of the text is delegated to a control. This taken care of, I can skip a great deal of that functional detail and concentrate on the mechanics of sewing up a Windows application.

I’ve seen Notepad described as “anemic,” and that is apt. For example:

* Notepad has no toolbar and no status bar
* Notepad remembers little from one moment to the next -- the only thing that comes to mind is your chosen font
* Notepad has no integration with standard input (see Chapter 7)
* Notepad lacks standard accelerator keys. (This has been fixed in the Notepad version delivered with Windows 98 and Windows 2000, but it sure didn’t used to be)
* Notepad uses the standard edit control, a widget that’s really beginning to show its age.

Notepad was never meant to be anything but a simple text editor, but I still feel that a couple of the roughest edges could have been filed off. 

Is there anything good to say about Notepad? Simple though it is, it is at least reliable, and has never failed me in all the years I’ve been exposed to the thing. It also handles Unicode gracefully.

The most important reason why I want to replace Notepad is more fundamental: The entire conceptual model that Notepad presents to the end user is faulty. Admittedly, Notepad shares this problem with most current software, so I’m being a bit unfair, picking on Notepad like this. 

I’ll get back to that particular problem in the next chapter.

## Roadbed: The Target Platform

TextEdit runs on a subset of Win32: Windows 95 or higher, and Windows NT 4.0 or higher. It does not run on any 16-bit version of Windows, or on Win32s, or on Windows NT versions prior to 4.0, or on Windows CE.

Why didn’t I concentrate on the very latest versions of Windows? As I write this, Windows 98 has been out for a while, Windows NT 5 is in its beta 3 incarnation and has been renamed to Windows 2000 (or W2K, if you like). Well, Windows 95 and Windows NT 4.0 will remain in the mainstream for quite some time yet. Furthermore, backward compatibility is an important subject that you must consider for almost all real-life applications. It’s a special form of platform independence, and I’ll show you some techniques and attitudes for making your programs robust in the face of platform changes.

The Windows family of operating systems is as afflicted by Dancing Deodorant Syndrome (DDS) as any software product these days. New features and marketing frills are apparently given higher priority than bug fixing and conceptual soundness. While I’m as enamoured of cool stuff as the next programmer, I believe that cool stuff is better built on solid ground than on shifting sands.

Why not go all the way, then, and create an application that would work on 16-bit Windows as well as on 32-bit Windows? Even if I had chosen a “least common denominator” functionality, this would have been a major undertaking. Most of the book would have been devoted to cross-platform issues, with little space left over for my real agenda. I certainly didn’t want to limit the functionality to what Win16 has to offer, since this would have cut off too many desirable features.

Creating an application that dynamically takes advantage of the underlying system is still more difficult. TextEdit does this to an extent; it uses some Windows 95-specific and some Windows NT-specific APIs dynamically. Doing this on the scale required to cover the gap between Win16 and Win32 is definitely out of the book’s scope.

There’s a “political” side to it, too – I don’t believe in flogging a dead operating system. This doesn’t mean that Win16 is dead, quite, but if you really need to create something cross-platform, the best way to go is usually to write a 16-bit application, then test it carefully on all target Win32-platforms.

## Vehicle: The Programming Language

C, C++, Basic, Java – several programming languages are reasonable choices for writing Windows applications. Why C++?

Choosing C++ over C is a no-brainer. Even if you don’t use a single object-oriented feature of C++, it is still a “better C” that lets you do function overloading and allows you to freely mix variable declarations and statements. The only reasonable reason ever to choose straight C is if you lack a C++ compiler for your platform – or if you’re writing the control software for a new airplane.

See also the discussion of MFC below.


Visual Basic is a truly amazing piece of software; it certainly amazed me the first time I saw it. Nevertheless, I’ve found it to be of limited use in my work. Here are some of the reasons:

* The language is too limiting. Object orientation really is a benefit, and Visual Basic does not offer it, advertising to the contrary. 
* You’re too shielded from the Windows API. Visual Basic “forms” are a peculiar variant of windows, a law unto themselves.
* Software distribution gets complicated, since you have to distribute the VB runtime as well as your application.
* Performance is an issue. In a given case, this may or may not be a problem. Some, however, seem to think that performance issues are no longer critical, since computers are getting faster and faster.

This kind of thinking implies that we will do the same things with software next year as we did last year, and that simply isn’t so. I’m writing this book on a laptop computer. It has a 300MHz Pentium II processor, 128MB of RAM and a 6.3GB hard disk. It was a high-end machine when I got it; now I suppose it’s barely midrange, although it continues to serve me well. 

Yet Microsoft Word takes longer to start from my hard disk than did WordStar from a floppy on a Z80 I once owned (sometime back in the early Pleistocene). The point is that Word does enormously much more for me than WordStar ever did. “Slow” may be “fast enough” sometimes, but with real speed, you can design totally different UI paradigms. 

One project I was once involved in had a window with a list of items on the left-hand side and details about the selected item on the right-hand side. Whenever a new item was selected on the left, details would appear on the right. This is a common paradigm; the problem in this project was that retrieving the detail involved heavy network operations and could take up to one minute, during which the application was completely unresponsive. The solution was to forego automatic updating, and instead add an explicit Update button to the window. A different UI paradigm.

The Windows Explorer is somewhat more clever about updating its right-hand pane. It doesn’t do this immediately after an item (folder) has been selected in the left-hand list, but (I think) starts a timer instead. The right-hand pane is updated when this timer fires. The timer is reset whenever the left-hand selection changes, and its interval is long enough that you can use the keyboard auto-repeat to arrow down the list without being held up by repeated updates.

The TextEdit Open File dialog uses this technique to update the preview window, as we shall see in Chapter 14.

* Visual Basic is easy to learn and to use. 

This is, of course, used as an argument in favor of Visual Basic, but there is a down side to it: Even beginners can get something up and running very quickly. In the ensuing adrenaline rush, they run with the ball, adding functionality and cool doodads right and left. Being beginners, they do this with neither discipline nor structure, in an environment that allows you to spread your code all over the place, or rather under each button. Sooner than you would have thought possible, you have an unmaintainable mess.

A few years ago, I was on a project where we wrote the hard parts in C, using VB as glue to stick all the parts together. Most of the programmers were inexperienced, you see, so the management felt that this was the best way. It did (of course) turn on us with a vengeance. VB’s ease of use does not turn amateurs into professional programmers; it is no substitute for actual knowledge and experience.


The Java language definition is superb in spots, but it does have holes. One hole is the lack of destructors. Garbage collection is fine, but what about objects that encapsulate files, network connections, window handles or other system resources? You’re forced to create methods for explicit destruction, and miss the beauty of C++ stack unwinding. Another hole is the lack of a preprocessor. I know that this lack is a result of a conscious decision, and I even understand the rationale behind it; I merely happen to disagree. You don’t deny grown men beef merely because it’s unfit food for babies. 

In spite of these and other holes (lack of const, for example), Java is a well-designed language with, I believe, a substantial role to play. Java still suffers – badly – from the immaturity of the tools and libraries surrounding it, and the marketing wars and quasi-political power struggles don’t help any. Those are not unique to Java, though. 

In truth, there is no “best programming language,” and you’d do well not to get religious on this issue. (That’s my prerogative.) Choose the tool best suited to the job at hand. 

I do believe that C++ is the best general-purpose programming language in widespread use available to the professional programmer. It is also the most difficult to learn and to use, which is why I emphasized “professional.” Java may yet threaten it, at least in some areas. Java’s design more or less eliminates several notorious classes of errors, including pointer arithmetic and memory management. This helps reduce the cost of software development, and, in most companies, it is difficult to argue with the bottom line.

## Class Libraries

Why write a native Windows application? Given that I’m programming in C++, why not use a ready-made class library such as Microsoft Foundation Classes (MFC)? Again, a question with no “right” answer. I’ve written MFC applications in the past, and been reasonably satisfied with the process and the results. Here are some of the reasons I chose another route for this book:

* A book about native Windows programming holds interest for MFC programmers too, while a book about MFC programming is unlikely to interest those that write to the native Windows API.
* MFC is not a true black box; you cannot use it effectively unless you have extensive knowledge of the underlying API. If you’re an MFC programmer, I hope you will take the time to read this book – to learn more about native Windows programming, to start thinking about how to design class libraries for Windows programming, and to start thinking “out of the MFC box.”
* MFC is (among other things) an application framework that makes certain assumptions about how applications should look and behave. This certainly has benefits; it allows you to create a standard MFC application very quickly, for example. It can be a straightjacket, though; things that are not provided for in the application framework are often hard to do. You can, of course, write MFC applications without taking advantage of CDocuments, CViews and CCmdTargets. If so, you give up the major benefits of MFC anyway.

If you’re building something that doesn’t fit the MFC framework, you’re better off devising your own framework. Cross-platform libraries blunt that sharp, platform-specific edge that you need for true excellence.

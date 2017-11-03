### Programming Industrial Strength Windows
[Next: The Road Ahead »](Chapter-1-—-The-Road-Ahead)
# Introduction

**“Error handling has been omitted for clarity.”**

Too many times have I read that sentence in a programming book or article. Not that there’s anything inherently wrong with this; simplification is a legitimate teaching device. The problem is that the literature is terribly one-sided – it’s a rare gem that says, “error handling has been included to show how it’s done.” Many programmers never learn how to handle errors and anomalies. Even more chilling, some never realize that the issue exists. The results are all around us: robustness and reliability are rarely considered defining characteristics of software.

This book gives you an understanding of what it takes to build industrial strength software – software that works reliably and robustly, software that doesn’t get between the user and his task. It will not bestow software nirvana upon you; indeed, no book ever could. That unattainable state of grace can only be approached through experience; the most a book can do is to help you get the right rather than the wrong kind of experience. My highest hope is that the book will inspire you to _care_ – about your users, about your profession and about getting the details right.


The word _fragmentation_ is descriptive of much current computer literature. Many books use minimalist examples to illustrate various APIs and subsystems, and rarely draw things together into a coherent whole. Again, this is not necessarily a Bad Thing; many excellent books use this approach. 

Unfortunately, it is not enough. When you assemble those fragments to create a full-blown application, their interaction gives rise to an exponential increase in complexity, and a myriad of details must be considered. This aspect of software construction is rarely covered, and the literature is left with a hole large enough to drive a truck-full of bugs through. Sadly, bugs and inconsistencies often are considered defining characteristics of computer software.

In this book I’ve taken a more holistic approach: I built it around the development of a single application called TextEdit. I did this with a considerable amount of trepidation, though – TextEdit doesn’t have years of field-testing under its belt, so I worry that it will fail to work correctly. 

One purpose of the book is to induce the same feeling of uneasiness in you. Anything that can go wrong will most certainly do so, one time or another; so preaches the Gospel according to Murphy. In software development, this often translates as “whatever users can do, some user will do,” one time or another. Many developers suffer a failure of the imagination when confronted by this simple law of nature. In one application I tried recently, a numeric input field commendably refused to accept anything but digits. It did not object, however, when I _pasted_ non-digits, realizing too late that it was in mortal danger: “Run-time error 13!” it cried, and expired.

This kind of glitch is not necessarily dishonorable; we all make mistakes. But listen to the reaction I got from the programmer when I reported it: “No, I’m not going to do anything about this. Nobody else has reported the problem, and _normal_ people wouldn’t do such a dumb thing anyway.” Idiot, he didn’t say.

Creating excellent software is difficult. No matter how hard you try, you will sometimes fail. The only sin is lack of trying; you owe it to yourself, as well as your users, to give it your best shot.


Usability comes in chunks of different granularity. On a high level, you have issues such as the overall conceptual model – the Unified File Model versus the current standard model, for example, which I’ll cover in Chapter 2. On a lower level, you have issues such as where to place the widgets in a dialog box and how to label them. This level gets the most attention in Web design, for example. On the detail level, you have a huge number of issues concerning efficient “flow,” i.e., smooth interaction. This concept is virtually unknown in Web design. Using default buttons correctly is one aspect of this that I’ll return to several times throughout the book.

Efficient flow of information ought to receive much more consideration during software design, but the current state of the World Wide Web is proof that it rarely receives any consideration at all. Does the user really need that animated deodorant on his desk, or would your development effort be better spent ensuring that the user can get his work done efficiently and effectively, or that his data are never lost?


The book is divided into two parts of unequal size. Part I is called Background, and does not concern itself much with TextEdit, but with general guiding principles for the design and implementation of computer programs. Part II is called Foreground, and is the largest by far. It is mostly concerned with the implementation details of TextEdit. 

## Who Should Read This Book?

If you are a programmer with some knowledge of {"C++"} and some Windows experience, this book is for you. If you don’t know what a {"C++"} class is, or if you have never heard of **WinMain** or window functions, you should read some introductory material first. This book doesn't cover the basics. 

It does, however, cover many aspects of application design, target platform and implementation language selection, usability issues and a myriad implementation details, including error handling and recovery, installation, internationalization and registry handling. Along the way, I touch upon enough programming issues to make this book interesting for programmers at many levels of experience. Both neophytes and experienced Windows programmers will find things of interest here.

The book also has something to offer MFC programmers and even Visual Basic programmers – in short, all programmers working with Windows. [Chapter 1](Chapter-1-—-The-Road-Ahead) discusses the relationship between several approaches to Windows programming.

This is not a reference manual, but more of an exploratory trip into nooks and crannies of the Windows API. If I don’t exhaustively explain everything found along the way, you will at least learn something about what’s out there, and can find the reference material for yourself – in the material included with your compiler or on the Net.

## The Source Code

I won’t describe my coding style in detail; you’ll get the idea by looking at the source code. Here are a couple of specifics, though, just to give you a flavor:

**I always use braces with conditionals and loops, even if only a single statement is controlled.** I started this practice after I had spent several hours hunting down a bug caused by indentation mismatch:

{code:C#}
if ( x == y )
   ++x;
   --y;
{code:C#}
I wowed that this bug would never bite me again, and it hasn’t. Yet.

**When comparing for equality, I always put the constant (if any) on the left-hand side.** In the past, I’ve been caught more than once by unintended assignments:

{code:C#}
if ( variable = CONSTANT ) {
   ...
}
{code:C#}
If you switch variable and CONSTANT, the assignment above generates a compilation error, as CONSTANT is not an l-value.

What these examples have in common is that they prevent bugs. Some aspects of coding style are matters of preference – it’s unimportant whether you indent your code with three or four spaces. If you do not indent your code at all, however, it matters a great deal, since such formatting fails to reflect the logical structure of the program.

Religious wars are fought over coding styles, and, curiously enough, the most intense battles are fought over insignificant details. I used to have a few pet hang-ups myself until I woke up one day and decided to do a thorough housecleaning. I completely revised my coding style, making conscious decisions at all points. As a result, I am now free of any religious hang-ups. (Some people will tell you that I’ve merely replaced old hang-ups with new ones. Pay them no heed; they are merely propagating vicious rumors spread by my enemies.)

All function and method names start with a lower-case character. This is a result of my exposure to Java; I found that I rather liked that convention. Preferences aside, the convention has practical advantages. First, it makes it easy to distinguish between class names and function names without resorting to the ugly C prefix for classes. Second, it avoids name clashes with the Windows API. The name space of windows.h and its underlings is a confused jumble where the documented name for a function as often as not is a macro resolving to either an ANSI or a Unicode version of the function. If, for example, a class has a method named GetWindowText, the method would actually be named GetWindowTextA or GetWindowTextW, depending. The result of this renaming behind your back can be anything from simple confusion to outright compilation errors.

For a deeper discussion of coding styles, I recommend Steve McConnel’s Code Complete. In particular, pay attention to the way he clearly formulates goals for a set of coding conventions. For example, the code should clearly reflect the logical structure of the program, it should be readable, it should be maintainable (not necessarily the same thing), and it should prevent bugs.


When I refer to a _method_, I invariably mean a non-static member function of some class. Everything else is a function.


TextEdit was developed using version 6 of Microsoft Visual {"C++"}. While the text reflects that fact on occasion, the book is meant to be compiler-independent. In particular, it is not a book about Visual {"C++"}.

| References to source code availability removed from text; they're irrelevant on CodePlex. |

## Acknowledgements

The good will and helpful assistance of many people helped turn this book into reality. Ron Burk, editor of Windows Developer’s Journal, has given me many a stimulating discussion on Windows programming, and has bought numerous articles from me. Berney Willams, acquisitions editor at R&D Books, liked my proposal well enough to offer me a contract. Nils Øveraas, my boss at Andersen Consulting’s Oslo office, approved of the project and gave me the time I needed to write.

Thanks to the above people, there is a book. Contributions from many others helped improve both the book and the TextEdit application. Ron Burk and Alf Steinbach offered insights on C++ and the Windows API; Oddmund Møgedal and Tone Pettersen offered insights on usability. Paal Beyer, Anders Blehr, Frode Bonesvoll, Kjell Furnes, Henning Normann, Øyvind Pedersen, Simen Røkaas, Øyvind Sandvold, Håvard Stenberg and Frode Strand gave me feedback during the beta testing of TextEdit. Henrik Lund-Hanssen, Velitchko Koulichev and Tone Pettersen commented on the text, as did my wife Linda. Francis Daveronis helped with Greek, and Jean-Marc Delcourt helped with the French (what little there is of either). Mike Wallace did the technical editing, Michelle Dowdy did the copy editing and Robert Ward created a nifty cover illustration. Any remaining inaccuracies, errors or omissions are my own responsibility.

On a more general note, I would like to thank my colleagues at Andersen Consulting’s Oslo office for providing a generally stimulating work environment. I find it hard to conceive of a more interesting place to work.

Last, but by no means least, I thank my family – Linda, Håkon, Vegard and Anders – for their support and encouragement while I was writing this book. I could not have done it without you.

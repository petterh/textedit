### Programming Industrial Strength Windows

[« Previous: TextEdit Command Index](Appendix-A-TextEdit-Command-Index.md)

# Appendix B: Bibliography and Recommended Reading

 _2008 update: Some of these recommendations are a bit outdated. I'll update this page when I get some spare time.

In addition to the books referenced in the text, this bibliography lists a few other books and publications that I consider good and useful. Some of them have had a strong influence on how I think about software design.

Looking over this list, I’m struck by one common feature of the books: Their permeating attitude that you must understand what you’re doing, that it’s important to get the details right, that the whole exercise is worth while. Some of them provide a wealth of technical detail as well, but so does a good reference manual. The underlying philosophy of design and programming is what wins me over.

### Laura Arlov: GUI Design for Dummies. IDG Books Worldwide 1997

Despite the dummy title, this is an excellent introduction to usability and the design of graphical user interfaces, full of valuable hints for the practicing designer.

### Alan Cooper: About Face: The Essentials of User Interface Design. IDG Books Worldwide 1995

This book covers a wide range of UI topics, from high-level conceptual models down to minute details of mouse interaction. Even if you don’t agree with everything Cooper says, you’re bound to find much of value here.

### Dave Edson: Dave’s Book of Top Ten Lists for Great Windows Programming. M&T Books 1995

This is a collection of articles on Windows programming. The material on Windows 3.x is a bit dated, unless you’re stuck doing 16-bit programming, but that’s only a minor part of the book anyway. Edson ranges wide and far across the Windows landscape, giving a good overview of what’s out there.

### Erich Gamma, Richard Helm, Ralph Johnson and John Vlissides: Design Patterns: Elements of Reusable Object-Oriented Software. Addison-Wesley 1995

Design patterns for software development are codified rules of thumb that solve recurring and often general problems. In the inimitable words of Martin Fowler: “Some idea, found to be useful in a practical context, which will probably be useful in other contexts.”

This book catalogues simple and elegant solutions that have developed and evolved over time. Don’t leave home without it.

### Andrew Koenig: C Traps and Pitfalls. Addison-Wesley 1988

Much of this book concern pitfalls in pre-ANSI C, which does date it a bit. It still contains valuable tips, though, and it is saturated with a pervading sense of the necessity to understand what you’re doing.

The following (perfectly portable) expression gives a flavor of the book; it evaluates to the hexadecimal character corresponding to x:

```C++
"0123456789ABCDEF" [ x & 0xf ]
```

### Steve Maguire: Writing Solid Code. Microsoft Press 1994

This book is devoted to exterminating bugs by eliminating bad habits and encouraging good ones. Maguire’s approach is anecdotal, entertaining and thought-provoking.

### Steve McConnell: Code Complete. Microsoft Press 1993

This is the most comprehensive overview of software construction that I know. To quote the author: “The research and programming experience collected in this book will help you create high-quality software and do your work more quickly and with fewer problems.”

If you don’t own this book, run out and buy it.

### Microsoft Systems Journal. Miller Freeman. [http://www.msj.com](http___www.msj.com)

This magazine is devoted to evangelizing the plethora of hot, new technologies that flows from Redmond. It has much well-presented technical detail, and is recommended reading if you work on the leading edge of Windows technology.

### Charles Petzold and Paul Yao: Programming Windows 95. Microsoft Press 1996

I maintain an army of random monkeys that bang away at my keyboard when I’m not using it. One of them came up with this:

Programming Windows is simple\
When Petzold is at your side,\
To smooth over every pimple\
And polish your program’s hide.

I can’t improve on that; I only want to add that my 1996 edition is not the latest Petzold. The latest Petzold is much heavier than mine, and requires an industrial-strength wheelbarrow just to get it home from the bookstore.

### Jeffrey Richter: Advanced Windows Programming (Third Edition). Microsoft Press 1996

True to its title, this book is a deep dive into the Windows API. It does an excellent and comprehensive job of describing kernel objects, processes, threads, fibers, multithreading, synchronization objects, asynchronous I/O, virtual memory and more. Much more, in fact.

### Bruce Tognazzini: Tog on Software Design. Addison-Wesley 1996

This is a collection of design essays from one of the pioneers behind the Apple Macintosh and the creator of Sun Microsystem’s Starfire vision.

I’m particularly fond of his catalog of surrealistic error messages such as these:

* Unable to save file. Save anyway (y or n)?
* No keyboard found. Hit F1 to continue.

Need I say more?

### Windows Developer’s Journal. Miller Freeman. [http://www.wdj.com](http___www.wdj.com)

Windows Developer’s Journal is devoted to practical solutions for the working Windows programmer. It differs from Microsoft Systems Journal in being independent of Microsoft, and in having a quite different editorial focus to begin with. WDJ is my favorite software magazine.

(Since I write regularly for WDJ, you may suspect a bias on my part. But it’s not my favorite magazine because I write for it; I write for it because it is my favorite magazine.)

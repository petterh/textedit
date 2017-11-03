### Programming Industrial Strength Windows
[« Previous: Setup, and Down Again](Chapter-20-—-Setup,-and-Down-Again) — [Next: TextEdit Command Index »](Appendix-A-—-TextEdit-Command-Index)
# Chapter 21: The End of the Road

This is it. We’ve reached the end of the road; TextEdit is done. At least, version 1 of TextEdit is done. There are many things I’d like to change, and there are many features that I’d like to add. In spite of blemishes and imperfections, however, I think the overall design and implementation of TextEdit illustrate my basic arguments. My intention with this book has not been to present TextEdit as an example of a perfect Windows program, but rather to make you think.

About this, for example:

* Error handling and error reporting must be integrated into the application architecture from the beginning. Even then, it is a difficult subject, and the unified file model complicates it still more.
* Defensive programming takes longer. It also gets you to your goal faster. Paradox!
* The details matter. They matter so much as to make black-box object-oriented programming very, very difficult. I contend that low-level systems programming can’t be truly object-oriented, and any attempt to describe an operating system as object-oriented is, at best, marketing fluff. 
* If you keep the user’s best interest in mind, you’ll end up doing lots more work. Your users may end up wearing expressions of delighted surprise.
* The Unified File Model is – I believe – superior to the standard model. As computers become household appliances, we need to adopt stern measures of simplicity and usability. 

I admit to some suspense as to how the unified file model will actually fare; the industry has decades of conditioning to turn around. TextEdit is an experiment, and the jury is still deliberating.


Let me close with some thoughts and ideas on how TextEdit could be improved. This includes additional functionality as well as improvements in the architecture and user interface, all mixed up.

* Forward/backward buttons a la browser (work within a single TextEdit session)
* Look at files across Internet protocols, such as HTTP
* Regular expressions for search and replace. Don’t reinvent the wheel; there are several good, free implementations on the Net
* Indent/outdent selection
* Mail integration: File/SendTo/Mail Recipient, Fax Recipient
* Copy to Floppy command
* Improve loading speed for large files. Load and display the first page (or one I/O page, typically 64KB), then read the rest at our leisure. The second-best solution is to update the window, finish reading and update the edit widget before starting any interaction; the waiting time before useful interaction would be the same as now. The best solution is more complicated to implement: Load the remainder of the document in a background thread, and allow whatever limited interaction possible until the total document has been loaded.
* Additional shell property page that displays Unicode/Linefeeds/preview
* Customizable headers and footers for printing
* Insert menu, with commands to insert Date and Time, perhaps whole files
* Bookmarks
* Properties dialog, file name edit field: Filter out illegal file name characters. This also requires that you intercept clipboard pasting, as discussed in Chapter 4
* Improve the File Not Found dialog – put the name of the offending file in an edit field rather than a static, and add the MRU as a drop-down list.
* Brace and parentheses matching; cool for programmers such as thee and me, and nobody else need know about it
* Option to keep paragraphs together during printing
* Spell checker
* Word count – as part of properties page, perhaps.
* Sensitivity to power status
* Overtype mode, toggled by INS key. Requires edit subclassing (or a different edit control), plus display on the status bar.
* Splitter windows, at least vertically
* An alternative to Window Maximize that makes the window as large as necessary, but no larger
* Corollary: For first-time viewing of a file, make an intelligent decision on window size and placement based on content rather than the Windows default values
* Track changes (view differences between current file and another)
* A “goto line number” command-line parameter, allowing integration with other programs in the form of links. Additional parameters are possible. Implementing OLE Automation is a thought, although not entirely a happy-making one
* Initialize different default font and tab setting for different types of source code documents. Most people will usually view source code will a fixed font, for example; assembly listings traditionally require eight spaces per tab
* File names are handled sloppily. If the value of MAX{"_"}PATH ever grows, we’ll have to recompile. A more bullet-proof solution would be to inspect the volume
* Event log integration on Windows NT
* Single process, multiple UI threads, rather than one process for each instance
* More robust setup. TextEdit makes some assumptions about its environment that may sometimes be wrong
* Setup/customization: Position the “Browse for Folder” dialogs under their corresponding edit fields so that it’s easier to see where they belong
* Unicode version that runs well under Windows 9x (non-trivial!)
* Using NTFS streams
* Try default extensions one by one when open file fails
* Fix typing in combo box when the drop-down list is open
* File New: Add a “select file type” dialog box?

Some of these are easy, and some are hard. It is never right, however, to just barge in and implement whatever seems easiest or most fun or interesting to implement. 

| 2008 update: Some are also irrelevant. If you want to create a Unicode version that runs well on Win9x, be my guest. I'm not going to, ever. |

First, make a list of priorities, of guiding principles that will help you select appropriate features. If you do this in the spirit of TextEdit, rock solid robustness takes precedence over flashy gimmicks, for example. 

Next, do the detailed design for the selected features, striving for a conceptually unified whole. If you elect to replace the MRU list with browser-type forward/backward buttons, for example, you should also consider an address field for the file name. 

Then, and only then, should you start thinking about the implementation.


In this book, I’ve tried to emphasize the importance of doing things properly, of getting the details right. But no matter how good your intentions, you can’t always do what you like. The marketplace imposes its own constraints, including competitors’ feature sets, impending ship dates and escalating costs. Striking the proper path through this terrain is a difficult balancing act. I do hope, though, that the industry as a whole will start to veer ever so slightly towards a stance of greater responsibility towards the end user. Perhaps the market forces will start to work that way, too, as the global economy comes to rely more heavily upon the correct functioning of software, and ever more software users – and software buyers! – come to realize that efficient flow of information is more important than dancing deodorants.


Do the world a favor. Help lead the way.

### Programming Industrial Strength Windows
[« Previous: Going Abroad](Chapter-18-Going-Abroad.md) — [Next: Setup, and Down Again »](Chapter-20-Setup-and-Down-Again.md)
# Chapter 19: Meanwhile, in the Background

Multi-threading is used for many different purposes. In a server application, a typical purpose is to service multiple requests in parallel, while in a client application, a typical purpose is to perform background processing while keeping the user interface responsive. 

An alternative approach to client-side background processing is idle-time processing. Under 16-bit Windows, this is the only possibility, as Win16 doesn’t have preemptive multitasking (except between virtual machines, which is of little help within a single application).

The best choice depends on the characteristics of the background processing. If your task is easily broken into small chunks, and it is easy to maintain the task’s internal state between the execution of chunks, idle-time processing may be a good choice. If not, threads may be a better choice.

The problem with multi-threading is that it can be difficult. To begin with, a whole slew of bugs is associated with multi-threading, including unprotected access to shared resources, synchronization between threads, deadlocks and race conditions. Moreover, multithreading bugs are hard to debug. For example, a multi-threaded program that works without fail on a single-processor machine may well fail without fail on a multi-processor machine, when the threads get the opportunity to really run at the same time, instead of merely being scheduled one at a time. A multi-threaded program that hasn’t been tested on a multi-processor machine should be regarded as untested.

In the case of memory management, excellent tools can help you find memory leaks, reads and writes outside allocated boundaries, reading from uninitialized memory and so on. No similar magic exists in the case of multi-threading; the best you can do is to apply the KISS principle in full force – keep things as simple as you possibly can. 

The Windows NT Explorer contains a lovely example of multithreading. When you open a directory (excuse me, a folder), the Explorer displays an icon for each file. Many icons are cached in the system image list, but the Explorer always displays icon or cursor files with the actual icon in the file. No icons are associated with .ico or .cur files in general. Getting hold of all those icons may take a long time, particularly if the folder is on a floppy or similarly slow medium. What the Explorer does is to display all the files immediately with a generic Windows icon, then update the icons at its leisure, remaining responsive to user input.

(Never having seen the source code for the Windows NT Explorer, I must admit that I do not know that this feature is implemented using threads. But I suspect it is. My Task Manager shows EXPLORER.EXE running no less than eleven threads right now, so the thing is certainly multithreaded.)

## Threads in TextEdit

There are several possible uses for background threads in TextEdit – automatic background saving, background printing or continuous monitoring of the disk. It would even be conceivable to run each concurrent TextEdit instance as a separate UI thread within the same single process. 

Keeping the KISS principle (Keep It Simple, Stupid!) in mind, I ended up with no background threads whatsoever in TextEdit proper. Triggering automatic background saving from a timer is smooth enough. TextEdit does have worker threads, but they only make an appearance during setup, not during normal operation. The purpose of searchPreviousThread, uninstallThread and installThread is to let the user interface of SetupDlg remain responsive while potentially time-consuming tasks are going on. I shall get back to SetupDlg in the next chapter.

## Starting Threads

The Win32 API gives you the **CreateThread** function. You should never use this function directly if you make calls to the C runtime library from multiple threads. Instead, you should use the CRT functions **{"_beginthread"}** or **{"_beginthreadex"}**. This allocates resources for the CRT on a per-thread basis, ensuring that different threads don’t trample each other’s data. The **strtok** function is a good example of a problem function: Since it must retain state between invocations, each thread needs its own storage for this.

In TextEdit, this is wrapped in **beginThread** and **endThread**, defined in threads.h.

< Listing 75: threads.h >

## Communication between Threads

The threads in SetupDlg communicate with the dialog’s UI thread by sending or posting messages. Message posting, being asynchronous, is usually a safer bet than message sending, but consider this code fragment in SetupDlg’s installThread function:

```C++
try {
   pSetupDlg->install();
}
catch ( const Exception& x ) {
   pSetupDlg->sendMessage( WM_APP, INSTALL_FAILED, 
      reinterpret_cast< LPARAM >( x.what() ) );
}
```
On the receiving end of this sendMessage call, the UI thread needs to get hold of the string passed in LPARAM. If the message were posted, the string would almost certainly be invalid by the time the UI thread got around to handling it. One possibility would be to copy the string to a static buffer. Since this creates reentrancy problems, it is bad as a general solution.

The problem with using **sendMessage** lies in the UI thread’s response to the message. Among other things, it calls **cleanupThread**, which calls **WaitForSingleObject** on the thread handle. Since the thread is currently in **SendMessage**, waiting for a response, it will not terminate any time soon. We have a deadlock situation until **WaitForSingleObject** times out.

One solution would be to split the message in two – one message for setting the string, another to indicate that the worker thread is done and about to die. The first would be sent, the second would be posted.

Another solution – the one actually used – is to employ **ReplyMessage**. On the receiving end, the UI thread first grabs the string, then calls **ReplyMessage** and finally **cleanupThread**:

```C++
case INSTALL_FAILED:
   ...
   setDlgItemText( 
      IDC_MESSAGE2, reinterpret_cast< LPCTSTR >( lParam ) );
   ReplyMessage( 0 );
   ...
   cleanupThread();
   break;
```
After the **ReplyMessage** call, the worker thread’s call to **sendMessage** returns, and the worker thread is free to continue to termination. Thus, **cleanupThread** avoids the deadlock.

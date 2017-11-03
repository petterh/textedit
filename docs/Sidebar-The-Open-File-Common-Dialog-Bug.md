### Programming Industrial Strength Windows
[« Back: File Management](Chapter-14-File-Management)
## Sidebar: The Open File Common Dialog Bug

The Win32 Open File common dialog has a curious bug – at least it does under Windows NT 4.0. It is not even close to being a showstopper, but it is nevertheless a bug. To manifest the bug, do the following:

# Start the Control Panel Display applet, and select the Appearance tab. Select the standard Windows color scheme, and click the Apply button.
# Start Notepad and select the File Open command. 
# Without closing Notepad’s Open File dialog box, go back to the Display applet, select the High Contrast Black color scheme and click the Apply button again. 

The expected result is for the file list in the Open File dialog to take on the characteristics of the High Contrast Black color scheme, namely a black background with white, boldface text. The background, however, remains white. The font remains black, and its boldness does not increase noticeably. 

To verify that this is indeed a bug, and not just some weird feature, close Notepad’s Open File dialog and open it again to verify that it has now taken on the expected look.

If you try the same trick with the Visual Studio editor rather than Notepad, the story is the same. If you try the same trick with Microsoft Word, though, the story is slightly different – Word’s Open File dialog does handle the color change, but not the font change. In other words, close, but no cigar.

My first thought was that the list view had a bug, but that turned out not to be the case. If you tell the list view that the color scheme has changed, it will change its looks to match the new scheme.

When the color scheme changes, Windows broadcasts a WM{"_"}SYSCOLORCHANGE message to all top-level windows. The Open File common dialog receives the WM{"_"}SYSCOLORCHANGE message, but fails to pass it on to the list view. According to the SDK documentation, it should have done so: “Top level windows that use common controls must forward the WM{"_"}SYSCOLORCHANGE message to the controls; otherwise, the controls will not be notified of the color change.”

What about the font change? When the font changes, Windows broadcasts a WM{"_"}SETTINGCHANGE (formerly WM{"_"}WININICHANGE) message to all top-level windows. Again, the Open File dialog receives the message, but fails to pass it on. The documentation for WM{"_"}SETTINGCHANGE does not mention common controls, but if you fail to pass on the message, the font does not change.

To solve the problem, it is not enough to add a hook function to the Open File dialog. Since WM{"_"}SYSCOLORCHANGE messages and WM{"_"}WININICHANGE messages aren’t sent to the hook function, we must subclass the dialog itself. An Instansubclasser called s{"_"}commonDlgSubclasser, defined in openDlgCommon.cpp, is applied when the hook function receives the CDN{"_"}INITDONE notification, and ensures that the list view gets the required messages.

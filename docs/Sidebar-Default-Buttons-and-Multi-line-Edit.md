### Programming Industrial Strength Windows
[« Back: About Dialogs](Chapter-13-About-Dialogs.md)
## Sidebar: Default Buttons and Multi-line Edit

I'd like to draw your attention to a user interface bug that runs rampant through the world of Windows applications. It involves the interaction of multi-line edit controls and default push buttons.

In the beginning was the multi-line edit control. Entering multiple lines was cumbersome in those days, as you were required to press Ctrl{"+"}Enter instead of just Enter. This was especially unfortunate whenever a default pushbutton was around; users would hit Enter to start a new line and promptly lose the whole dialog box (along with their peace of mind).

This whole Ctrl{"+"}Enter business was so obscure that many applications added extra text to educate users in proper procedure, i.e., "Hit Ctrl{"+"}Enter for new line.” Mistakes were nevertheless common, even among experienced users.

Along came Windows 3.1 and the ES{"_"}WANTRETURN style, and all was well with the world. Or was it? The problem was now turned on its head; users would type some text and then hit Enter to invoke the default push button. What they got was a new line (along with a fresh set of aggravations).

The old style of interaction was merely bad. The new style was, when used in the wrong context, an actual bug in the user interface. That bug has been with us ever since.

There are three approaches to fixing the problem: 

# Read the user's mind to decide which action is appropriate -- a new line or a button push.
# Resurrect the Ctrl{"+"}Enter kludge.
# Remove the fat border from the default button whenever a multi-line edit control with the ES{"_"}WANTRETURN style has the keyboard focus.

The first solution is beyond the current state of the art. By the time we get around to it, this little problem is most likely irrelevant anyway.

The second solution is easy; just remove the ES{"_"}WANTRETURN style from the edit control. Unfortunately, this results in a bad user interface.

The third solution is the only sensible way to handle the problem. Some applications use it, others don't. Still others are inconsistent; this last group even includes luminaries such as Word for Windows.

This WM{"_"}COMMAND handler code fragment implements this in the simplest possible way: 

```C++
if ( IDC_MY_MULTILINE_EDIT == id ) {
   if ( EN_SETFOCUS == codeNotify ) {
      SendMessage( DM_SETDEFID, 0, 0 );
   } else if ( EN_KILLFOCUS == codeNotify ) {
      SendMessage( DM_SETDEFID, IDC_OK, 0 );
   }
   return TRUE;
}
```

If your dialog contains a button with the ID IDOK, the call

```C++
SendMessage( DM_SETDEFID, 0, 0 );
```
will give the OK button the default property. This is why the code fragment above uses IDC{"_"}OK, a value presumably different from 1. 

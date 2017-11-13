### Programming Industrial Strength Windows

[« Back: About Dialogs](Chapter-13-About-Dialogs.md)

## Sidebar:  The HTML Static Control

The resource script for the About TextEdit dialog box includes this line:

```
LTEXT  "TextEdit was developed as a companion program to the (shamelessly recommended) book \
<b>Programming Industrial Strength Windows</b>, written by Petter Hesselberg and published by R&D Books, an imprint of Miller Freeman.", \
IDC_COMMERCIAL,42,130,205,37,SS_NOPREFIX
```

As you can see, the text of the static control is HTML (HyperText Markup Language). (Actually, it isn’t, quite, but it does use HTML formatting codes.) If I didn’t subclass the thing, the text would appear verbatim, including the formatting codes \<b\> and \</b\>.

The two main tasks involved in this are HTML parsing, handled by getToken, and HTML rendering, handled by paintHTML. These functions are implemented in HTML.cpp; the interface to the module is in HTML.h.

## HTML Parsing

To keep the parsing engine small and simple, I implemented only the three HTML tags \<b\>, \</b\> and \<p\>. These are all hard-coded into the parser; should you ever want to support additional tags, consider using a more flexible parser framework. In addition to the HTML tags, the parser understands about white space and text, making five token types in all.

To pick the HTML source string apart, I give it to getToken, which strips the first token off the source string. I call getToken repeatedly until the source string is empty, or until the client area of the control has been filled up, whichever comes first.

First, I must decide what kind of token it is. If the first character is any kind of white space, I’ve found a white space token. If the string starts with one of the supported tags, I have a tag token, and if it’s neither a tag nor a white space, I have a text token on my hands.

Second, I strip the token off the source string. If the token is a white space token, I trim all leading white space off the string, as multiple white space characters should be merged into a single white space token. If it’s a tag token, I strip off the tag, then set or clear the corresponding flag, as appropriate. If it’s a text token, I figure out where the next token starts, and store the offset in the iNext variable. When I’m parsing the last token, iNext equals the length of the remaining string.

getToken returns false when the source string is empty.

## HTML Rendering

The WM_PAINT message is handled by onPaint, which does housekeeping chores such as selecting appropriate colors into the device context. It defers the actual rendering to paintHTML, a useful function in its own right. Using paintHTML, you can render HTML in any display context; look at the TextEdit status bar for an example of this.

In truth, paintHTML doesn’t do much but housekeeping chores itself, deferring the rendering to doPaint. This has no deep significance; it came about because paintHTML was becoming a bit unwieldy.

In doPaint, I call getToken to get the next token. If it’s a paragraph token, I move the current position to the left margin, one and a half lines down. Otherwise, if the token string is empty, I ignore it. An empty token string means that getToken found the \<b\> or the \</b\> tag; this fact has already been recorded in the state of the bBold flag.

If it’s a text token, I display it with TextOut. First, I select either the normal font or the bold font into the display context, depending on the value of bBold. Next, I measure the extent of the string. If the word is too wide to fit on the current line, I move to the start of the next line – unless I’m currently at the left-hand border. In that case, starting a new line won’t help, as the word won’t fit in any case. If I find that I’ve dropped below the bottom of the control’s client area, I quit the loop.

A space is handled exactly like a word, except that I don’t display it if it’s the first thing on a line. Displaying a space with TextOut makes no visible marks on the display context, of course, but it does update the current position.

Actually, the above account doesn’t tell the whole story. I claimed that, when a word is too long to fit, I just wrap to the next line. I also said that paintHTML is used by the status bar, which has but a single line of text. Wrapping to the next line is a useless strategy when only one line is available.

To handle this situation, pass the PHTML_SINGLE_LINE flag to paintHTML. If this flag is specified, doPaint doesn’t wrap to the next line, but truncates the string as best it can -- using compactPath. This function, defined in fileUtils.cpp, replaces part of the string with an ellipsis, so that it fits in the available space.

The window function for the Pocket HTML control is Simple_HTML_WndProc. In addition to painting in response to WM_PAINT, the window function uses fillSysColorSolidRect to clear the background in response to the WM_ERASE message.

Since I sometimes want to set the control’s text dynamically, the WM_SETTEXT message must also be handled. Why? When the static control receives the WM_SETTEXT message, it repaints itself directly, rather than invalidating itself and waiting for WM_PAINT to arrive. This kind of (regrettably common) misbehavior is done to improve efficiency, but it does complicate subclassing. The subclassed WM_SETTEXT handler calls the original window function first, then invalidates the client area.

A final note about the Pocket HTML control: Both FindDlg and StatusBar like to highlight the text, as shown in Figure 27. HTML.h defines a pair of functions – setHighlight and removeHighlight – that set or remove a window property called – you guessed it – “Highlight.” If this property is set, a different set of colors is used to paint the background and the text. On WM_DESTROY, the highlight property is removed, to avoid leakage.

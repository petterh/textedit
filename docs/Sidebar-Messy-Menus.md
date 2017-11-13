### Programming Industrial Strength Windows

[« Back: The Main Window](Chapter-9-The-Main-Window.md)

## Sidebar:  Messy Menus

The keyboard interface to the menu has its problems. Hold down the Alt key, press F, and the File menu pops up (or rolls down, whichever you prefer). Don’t let go of that Alt key yet; look instead at the line that says “Properties… Alt+Enter.”

Now imagine the neophyte with his Alt key down, looking at Figure 11 (either half) and trying to get to File Properties:

“OK, I have the Alt key down already, after doing Alt+F. All I have to do now is press Enter, lets see…damn! What happened? Why did I get a new file?”

Or imagine him, the Alt key now released, still looking at Figure 11, pressing Ctrl+O for all he’s worth, and still no Open dialog is forthcoming.

Microsoft, are you listening?

![](Sidebar: Messy Menus_Figure11.bmp)

**Figure 11: The File menu is unnecessarily wide (left-hand menu), or the accelerators are not aligned (right hand menu).**

## Sidebar: Accelerator Alignment

To align the accelerator descriptions in a column at the right-hand side of the menu, you include a tab character (\t) in the menu text. The left part of Figure 11 illustrates this. It also illustrates a problem with this approach; the accelerator descriptions are lined up beyond the widest item in the menu. The widest items are on the MRU list, which is visually disconnected from the rest of the menu. We could save a lot of real estate by ignoring the MRU list when determining the tab position.

An alternative is to replace \t with \a in the resource script. This aligns all the text that follows it flush right on the menu, as shown in the right-hand part of Figure 11. This menu is much more efficient in its use of pixels than the left-hand example, achieved at the cost of destroying the alignment of the accelerator descriptions.

I would dearly have loved to have the best of both worlds: the space efficiency of the right-hand variant with the alignment of the left-hand variant.

Microsoft, écoutez-vous?

Hint: If you add menu items programmatically rather than from a resource script, replace \a with \b, or it won’t work. I can’t say whether this is a bug or a feature.

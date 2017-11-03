# textedit

TextEdit is a text editor, written in C++ to the raw Win32 API. It illustrates how to fit together all the myriad bits and pieces that make a robust Windows application a, well, robust Windows application.

TextEdit was originally written as the companion program to my book Programming Industrial Strength Windows (2000); I've occasionally fiddled with it in the years since then. It used to have a [home on CodePlex](https://textedit.codeplex.com/); it has now moved to GitHub. RIP, CodePlex.

## About TextEdit

TextEdit demonstrates a number of things:

* There is no _Save_ command. TextEdit follows Allan Cooper's unified file model, where all changes are committed directly to disk.
* Error handling and robustness.
* Persistence everywhere.
* Send as email.
* Installation and registration.
* Utilizing the SendTo folder.
* The myriad ways of starting a Windows app and receiving arguments.
* The fine points of dialogs.
* Simple HTML parsing and rendering.
* Snap window to edges of work area during move and resize.

## Documentation

It's in the [`docs`](docs) folder, but be warned: All links need tweaking -- the import from CodePlex was imperfect (spaces in file names being the main problem). I'll get around to it.

## Getting started

* [Install Visual Studio](https://www.visualstudio.com/downloads/)
* [Install WiX](http://wixtoolset.org/)
* Clone this project

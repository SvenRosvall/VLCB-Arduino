# Generating API documentation with Doxygen

Doxygen is a powerful tool for generating documentation from
source code files, augmented by comments.

Generating API documentation directly from source code reduces the
risk of documentation getting out-of-sync with the source code.
Keeping documentation in separate files risks getting stale as
the source code evolves.
Having documentation in comments in the source file, near declarations
makes it more obvious to the editor that the comments also need to 
be changed when the code changes.

The VLCB-Arduino library has two sets of API documentation, one complete
set for library developers, and a second restricted set that describes 
functions and objects that sketch developers need.

## Using Doxygen in VLCB-Arduino

Download and install Doxygen from https://www.doxygen.nl/

Doxygen requires a configuration file.
This is [Doxygen.conf](../Doxygen.Sketch.conf) in the root directory.

Run Doxygen with this command line in the root directory:
```
doxygen Doxygen.Sketch.conf
doxygen Doxygen.Library.conf
```
This updates documentation in the `html.sketch` directory and 
`html.library` directory respectively.

Once the documentation is updated, review it and commit to git.
Note that some new files may have been created. 
These needed to be staged for addition to git.

## Browsing the generated documentation

Read the generated documentation locally by opening one of the
[Sketch developer index file](../html.sketch/index.html) or
[Library developer index file](../html.library/index.html)
in your browser.

If you browse the index file from Github you will see the raw HTML
source code.
Github has a feature (called "pages") that renders the HTML pages
(and Markdown pages).
Enable the "pages" in **Settings** in the Github repository.
Select **Pages** on the left.
Set **Source** to "Deploy from a brach" and under **Branch** choose
the branch that is used for documentation (if anything else than "main").
The directory shall be "/ (root)".
Press [Save] and you are done.
The pages will now be available at https:<your-username>.githop.io/VLCB-Arduino.
(See https://stackoverflow.com/questions/8446218/how-to-see-an-html-page-on-github-as-a-normal-rendered-html-page-to-see-preview)

The main repository will be maintained in https://svenrosvall.github.io/VLCB-Arduino
The index of the generated documentation is available at https://svenrosvall.github.io/VLCB-Arduino/html.library/index.html

## What to document
High level documentation is kept in the `docs` directory.
This high level documentation includes [Design Overview](Design.md) and
pages that describe certain crucial concepts and classes. 

Any detailed documentation on specific classes and functions shall
be documented with Doxygen comments in the source code where these are declared.
Typically, these declarations are in header files.

Focus is on classes and functions that are used by users of this library.
Internal entities only need standard C++ comments which are not read by Doxygen.

## How to write Doxygen comments

The Doxygen website had documentation on [documentation comments](https://www.doxygen.nl/manual/docblocks.html).

In summary:

* Use three slashes (`///`) for documentation comments.
* Place these comments above the declaration they apply to.
* You can use some Markdown markers in these comments.
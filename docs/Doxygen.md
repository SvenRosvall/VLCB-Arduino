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

## CI-automated documentation

The documentation is generated automatically by a [GitHub Actions workflow](../.github/workflows/ci-docs.yaml)
on every push or pull request that changes source files or documentation.
On pushes to the `main` branch, the generated documentation is deployed to GitHub Pages.

The live documentation is available at:
- [Library developer docs](https://svenrosvall.github.io/VLCB-Arduino/html.library/index.html)
- [Sketch developer docs](https://svenrosvall.github.io/VLCB-Arduino/html.sketch/index.html)

The main repository is maintained at https://svenrosvall.github.io/VLCB-Arduino

## Generating documentation locally

For previewing changes before pushing, generate the documentation locally.
Download and install Doxygen from https://www.doxygen.nl/.

Doxygen requires a configuration file.
This is [Doxygen.conf](../Doxygen.Sketch.conf) in the root directory.

Run Doxygen with this command line in the root directory:
```
doxygen Doxygen.Sketch.conf
doxygen Doxygen.Library.conf
```
This updates documentation in the `html.sketch` directory and 
`html.library` directory respectively.

Read the generated documentation locally by opening one of the
[Sketch developer index file](../html.sketch/index.html) or
[Library developer index file](../html.library/index.html)
in your browser.

> **Note**: Do not commit the generated `html.sketch/` or `html.library/` directories.
> The CI workflow generates and deploys them automatically.

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

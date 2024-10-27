
# nothydrus

Simple, tag-based, CLI and TUI media management program inspired by [hydrus](https://hydrusnetwork.github.io/hydrus/).

nothydrus creates an sqlite database to manage media files in a folder, and allows you to tag files, search them, and browse them, while leaving the files in-place.
The TUI uses [notcurses](https://github.com/dankamongmen/notcurses), so images are displayed to the best of your terminal's abilities.

## Current state

Currently in early development, future versions may break backwards compatibility.
Many features are still not implemented, particularly when using the TUI instead of the CLI.

> **Warning**: this is all still somewhat buggy and not properly tested.
Do not delete your hydrus database to migrate to this program!

## Installation

### Requirements

+ GNU/Linux
+ [sqlite](https://www.sqlite.org/index.html)
+ [notcurses](https://github.com/dankamongmen/notcurses)
+ [xxHash](https://github.com/Cyan4973/xxHash)

### Building

To build from source, run:
```
make
```
To install, then run:
```
sudo make install
```
This will copy the executable to your path.

### Uninstallation

To uninstall, run:
```
sudo make uninstall
```

## Usage

See `nothydrus --help` for a full list and explanation of all commands.
In short, use:
+ `nothydrus init` to initialize the program at your desired path, creating a database
+ `nothydrus add` to add files to the database
+ `nothydrus` to enter the search TUI.

Once in the TUI, press:
+ 'o' to modify TUI options and key shortcuts. These are persistent per database
+ The up and down arrow keys to navigate menus
+ The enter key to select options
+ The 't' key or the 'Add new tag' option to add tags to the search
    + Enter and write a tag to search
    + 'q' to go back
    + Enter to select a tag
    + Space to select multiple tags in an OR search
    + Precede a search with '-' to exclude the tag
+ 'd' on a tag to delete it from the search
+ 'r' to run the search
+ 'f' to display the results

Once in display mode, use:
+ The left and right arrow keys to navigate the results
+ 'm' or 'e' to open the file with mpv
+ 't' to modify the file's tags
+ 'q' to quit
+ Configurable key shortcuts from the TUI options menu

### Importing from hydrus

In the 'scripts' folder, the `hydrus_txt_import.sh` shell script will add files to your nothydrus database and look for accompanying .txt sidecars.
Files can be exported from hydrus with their sidecars using 'right click' > 'share' > 'export files'.
For sidecar options, 'destination' should be 'a .txt sidecar' and the separator between tags should be a newline.

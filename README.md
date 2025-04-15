# Project Alice Map Editor
Tool to edit maps and according files required to create Project Alice scenarios.

# CAUTION: ALWAYS MAKE BACKUPS OF YOUR WORK
# CURRENTLY DOESN'T SUPPORT MULTIPLE BOOKMARKS

## Editor controls:
- wasd - camera shift
- qe - zoom

## How to start:

- Launch `editor.exe`.
- Choose paths to base game and mod folders.
- (Alternatively: Place vic2/mod files to default folders)
- Load and wait: depending on a mod loading might take some time

Inside the editor there are three main tools displayed on the left bar:

- Selection tool: allows to select provinces/nations on map
- Fill tool: choose any pixel and hold the `LMB` to paint provinces with a line. *BE CAREFUL WITH THIS TOOL AS THERE IS NO UNDO OPTION YET*
- Pick color tool: press pixel to pick nation and province to use during painting

Also there is an auxilarry tool: Context. Press `RMB` on any pixel to quickly select any of main tool or to perform one of additional actions.

## Explorers:
Currently there are two explorers: Provinces and nations.
National explorer allows you to create new nations based on currently selected nation. Do not forget to set their capital to a new province.

## Saving
At the right top corner there is a "Save" button. Saves are stored in the `editor-output` folder.
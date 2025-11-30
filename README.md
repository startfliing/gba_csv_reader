# gba_csv_reader
Import CSVs into usable C++ header files for (GBA) Application

[My Youtube Channel](https://www.youtube.com/@StartFliing)

## GBA Resources
Here are some resources I used/am using to learn about the GBA
- [devkitPro](https://devkitpro.org/)
- [Tonc](https://gbadev.net/tonc/foreword.html)
- [gbadev](https://gbadev.net/)

## Foreword
THIS IS NOT THE BEST C++ CODE EVER. IT IS FAR FROM PERFECT. THIS IS INTENDED AS A PROOF OF CONCEPT, RATHER THAN A FINAL PRODUCT. PLEASE TAKE THIS INTO CONSIDERATION!

I love feedback, criticisms, suggestions, comments, concerns, PRs, issues, and things of that nature.

I really reccomend using the `no$gba` emulator specifically for it's debugging tools. Extremely helpful for seeing tilesets and maps in the VRAM viewer while a game is running. It can be daunting at first, but I encourage you to explore some of it's other functionalities and tools as well.

I also have used a Windows machine for the development of this project. There might be differences for building this project on a Mac or Linux that I am not familiar with.

### Makefile

Important environment variables to set.

- DEVKITARM — path to devkitARM installation
- DEVKITPRO — path to devkitPro

In addition to `make` and `make clean`, I've added two custom options for `make`;

- `make pad` will pad the gba file to the nearest 4kb which might help if flashing the file to a GBA cart
- `make full` will run `make clean`, `make`, and then `make pad` in a row for a "full build" of the projects

## csv_reader
The `csv_reader.exe` app can be created by directly from `csv_reader.cpp` with a line like this in the project root
```
g++ csv_reader.cpp -o csv_reader.exe
```

If you are using devkitPro and your GBA development enviornment is already setup, you can add `csv_reader.exe` to this directory so it can be used like other tools, like grit.
```
C:\devkitPro\tools\bin
```

Also take a look at the Makefile to see how the CSV files are being collected to be used when you call `make` on the project.

To use the app for individual CSV's, you can use a command like this.
```
csv_reader.exe *path of CSV*
```

## Example CSV
This is a demo that utilizes my Terminal and Text-engine and an example CSV to test functionality. 

## pokedex
This is a demo that creates a "working" pokedex from a CSV with pokemon data. It uses a grayscale palette for the pokemon and utilizes Metatiles in the grit configuration to load all 1025 pokemon photos in one image.
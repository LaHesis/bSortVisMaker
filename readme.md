# bSortVisMaker

Bubble sort visualization maker -- a tool for creating visualizations of bubble (swapping) array sorting. It's written in C, uses libpng 1.6.28.1 to create animation frames, GraphicsMagick 1.3.35 to convert them into a gif file and gifsicle 1.92 for gifs optimization.

Requirements:

- **Windows 7 64/32 bit**. Although the program would work in newer Windows versions maybe.
- **GraphicsMagick** should be installed and available in cmd in order to automatically convert frames into a gif.
- **gifsicle** execution file should be at the program directory in order to automatically optimize gifs just after their creation.

Available options:

- `-n` - number of values to sort;
- `-w` - width of the image (in pixels);
- `-s` - whether to show stability (0 - show, 1 - not to show);
- `-m` - animation morph, i.e. additional transition frames;
- `-d` - delay between frames (in 1/100 second, probably between transition frames too);
- `-h` - print help information about the program and its options.

Features:

- choosing a number of array elements, image width, animation delay, animation morph and whether to show sorting stability;
- showing both elements comparisons and swaps;
- removing old frames before creating new animation;
- saving parameters into animation file name;
- easy to add other sorts (I think so).

**Microsoft Visual Studio Community 2019** was used along with **NuGet** to install libpng. libpng is used as a library. **GraphicsMagick** and **gifsicle** are used through `system` function call. To compile the code it is necessary to add new additional dependencies in the linker settings after installing libpng through **NuGet**. For example, line `$(ProjectDir)packages\libpng.1.6.28.1\build\native\lib\Win32\v140\dynamic\Debug\libpng16.lib`. Also zlib and libpng dll files should be placed into Debug/Release directories.
Not sorted array and sorted array frames are being duplicated in order to show them longer in animation (I tried other ways to implement it but failed, for example, denoting a delay for each frame results in too long command). Although a probably better solution was found that is denoting a long delay for the first frame, then normal delay for *.png and a long delay for the last frame. I.e., command `gm convert -delay 100 0.png -delay 10 *.png -delay 100 last.png`. But the idea is not implemented.

## Demo animations

`width_800  N_14  stability_shown  iterations_129  delay_7  morph_1 __ optimized.gif` (2.14 MB):

![width_800  N_12   iterations_100  delay_12  morph_1 __ optimized.gif](<https://github.com/LaHesis/bSortVisMaker/raw/master/demo/width_800  N_14  stability_shown  iterations_129  delay_7  morph_1 __ optimized.gif>)

`width_800  N_12   iterations_100  delay_12  morph_1 __ optimized.gif` (1.83 MB):

![width_800  N_12   iterations_100  delay_12  morph_1 __ optimized.gif](<https://github.com/LaHesis/bSortVisMaker/raw/master/demo/width_800  N_12   iterations_100  delay_12  morph_1 __ optimized.gif>)

There are longer animation examples [here](<https://github.com/LaHesis/bSortVisMaker/tree/master/demo/longer>).

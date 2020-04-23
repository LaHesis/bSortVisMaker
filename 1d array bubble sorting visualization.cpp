#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <png.h>

#define N 20
// Min is 2.
#define BLOCK_WIDTH 40
#define WITH_STABILITY_SHOWN 1
#define SORTING_IMG_DIR "sorting img"

#define MIN_LIGHTNESS 90
#define MAX_LIGHTNESS 145
#define STAB_LIGHTNESS 230

#define BORDER_ADDITION_MIN_WIDTH 7
#define BORDER_LIGHTNESS 20


enum stabilityColors {
    regularColor,
    red,
    green,
    blue,
};

typedef struct {
    enum stabilityColors stabColor;
    short value;
    short startPos;
} colorBlock;

void printArrayIntoImage(colorBlock*, short, short, short, char*);
void paintPixelWithColorRatio(unsigned char*, float);
void printArrayIntoConsole(colorBlock*);
void paintPixelWithColorRatio(unsigned char* pixelRGB, float colorRatio);
void shuffleArray(colorBlock*);
colorBlock* generateArray();
void bubbleSort(colorBlock*);


int main()
{
    system("chcp 1251");
    colorBlock* colorBlocks = generateArray();

    printf("initial:\n");
    printArrayIntoConsole(colorBlocks);

    shuffleArray(colorBlocks);
    printf("after shuffling:\n");
    printArrayIntoConsole(colorBlocks);
    
    char createDirCommand[30];
    sprintf_s(createDirCommand, 30, "mkdir \"%s\"", SORTING_IMG_DIR);
    system(createDirCommand);
    bubbleSort(colorBlocks);
    printf("after sortings:\n");
    printArrayIntoConsole(colorBlocks);
    return 0;
}

void printArrayIntoImage(colorBlock *blocks, short swapNum, short greaterSwappedBlockInd, short smallerSwappedBlockInd, char *fileDir) {
    FILE* fp;
    char fileName[100];
    sprintf_s(fileName, 100, "%s\\image%d.png", fileDir, swapNum);
    fopen_s(&fp, fileName, "wb");

    // Errors checking.
    if (!fp) {
        printf("failed to write into file");
        return;
    }
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("failed to create write struct");
        fclose(fp);
        return;
    }
    png_infop png_info;
    if (!(png_info = png_create_info_struct(png_ptr)) || setjmp(png_jmpbuf(png_ptr))) {
        printf("failed to create info struct");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return;
    }

    short width = N*BLOCK_WIDTH;
    short height = width / 2;
    png_set_IHDR(png_ptr, png_info, width, height, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    unsigned char** rows = (unsigned char**) malloc(sizeof(unsigned char*) * height);
    for (int i = 0; i < height; i++) {
        unsigned char *row = malloc(sizeof(unsigned char) * width * 3);
        rows[i] = row;
        for (short j = 0; j < width * 3; j++) {
            float color = j;
            color = 250;
            row[j] = (unsigned char) color;
        }
    }

    //It's how many pixel rows are in 1 value of colorBlock.
    short rowsInOneVal = (BLOCK_WIDTH / 2);

    //Border flags.
    unsigned char whetherShowBorders = BLOCK_WIDTH >= BORDER_ADDITION_MIN_WIDTH;
    unsigned char topBorderIsHere;
    unsigned char leftBorderIsHere;
    unsigned char rightBorderIsHere;
    unsigned char bottomBorderIsHere;
    unsigned char borderUnderCapIsHere;

    for (short blockInd = 0; blockInd < N; blockInd++) {
        colorBlock clrBl = *(blocks + blockInd);
        short rowAboveBlockTop = height - (clrBl.value + 1) * rowsInOneVal;
        short rowBelowBlockCap = rowAboveBlockTop + rowsInOneVal * .7 + 1;
        unsigned char isBlockToEmphasize = (greaterSwappedBlockInd == blockInd || smallerSwappedBlockInd == blockInd);
        for (short i = height - 1; i > rowAboveBlockTop; i--) {
            short blockStartPos = blockInd * BLOCK_WIDTH * 3;
            short blockEndPos = (blockInd + 1) * BLOCK_WIDTH * 3;

            //Horizontal borders.
            if (whetherShowBorders) {
                topBorderIsHere = (i == rowAboveBlockTop + 1);
                bottomBorderIsHere = (i == height - 1);
                borderUnderCapIsHere = (i == rowBelowBlockCap);
            }

            for (short j = blockStartPos; j < (blockInd + 1) * BLOCK_WIDTH * 3; j += 3) {
                unsigned char* pixelRGB = *(rows + i) + j;
                //Borders.
                if (whetherShowBorders) {
                    leftBorderIsHere = (j == blockStartPos && whetherShowBorders);
                    rightBorderIsHere = (j == (blockInd + 1) * BLOCK_WIDTH * 3 - 3 && whetherShowBorders);
                }
                if (leftBorderIsHere || rightBorderIsHere || topBorderIsHere || bottomBorderIsHere || borderUnderCapIsHere) {
                    *pixelRGB = BORDER_LIGHTNESS;
                    *(pixelRGB + 1) = BORDER_LIGHTNESS;
                    *(pixelRGB + 2) = BORDER_LIGHTNESS;
                } else {
                    // Color blocks are painted with gradient according to clrBl.value as a whole.
                    float colorRatio = clrBl.value / ((float)N - 1);
                    if (WITH_STABILITY_SHOWN) {
                    switch (clrBl.stabColor) {
                        case red:
                            *pixelRGB = STAB_LIGHTNESS;
                            *(pixelRGB + 1) = 0;
                            *(pixelRGB + 2) = 0;
                            break;
                        case green:
                            *pixelRGB = 0;
                            *(pixelRGB + 1) = STAB_LIGHTNESS;
                            *(pixelRGB + 2) = 0;
                            break;
                        case blue:
                            *pixelRGB = 0;
                            *(pixelRGB + 1) = 0;
                            *(pixelRGB + 2) = STAB_LIGHTNESS;
                            break;
                        default:
                            paintPixelWithColorRatio(pixelRGB, colorRatio);
                            break;
                        }
                    }
                    else {
                        paintPixelWithColorRatio(pixelRGB, colorRatio);
                    }

                    //Emphasize block cap.
                    if (i < rowBelowBlockCap) {
                        if (isBlockToEmphasize) {
                            *pixelRGB = 245;
                            *(pixelRGB + 1) = 200;
                            *(pixelRGB + 2) = 0;
                        } else {
                            *pixelRGB = 137;
                            *(pixelRGB + 1) = 169;
                            *(pixelRGB + 2) = 169;
                        }
                    }
                }
            }
        }
    }
    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, png_info, rows);
    png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(png_ptr, png_info);

    for (short y = 0; y < height; y++) {
        free(rows[y]);
    }
    free(rows);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, NULL);
}

void paintPixelWithColorRatio(unsigned char* pixelRGB, float colorRatio) {
    *pixelRGB = MAX_LIGHTNESS * colorRatio + MIN_LIGHTNESS;
    *(pixelRGB + 1) = MAX_LIGHTNESS * colorRatio + MIN_LIGHTNESS;
    *(pixelRGB + 2) = MAX_LIGHTNESS * colorRatio + MIN_LIGHTNESS;
}

void printArrayIntoConsole(colorBlock* ar) {
    for (short i = 0; i < N; i++) {
        printf("%.2d ", ar[i].value);
    }
    printf("\n");
}

colorBlock* generateArray() {
    colorBlock* colorBlocks = (colorBlock*)malloc(sizeof(colorBlock) * N);
    for (short cur_val = 0; cur_val < N; cur_val++) {
        colorBlock newBlock;
        newBlock.startPos = BLOCK_WIDTH * cur_val;
        newBlock.stabColor = regularColor;
        newBlock.value = cur_val;

        //Stability blocks preparing.
        if (WITH_STABILITY_SHOWN) {
            if (cur_val == N / 2) {
                newBlock.stabColor = red;
                newBlock.value = N / 2;
            }
            else if (cur_val == N / 2 - 1) {
                newBlock.stabColor = green;
                newBlock.value = N / 2;
            }
            else if (cur_val == N / 2 + 1) {
                newBlock.stabColor = blue;
                newBlock.value = N / 2;
            }
        }
        *(colorBlocks + cur_val) = newBlock;
    }
    return colorBlocks;
}

void shuffleArray(colorBlock* blocks) {
    unsigned int t = time(NULL);
    colorBlock tmp;

    for (int i = N - 1; i >= 1; i--) {
        srand(t + i);
        short r = rand() % (i + 1);
        //printf("r%d ", r);
        tmp = *(blocks + r);
        *(blocks + r) = *(blocks + i);
        *(blocks + i) = tmp;
    }

    //Stability blocks relocation.
    if (WITH_STABILITY_SHOWN) {
        for (int i = 0; i < N; i++) {
            tmp = *(blocks + i);
            switch (blocks[i].stabColor) {
            case red:
                *(blocks + i) = *blocks;
                *blocks = tmp;
                break;
            case green:
                *(blocks + i) = *(blocks + N / 2);
                *(blocks + N / 2) = tmp;
                break;
            case blue:
                *(blocks + i) = *(blocks + N - 1);
                *(blocks + N - 1) = tmp;
                break;
            }
        }
    }
}

void bubbleSort(colorBlock* blocks) {
    short iterNum = 0;
    printArrayIntoImage(blocks, iterNum++, -1, -1, SORTING_IMG_DIR);
    for (short j = 0; j < N; j++) {
        for (short n = 0; n < N - j - 1; n++) {
            printArrayIntoImage(blocks, iterNum++, n + 1, n, SORTING_IMG_DIR);
            colorBlock current = *(blocks + n);
            colorBlock next = *(blocks + n + 1);
            if (current.value > next.value) {
                colorBlock tmp = current;
                *(blocks + n) = next;
                *(blocks + n + 1) = tmp;
                printArrayIntoImage(blocks, iterNum++, n + 1, n, SORTING_IMG_DIR);
            }
        }
    }
    printArrayIntoImage(blocks, iterNum, -1, -1, SORTING_IMG_DIR);
}

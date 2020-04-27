﻿#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>
#include <time.h>
#include <png.h>

// Array element and color block amount.
#define N 35
// In pixels.
#define IMAGE_WIDTH 900
// Whether to show blocks with the same value
// but colored with red, green and blue.
#define WITH_STABILITY_SHOWN 1
// Where to save frames.
#define FRAMES_DIR "animation frames"
// Where to save animation files.
#define ANIMATIONS_DIR "animations"
// Delay between frames, in 1/100 second.
#define ANIMATION_DELAY "3"
// Transitions between frames.
#define ANIMATION_MORPH "3"

#define BACKGROUND_LIGHTNESS 250
#define MIN_LIGHTNESS 90
#define MAX_LIGHTNESS 145
#define STAB_LIGHTNESS 230

#define BORDER_ADDITION_MIN_WIDTH 7
#define BORDER_LIGHTNESS 20
const short BLOCK_WIDTH = IMAGE_WIDTH / N;
char quotedFramesDir[25];

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

void printArrayIntoPNGFrame(colorBlock* blocks, int, short, short);
void saveAnimation(int);
void printArrayIntoConsole(colorBlock*);
void paintPixel(unsigned char*, unsigned char, unsigned char, unsigned char);
void shuffleArray(colorBlock*);
colorBlock* generateArray();
int bubbleSortAndGetStepCount(colorBlock*);
unsigned char* encodeNumberIntoString(int);


int main() {
    system("chcp 1251");
    colorBlock* colorBlocks = generateArray();
    printf("initial:\n");
    printArrayIntoConsole(colorBlocks);

    shuffleArray(colorBlocks);
    printf("after shuffling:\n");
    printArrayIntoConsole(colorBlocks);
    
    sprintf_s(quotedFramesDir, 27, "\"%s\"", FRAMES_DIR);
    char removeDirCommand[35];
    sprintf_s(removeDirCommand, 35, "rmdir %s /S /Q", quotedFramesDir);
    system(removeDirCommand);

    char createDirCommand[35];
    sprintf_s(createDirCommand, 35, "mkdir %s", quotedFramesDir);
    system(createDirCommand);

    int stepsCount = bubbleSortAndGetStepCount(colorBlocks);
    printf("after sortings:\n");
    printArrayIntoConsole(colorBlocks);

    printf("creating animation from frames");
    saveAnimation(stepsCount);
    return 0;
}

unsigned char* encodeNumberIntoString(int num) {
    const short usedCharsCount = 36;
    short length = num / usedCharsCount + 1;
    unsigned char* string = (unsigned char*)malloc(sizeof(unsigned char) * (length + 1));
    int i = 0;
    while (num > usedCharsCount - 1) {
        string[i++] = 'Z';
        num -= usedCharsCount;
    }
    short remainder = num % usedCharsCount;
    if (remainder < 10) {
        // '0' - '9' mapping.
        string[i++] = (unsigned char) (48 + (remainder));
    } else if (remainder < 36) {
        // 'A' - 'Z' mapping.
        string[i++] = (unsigned char) (65 + (remainder - 10));
    }
    string[i] = '\0';
    return string;
}

void printArrayIntoPNGFrame(colorBlock *blocks, int frmNum, short greaterSwappedBlockInd, short smallerSwappedBlockInd) {
    unsigned char fileName[100];
    // If to use numbers for frame names there is problem with wrong frames order.
    unsigned char* encodedNum = encodeNumberIntoString(frmNum);
    sprintf_s(fileName, 100, "%s\\%s.png", FRAMES_DIR, encodedNum);
    
    FILE* fp;
    printf("trying to save step %d as \"%s\"\n", frmNum, fileName);
    fopen_s(&fp, fileName, "wb");
    free(encodedNum);

    // Errors checking.
    if (!fp) {
        printf("failed to write into file");
        exit(1);
    }
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        printf("failed to create write struct");
        fclose(fp);
        exit(1);
    }
    png_infop png_info;
    if (!(png_info = png_create_info_struct(png_ptr)) || setjmp(png_jmpbuf(png_ptr))) {
        printf("failed to create info struct");
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        exit(1);
    }
    
    // In pixels.
    short imgWidthComputed = N*BLOCK_WIDTH;
    short imgHeight = imgWidthComputed / 2;
    
    png_set_IHDR(png_ptr, png_info, imgWidthComputed, imgHeight, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);

    unsigned char** rows = (unsigned char**) malloc(sizeof(unsigned char*) * imgHeight);
    // Background drawing.
    for (int i = 0; i < imgHeight; i++) {
        unsigned char *row = malloc(sizeof(unsigned char) * imgWidthComputed * 3);
        rows[i] = row;
        for (short j = 0; j < imgWidthComputed * 3; j++) {
            float color = j;
            color = 250;
            row[j] = (unsigned char)color;
        }
    }

    // It's how many pixel rows are in 1 value of colorBlock.
    short rowsInOneVal = imgHeight / N;

    // Border flags.
    unsigned char whetherShowBorders = BLOCK_WIDTH >= BORDER_ADDITION_MIN_WIDTH;
    unsigned char topBorderIsHere = 0;
    unsigned char leftBorderIsHere = 0;
    unsigned char rightBorderIsHere = 0;
    unsigned char bottomBorderIsHere = 0;
    unsigned char borderUnderCapIsHere = 0;

    // Drawing each color block.
    for (short blockInd = 0; blockInd < N; blockInd++) {
        colorBlock clrBl = *(blocks + blockInd);
        short rowAboveBlockTop = imgHeight - (clrBl.value + 1) * rowsInOneVal;
        short rowBelowBlockCap = (short)(rowAboveBlockTop + rowsInOneVal * .7 + 1);
        unsigned char isBlockToEmphasize = (greaterSwappedBlockInd == blockInd || smallerSwappedBlockInd == blockInd);
        for (short i = imgHeight - 1; i > rowAboveBlockTop; i--) {
            short blockStartPos = blockInd * BLOCK_WIDTH * 3;
            short blockEndPos = (blockInd + 1) * BLOCK_WIDTH * 3;

            // Used for emphasizing swapping or comparing blocks by gradient fill.
            float verticalColorRatio = isBlockToEmphasize ? (imgHeight - i) / (float)(imgHeight - rowBelowBlockCap) : 0;
            // Color blocks are painted with gradient according to clrBl.value as a whole.
            float horizontalColorRatio = clrBl.value / ((float)N - 1);
            unsigned char clrVal = (unsigned char)MAX_LIGHTNESS * horizontalColorRatio + MIN_LIGHTNESS;

            // Horizontal borders.
            if (whetherShowBorders) {
                topBorderIsHere = (i == rowAboveBlockTop + 1);
                bottomBorderIsHere = (i == imgHeight - 1);
                borderUnderCapIsHere = (i == rowBelowBlockCap);
            }

            for (short j = blockStartPos; j < (blockInd + 1) * BLOCK_WIDTH * 3; j += 3) {
                unsigned char* pixelRGB = *(rows + i) + j;
                // Borders.
                if (whetherShowBorders) {
                    leftBorderIsHere = (j == blockStartPos && whetherShowBorders);
                    rightBorderIsHere = (j == (blockInd + 1) * BLOCK_WIDTH * 3 - 3 && whetherShowBorders);
                }
                if (leftBorderIsHere || rightBorderIsHere || topBorderIsHere || bottomBorderIsHere || borderUnderCapIsHere) {
                    paintPixel(pixelRGB, BORDER_LIGHTNESS, BORDER_LIGHTNESS, BORDER_LIGHTNESS);
                } else {
                    if (WITH_STABILITY_SHOWN) {
                    switch (clrBl.stabColor) {
                        case red:
                            paintPixel(pixelRGB, STAB_LIGHTNESS, 0, 0);
                            break;
                        case green:
                            paintPixel(pixelRGB, 0, STAB_LIGHTNESS, 0);
                            break;
                        case blue:
                            paintPixel(pixelRGB, 0, 0, STAB_LIGHTNESS);
                            break;
                        default:
                            paintPixel(pixelRGB, clrVal, clrVal, clrVal);
                            break;
                        }
                    }
                    else
                        paintPixel(pixelRGB, clrVal, clrVal, clrVal);

                    // Emphasize swapping or comparing blocks.
                    if (isBlockToEmphasize) {
                        *pixelRGB = (255 - MIN_LIGHTNESS) * verticalColorRatio + MIN_LIGHTNESS;
                        *(pixelRGB + 1) = (220 - MIN_LIGHTNESS) * verticalColorRatio + MIN_LIGHTNESS;
                        *(pixelRGB + 2) = MIN_LIGHTNESS;
                    }

                    // Emphasize block cap.
                    if (i < rowBelowBlockCap) {
                        if (isBlockToEmphasize) {
                            paintPixel(pixelRGB, 255, 255, 170);
                        } else {
                            paintPixel(pixelRGB, 137, 169, 169);
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

    for (short y = 0; y < imgHeight; y++) {
        free(rows[y]);
    }
    free(rows);
    fclose(fp);
    png_destroy_write_struct(&png_ptr, NULL);
}

void paintPixel(unsigned char* pixelRGB, unsigned char r, unsigned char g, unsigned char b) {
    *pixelRGB = r;
    *(pixelRGB + 1) = g;
    *(pixelRGB + 2) = b;
}

void saveAnimation(int stepsCount) {
    char createDirCommand[18];
    sprintf_s(createDirCommand, 18, "mkdir %s", ANIMATIONS_DIR);
    system(createDirCommand);

    // Parameters info.
    char imageWidth[11] = { 0 };
    sprintf_s(imageWidth, 11, "width_%d", IMAGE_WIDTH);
    char elementsAmount[18] = { 0 };
    sprintf_s(elementsAmount, 18, "%d_elements", N);
    char stability[16] = { 0 };
    if (WITH_STABILITY_SHOWN)
        sprintf_s(stability, 16, "stability_shown");
    char stepsCountString[25] = { 0 };
    sprintf_s(stepsCountString, 25, "%d_steps", stepsCount);

    char name[120] = { 0 };
    sprintf_s(name, 120, "%s %s %s %s", imageWidth, elementsAmount, stability, stepsCountString);

    char *createAnimationCommand[201];
    // GraphicsMagick utility "convert". I.e. GraphicsMagick should be installed.
    sprintf_s(createAnimationCommand, 201, "gm convert -morph %s -delay %s \"%s\\*.png\" \"%s\\%s.gif\"", ANIMATION_MORPH, ANIMATION_DELAY, FRAMES_DIR, ANIMATIONS_DIR, name);
    printf("\ncommand for creating animation: %s\n", createAnimationCommand);
    system(createAnimationCommand);
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

        // Stability blocks preparing.
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
        tmp = *(blocks + r);
        *(blocks + r) = *(blocks + i);
        *(blocks + i) = tmp;
    }

    // Stability blocks relocation.
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

int bubbleSortAndGetStepCount(colorBlock* blocks) {
    int stepNum = 0;
    printArrayIntoPNGFrame(blocks, stepNum++, -1, -1);
    printArrayIntoPNGFrame(blocks, stepNum++, -1, -1);
    printArrayIntoPNGFrame(blocks, stepNum++, -1, -1);
    for (short j = 0; j < N; j++) {
        for (short n = 0; n < N - j - 1; n++) {
            printArrayIntoPNGFrame(blocks, stepNum++, n + 1, n);
            colorBlock current = *(blocks + n);
            colorBlock next = *(blocks + n + 1);
            if (current.value > next.value) {
                colorBlock tmp = current;
                *(blocks + n) = next;
                *(blocks + n + 1) = tmp;
                printArrayIntoPNGFrame(blocks, stepNum++, n + 1, n);
            }
        }
    }
    printArrayIntoPNGFrame(blocks, stepNum++, -1, -1);
    printArrayIntoPNGFrame(blocks, stepNum++, -1, -1);
    printArrayIntoPNGFrame(blocks, stepNum, -1, -1);
    return stepNum - 5;
}

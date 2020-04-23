#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <png.h>

#define N 40
#define COLWIDTH 20
#define MINLIGHTNESS 30
#define MAXLIGHTNESS 150
#define STABLIGHTNESS 230

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

void printArrayIntoImage(colorBlock*, short);
void printArrayIntoConsole(colorBlock*);
void shuffleArray(colorBlock*);
colorBlock* generateArray();
void bubbleSort(colorBlock*);

int main()
{
    system("chcp 1251");
    system("mkdir img");
    colorBlock* colorBlocks = generateArray();
    printf("initial:\n");
    printArrayIntoConsole(colorBlocks);

    shuffleArray(colorBlocks);
    printf("after shuffling:\n");
    printArrayIntoConsole(colorBlocks);

    bubbleSort(colorBlocks);
    printf("after sortings:\n");
    printArrayIntoConsole(colorBlocks);
    return 0;
}

void printArrayIntoImage(colorBlock* ar, short swapNum) {
    FILE* fp;
    char fileName[100];
    sprintf_s(fileName, 100, "img\\image%d.png", swapNum);
    fopen_s(&fp, fileName, "wb");
    if (!fp) {
        return;
    }
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        goto closeFile;
    }

    png_infop png_info;
    if (!(png_info = png_create_info_struct(png_ptr))) {
        goto destroyWrite;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        goto destroyWrite;
    }

    short width = N*COLWIDTH;
    short height = width;
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

    for (short clBlockI = 0; clBlockI < N; clBlockI++) {
        colorBlock clrBl = *(ar + clBlockI);
        float colorRatio = clrBl.value / (float)N;
        for (short i = height - 1; i > height - (clrBl.value + 1) * COLWIDTH; i--) {
            for (short j = clBlockI * COLWIDTH * 3; j < (clBlockI + 1) * COLWIDTH * 3; j += 3) {
                unsigned char* pixelClr = *(rows + i) + j;

                switch (clrBl.stabColor) {
                case red:
                    *pixelClr = STABLIGHTNESS;
                    *(pixelClr + 1) = 0;
                    *(pixelClr + 2) = 0;
                    break;
                case green:
                    *pixelClr = 0;
                    *(pixelClr + 1) = STABLIGHTNESS;
                    *(pixelClr + 2) = 0;
                    break;
                case blue:
                    *pixelClr = 0;
                    *(pixelClr + 1) = 0;
                    *(pixelClr + 2) = STABLIGHTNESS;
                    break;
                default:
                    *pixelClr = MAXLIGHTNESS * colorRatio + MINLIGHTNESS;
                    *(pixelClr + 1) = MAXLIGHTNESS * colorRatio + MINLIGHTNESS;
                    *(pixelClr + 2) = MAXLIGHTNESS * colorRatio + MINLIGHTNESS;
                    break;
                }

                //Cap darkening.
                if (i < height - clrBl.value * COLWIDTH - COLWIDTH / 2) {
                    *pixelClr *= .5;
                    *(pixelClr + 1) *= .5;
                    *(pixelClr + 2) *= .5;
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

    destroyWrite:
        png_destroy_write_struct(&png_ptr, NULL);
    closeFile:
        fclose(fp);
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
        
        newBlock.startPos = COLWIDTH * cur_val;
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
        else {
            newBlock.stabColor = regularColor;
            newBlock.value = cur_val;
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

    //Stability markers relocation.
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

void bubbleSort(colorBlock* blocks) {
    short iterNum = 0;
    printArrayIntoImage(blocks, iterNum++);
    for (int j = 0; j < N; j++) {
        for (int n = 0; n < N - j - 1; n++) {
            colorBlock current = *(blocks + n);
            colorBlock next = *(blocks + n + 1);
            if (current.value > next.value) {
                colorBlock tmp = current;
                *(blocks + n) = next;
                *(blocks + n + 1) = tmp;

                printArrayIntoImage(blocks, iterNum++);
            }
        }
    }
}

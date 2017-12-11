#include <iostream>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>

using namespace std;
typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned int DWORD;
typedef int LONG;

#pragma pack(2)
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffbytes;
} BMPHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BMPINFO;

typedef struct tagRGBTRIPLE
{
    BYTE color;
} RGBTRIPLE;

BMPHEADER bmpHeader;
BMPINFO bmpInfo;
char* other;
RGBTRIPLE **BMPdata = NULL;
RGBTRIPLE **BMPoutput = NULL;
RGBTRIPLE **BMPoutput2 = NULL;

RGBTRIPLE **alloc_memory(int Y, int X)
{
    RGBTRIPLE **temp = new RGBTRIPLE*[Y];
    RGBTRIPLE *temp2 = new RGBTRIPLE[Y*X];
    memset(temp, 0, sizeof(RGBTRIPLE) * Y);
    memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);
    for(int i=0; i<Y; ++i)
        temp[i] = &temp2[i*X];
    return temp;
}

void readBMP(string fileName, int state)
{
    ifstream bmpFile(fileName, ios::in | ios::binary);
    if(!bmpFile)
    {
        cout << "Read " << fileName << " fails" << endl;
        exit(1);
    }
    bmpFile.read((char* )&bmpHeader, sizeof(BMPHEADER));
    if(bmpHeader.bfType != 0x4d42)
    {
        cout << "The " << fileName << " is not BMP" << endl ;
        exit(1);
    }
    if(1 == state)
    {
        bmpFile.read(( char* )&bmpInfo, sizeof(BMPINFO));
        other = (char*)malloc(sizeof(char)*(bmpHeader.bfOffbytes - 54));
        bmpFile.read((char*)&other, bmpHeader.bfOffbytes - 54);
        BMPdata = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
        bmpFile.read((char* )BMPdata[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);
    }
    else
    {
        bmpFile.seekg(bmpHeader.bfOffbytes, ios::beg);
        bmpFile.read((char* )BMPdata[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);
    }
    bmpFile.close();
    cout << "Read " << fileName << " successfully" << endl;
}

void saveBMP(string fileName, RGBTRIPLE **destination)
{
    ofstream newFile(fileName, ios:: out | ios::binary);
    if(!newFile)
    {
        cout << "Save " << fileName << " fails" << endl;
        exit(1);
    }
    newFile.write((char*)&bmpHeader, sizeof(BMPHEADER));
    newFile.write((char*)&bmpInfo, sizeof(BMPINFO));
    newFile.write((char*)&other, bmpHeader.bfOffbytes - 54);
    newFile.write((char*)destination[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);
    newFile.close();
    cout << "Save " << fileName << " successfully" << endl;
}

void Binary(string outfileName)
{
    BMPoutput = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
    BMPoutput2 = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
}

void clearall()
{
    for(int i=0; i<bmpInfo.biHeight; ++i)
        for(int j=0; j<bmpInfo.biWidth; ++j)
        {
            BMPoutput[i][j].color = 0;
            BMPoutput2[i][j].color = 0;
        }
}

void dilation(string outfileName, string type, RGBTRIPLE **source, RGBTRIPLE **destination)
{
    int i, j, x, y;
    for(i=bmpInfo.biHeight-1; i>-1; --i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            if(source[i][j].color)
            {
                for(x=i+2; x>i-3; --x)
                    for(y=j-2; y<j+3; ++y)
                    {
                        if(x == i+2 && y == j-2 || x == i+2 && y == j+2 || x == i-2 && y == j-2 || x == i-2 && y == j+2)
                            continue;
                        if(x > -1 && x < bmpInfo.biHeight && y > -1 && y < bmpInfo.biWidth && destination[x][y].color < source[i][j].color)
                            destination[x][y].color = source[i][j].color;
                    }
            }
    saveBMP(type + outfileName, destination);
}

void erosion(string outfileName, string type, RGBTRIPLE **source, RGBTRIPLE **destination)
{
    int i, j, x, y, _min = 255;
    for(i=bmpInfo.biHeight-1; i>-1; --i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            for(x=i+2; x>i-3; --x)
            {
                for(y=j-2; y<j+3; ++y)
                {
                    if(x == i+2 && y == j-2 || x == i+2 && y == j+2 || x == i-2 && y == j-2 || x == i-2 && y == j+2)
                        continue;
                    if(x < 0 || x >= bmpInfo.biHeight || y < 0 || y >= bmpInfo.biWidth || !source[x][y].color)
                        goto fail;
                    if(source[x][y].color < _min)
                        _min = source[x][y].color;
                }
            }
            destination[i][j].color = _min;
fail:
            _min = 255;
        }
    saveBMP(type + outfileName, destination);
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cout << "Please give input filename" << endl;
        return 0;
    }
    string infileName = argv[1];
    readBMP(infileName, 1);
    Binary(infileName);
    clearall();
    dilation(infileName, "dilation_", BMPdata, BMPoutput);
    erosion(infileName, "closing_", BMPoutput, BMPoutput2);
    clearall();
    erosion(infileName, "erosion_", BMPdata, BMPoutput);
    dilation(infileName, "opening_", BMPoutput, BMPoutput2);
    return 0;
}

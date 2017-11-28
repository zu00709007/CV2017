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

void readBMP(string fileName)
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
    bmpFile.read(( char* )&bmpInfo, sizeof(BMPINFO));
    other = (char*)malloc(sizeof(char)*(bmpHeader.bfOffbytes - 54));
    bmpFile.read((char*)&other, bmpHeader.bfOffbytes - 54);
    BMPdata = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
    bmpFile.read((char* )BMPdata[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);
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

void Binary_binary(unsigned char binary[][514])
{
    int i, j;
    for(i=bmpInfo.biHeight-1; i>-1; --i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            binary[512-i][1+j] = (BMPdata[i][j].color < 128 ? 0 : 1);
}

void Mark_Interior_Border(unsigned char bordermap[][514], unsigned char binary[][514])
{
    int i, j;
    for(i=0; i<514; ++i)
        for(j=0; j<514; ++j)
        {
            if(binary[i][j])
            {
                if(binary[i+1][j] && binary[i-1][j] && binary[i][j+1] && binary[i][j-1])
                    bordermap[i][j] = 2;
                else
                    bordermap[i][j] = 1;
            }
            else
                bordermap[i][j] = 0;
        }
}

void Pair_Relationship(unsigned char bordermap[][514], unsigned char pairmatrix[][514])
{
    int i, j;
    for(i=0; i<514; ++i)
        for(j=0; j<514; ++j)
            if(1 != bordermap[i][j] || (2 != bordermap[i-1][j] && 2 != bordermap[i+1][j] && 2 != bordermap[i][j-1] && 2 != bordermap[i][j+1]))
                pairmatrix[i][j] = 'q';
            else
                pairmatrix[i][j] = 'p';
}

void Connected_Shrink(unsigned char pairmatrix[][514], unsigned char binary[][514], bool* check)
{
    int i, j, c;
    for(j=0; j<514; ++j)
        for(i=0; i<514; ++i)
            if('p' == pairmatrix[i][j])
            {
                c = 0;
                if((binary[i-1][j]) && (!binary[i-1][j-1] || !binary[i][j-1]))
                    ++c;
                if((binary[i][j+1]) && (!binary[i-1][j+1] || !binary[i-1][j]))
                    ++c;
                if((binary[i][j-1]) && (!binary[i+1][j-1] || !binary[i+1][j]))
                    ++c;
                if((binary[i+1][j]) && (!binary[i+1][j+1] || !binary[i][j+1]))
                    ++c;
                if(1 == c)
                {
                    (*check) = true;
                    binary[i][j] = 0;
                }
            }
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        cout << "Please give input filename" << endl;
        return 0;
    }
    string infileName = argv[1];
    readBMP(infileName);
    unsigned char binary[514][514] = {0}, bordermap[514][514], pairmatrix[514][514];
    Binary_binary(binary);

    bool check = true;
    while(check)
    {
        check = false;
        Mark_Interior_Border(bordermap,binary);
        Pair_Relationship(bordermap, pairmatrix);
        Connected_Shrink(pairmatrix, binary, &check);
    }

    for(int i=1; i<513; ++i)
        for(int j=1; j<513; ++j)
        {
            if(binary[i][j])
                BMPdata[512-i][j-1].color = 255;
            else
                BMPdata[512-i][j-1].color = 0;
        }
    saveBMP("thinning_" + infileName, BMPdata);
    return 0;
}

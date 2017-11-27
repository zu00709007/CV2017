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

void Binary_Scale(unsigned char scale[][514])
{
    int i, j;
    for(i=bmpInfo.biHeight-1; i>-1; --i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            scale[512-i][1+j] = (BMPdata[i][j].color < 128 ? 0 : 1);
}

void Yokoi_Connectivity(unsigned char output[][514], unsigned char scale[][514])
{
    int i, j, r, q, up, down, left, right;
    for(i=0; i<514; ++i)
        for(j=0; j<514; ++j)
        {
            if(scale[i][j])
            {
                r = 0;
                q = 0;
                up = i>0 ? i-1 : 0;
                down = i<513 ? i+1 : 513;
                left = j>0 ? j-1 : 0;
                right = j<513 ? j+1 : 513;

                if(scale[up][j])
                {
                    if(scale[up][left]  && scale[i][left])
                        ++r;
                    else
                        ++q;
                }
                if(scale[i][right])
                {
                    if(scale[up][right] && scale[up][j])
                        ++r;
                    else
                        ++q;
                }
                if(scale[i][left])
                {
                    if(scale[down][left] && scale[down][j])
                        ++r;
                    else
                        ++q;
                }
                if(scale[down][j])
                {
                    if(scale[down][right] && scale[i][right])
                        ++r;
                    else
                        ++q;
                }
                if(r == 4)
                    output[i][j] = 5;
                else
                    output[i][j] = q;
            }
            else
                output[i][j] = 0;
        }
}

void Pair_Relationship(unsigned char output[][514], unsigned char pairmatrix[][514])
{
    int i, j;
    for(i=0; i<514; ++i)
        for(j=0; j<514; ++j)
            if(1 == output[i][j] && (1 == output[i-1][j] || 1 == output[i+1][j] || 1 == output[i][j-1] || 1 == output[i][j+1] || 1 == output[i-1][j-1] || 1 == output[i+1][j+1] || 1 == output[i+1][j-1] || 1 == output[i-1][j+1]))
                pairmatrix[i][j]='p';
            else
                pairmatrix[i][j]='q';
}

void Connected_Shrink(unsigned char pairmatrix[][514], unsigned char scale[][514], bool* check)
{
    int i, j, c;
    for(i=0; i<514; ++i)
        for(j=0; j<514; ++j)
            if('p' == pairmatrix[i][j])
            {
                c = 0;
                (*check) = true;
                if((scale[i-1][j]) && (!scale[i-1][j-1] || !scale[i][j-1]))
                    ++c;
                if((scale[i][j+1]) && (!scale[i-1][j+1] || !scale[i-1][j]))
                    ++c;
                if((scale[i][j-1]) && (!scale[i+1][j-1] || !scale[i+1][j]))
                    ++c;
                if((scale[i+1][j]) && (!scale[i+1][j+1] || !scale[i][j+1]))
                    ++c;
                if(1 == c)
                    scale[i][j] = 0;
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
    unsigned char scale[514][514] = {0}, output[514][514], pairmatrix[514][514];
    Binary_Scale(scale);

    bool check = true;
    while(check)
    {
        check = false;
        Yokoi_Connectivity(output,scale);
        Pair_Relationship(output, pairmatrix);
        Connected_Shrink(pairmatrix, scale, &check);
    }

    for(int i=1; i<513; ++i)
        for(int j=1; j<513; ++j)
        {
            if(scale[i][j])
                BMPdata[512-i][j].color = 255;
            else
                BMPdata[512-i][j].color = 0;
        }
    saveBMP("thinning_" + infileName, BMPdata);
    return 0;
}

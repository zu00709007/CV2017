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

void Binary_Scale(int scale[][66])
{
    int i, j;
    for(i=bmpInfo.biHeight-1; i>-1; i-=8)
        for(j=0; j<bmpInfo.biWidth; j+=8)
            scale[64-(i>>3)][1+(j>>3)] = (BMPdata[i][j].color < 128 ? 0 : 1);
}

void Yokoi_Connectivity(int scale[][66])
{
    int i, j, r, q, up, down, left, right, output[66][66] = {0};
    for(i=0; i<66; ++i)
        for(j=0; j<66; ++j)
        {
            if(scale[i][j])
            {
                r = 0;
                q = 0;
                up = i>0 ? i-1 : 0;
                down = i<65 ? i+1 : 65;
                left = j>0 ? j-1 : 0;
                right = j<65 ? j+1 : 65;
                if(scale[up][j])
                {
                    if(scale[up][left] && scale[i][left])
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
                    if(scale[down][right] && scale[down][j])
                        ++r;
                    else
                        ++q;
                }
                if(r == 4)
                    output[i][j] = 5;
                else
                    output[i][j] = q;
            }
        }
    FILE* fptr = fopen("Yokoi","w");
    for(i=1; i<64; ++i)
    {
        for(j=1; j<65; ++j)
        {
            if(!output[i][j])
                fprintf(fptr, " ");
            else
                fprintf(fptr, "%d", output[i][j]);
        }
        fprintf(fptr, "\n");
    }
    for(j=1; j<65; ++j)
    {
        if(!output[i][j])
            fprintf(fptr, " ");
        else
            fprintf(fptr, "%d", output[i][j]);
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
    int scale[66][66] = {0};
    Binary_Scale(scale);
    Yokoi_Connectivity(scale);
    return 0;
}

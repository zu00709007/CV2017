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
    memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X );
    for(int i = 0; i < Y; ++i)
    {
        temp[i] = &temp2[i*X];
    }
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

void saveBMP(string fileName)
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
    newFile.write((char*)BMPdata[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);
    newFile.close();
    cout << "Save " << fileName << " successfully" << endl;
}

void Binary_Histogram(string outfileName)
{
    int i, j, max = 0, histogram[256] = {0}, histogram2[256] = {0};
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            ++histogram[BMPdata[i][j].color];

    for(i=0; i<256; ++i)
        max = max > histogram[i] ? max : histogram[i];
    for(i=0; i<256; ++i)
        histogram[i] = histogram[i] * 512 / max ;	
	for(i=1; i<256; ++i)
		histogram2[i] = (histogram[i] + histogram[i-1]) >> 1;
	histogram2[0] = histogram[0] >> 1;
	
    for(i=0; i<512; ++i)
        for(j=0; j<256; ++j)
        {
            if(histogram[j] > 0)
            {
                BMPdata[i][(j<<1)+1].color = 0;
                --histogram[j];
            }
            else
                BMPdata[i][(j<<1)+1].color = 255;
			if(histogram2[j] > 0)
            {
                BMPdata[i][j<<1].color = 0;
                --histogram2[j];
            }
            else
                BMPdata[i][j<<1].color = 255;
        }
    saveBMP("histogram_" + outfileName);
}

void Equalization_Histogram(string outfileName)
{
	int i, j, max = 0, histogram[256] = {0}, pdf[256] = {0};
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            ++histogram[BMPdata[i][j].color];

    pdf[0] = histogram[0];
    for(i=1; i<256; ++i)
        pdf[i] = histogram[i] + pdf[i-1];
	
	for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
			BMPdata[i][j].color = pdf[BMPdata[i][j].color] * 255 / pdf[255];
	
	saveBMP("equalization_" + outfileName);

	for(i=0; i<256; ++i)
        histogram[i] = 0;	
	for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            ++histogram[BMPdata[i][j].color];
    for(i=0; i<256; ++i)
        max = max > histogram[i] ? max : histogram[i];
    for(i=0; i<256; ++i)
        histogram[i] = histogram[i] * 512 / max ;	
	
    for(i=0; i<512; ++i)
        for(j=0; j<256; ++j)
        {
            if(histogram[j] > 0)
            {
                BMPdata[i][(j<<1)+1].color = 0;
				BMPdata[i][j<<1].color = 0;
                --histogram[j];
            }
            else
			{
				BMPdata[i][(j<<1)+1].color = 255;
				BMPdata[i][(j<<1)].color = 255;
			}             
        }
    saveBMP("histogram2_" + outfileName);
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
    Binary_Histogram(infileName);
	readBMP(infileName, 2);
    Equalization_Histogram(infileName);
    return 0;
}
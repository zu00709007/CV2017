#include <iostream>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include<algorithm>

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
RGBTRIPLE **BMPoutput_data = NULL;

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

void Robert(int threshold)
{
    int i, j, r1, r2;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            r1 = tmp[i-1][j+1] - tmp[i][j];
            r2 = tmp[i-1][j] - tmp[i][j+1];
            BMPoutput_data[i-1][j-1].color = (sqrt(pow(r1, 2) + pow(r2, 2)) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("robert_" + string(buf) + ".bmp", BMPoutput_data);
}

void Prewitt(int threshold)
{
    int i, j, p1, p2;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            p1 = tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1] - tmp[i+1][j-1] - tmp[i+1][j] - tmp[i+1][j+1];
            p2 = tmp[i-1][j+1] + tmp[i][j+1] + tmp[i+1][j+1] - tmp[i-1][j-1] - tmp[i][j-1] - tmp[i+1][j-1];
            BMPoutput_data[i-1][j-1].color = (sqrt(pow(p1, 2) + pow(p2, 2)) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("prewitt_" + string(buf) + ".bmp", BMPoutput_data);
}

void Sobel(int threshold)
{
    int i, j, s1, s2;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            s1 = tmp[i-1][j-1] + 2 * tmp[i-1][j] + tmp[i-1][j+1] - tmp[i+1][j-1] - 2 * tmp[i+1][j] - tmp[i+1][j+1];
            s2 = tmp[i-1][j+1] + 2 * tmp[i][j+1] + tmp[i+1][j+1] - tmp[i-1][j-1] - 2 * tmp[i][j-1] - tmp[i+1][j-1];
            BMPoutput_data[i-1][j-1].color = (sqrt(pow(s1, 2) + pow(s2, 2)) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("sobel_" + string(buf) + ".bmp", BMPoutput_data);
}

void Frei_Chen(int threshold)
{
    int i, j, f1, f2;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            f1 = tmp[i-1][j-1] + sqrt(2 * tmp[i-1][j] * tmp[i-1][j]) + tmp[i-1][j+1] - tmp[i+1][j-1] - sqrt(2 * tmp[i+1][j] * tmp[i+1][j]) - tmp[i+1][j+1];
            f2 = tmp[i-1][j+1] + sqrt(2 * tmp[i][j+1] * tmp[i][j+1]) + tmp[i+1][j+1] - tmp[i-1][j-1] - sqrt(2 * tmp[i][j-1] * tmp[i][j-1]) - tmp[i+1][j-1];
            BMPoutput_data[i-1][j-1].color = (sqrt(pow(f1, 2) + pow(f2, 2)) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("frei&chen_" + string(buf) + ".bmp", BMPoutput_data);
}

void Kirsch(int threshold)
{
    int i, j, k[8];
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            k[0] = 5 * (tmp[i-1][j+1] + tmp[i][j+1] + tmp[i+1][j+1]) - 3 * (tmp[i+1][j] + tmp[i+1][j-1] + tmp[i][j-1] + tmp[i-1][j-1] + tmp[i-1][j]);
            k[1] = 5 * (tmp[i-1][j] + tmp[i-1][j+1] + tmp[i][j+1]) - 3 * (tmp[i+1][j+1] + tmp[i+1][j] + tmp[i+1][j-1] + tmp[i][j-1] + tmp[i-1][j-1]);
            k[2] = 5 * (tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1]) - 3 * (tmp[i][j+1] + tmp[i+1][j+1] + tmp[i+1][j] + tmp[i+1][j-1] + tmp[i][j-1]);
            k[3] = 5 * (tmp[i][j-1] + tmp[i-1][j-1] + tmp[i-1][j]) - 3 * (tmp[i-1][j+1] + tmp[i][j+1] + tmp[i+1][j+1] + tmp[i+1][j] + tmp[i+1][j-1]);
            k[4] = 5 * (tmp[i+1][j-1] + tmp[i][j-1] + tmp[i-1][j-1]) - 3 * (tmp[i-1][j] + tmp[i-1][j+1] + tmp[i][j+1] + tmp[i+1][j+1] + tmp[i+1][j]);
            k[5] = 5 * (tmp[i+1][j] + tmp[i+1][j-1] + tmp[i][j-1]) - 3 * (tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1] + tmp[i][j+1] + tmp[i+1][j+1]);
            k[6] = 5 * (tmp[i+1][j+1] + tmp[i+1][j] + tmp[i+1][j-1]) - 3 * (tmp[i][j-1] + tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1] + tmp[i][j+1]);
            k[7] = 5 * (tmp[i][j+1] + tmp[i+1][j+1] + tmp[i+1][j]) - 3 * (tmp[i+1][j-1] + tmp[i][j-1] + tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1]);
            BMPoutput_data[i-1][j-1].color = (*max_element(k, k+8) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("kirsch_" + string(buf) + ".bmp", BMPoutput_data);
}

void Robinson(int threshold)
{
    int i, j, r[8];
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=2; i<bmpInfo.biHeight; ++i)
        for(j=2; j<bmpInfo.biWidth; ++j)
        {
            r[0] = tmp[i-1][j+1] + 2 * tmp[i][j+1] + tmp[i+1][j+1] - tmp[i+1][j-1] - 2 * tmp[i][j-1] - tmp[i-1][j-1];
            r[1] = tmp[i-1][j] + 2 * tmp[i-1][j+1] + tmp[i][j+1] - tmp[i+1][j] - 2 * tmp[i+1][j-1] - tmp[i][j-1];
            r[2] = tmp[i-1][j-1] + 2 * tmp[i-1][j] + tmp[i-1][j+1] - tmp[i+1][j+1] - 2 * tmp[i+1][j] - tmp[i+1][j-1];
            r[3] = tmp[i][j-1] + 2 * tmp[i-1][j-1] + tmp[i-1][j] - tmp[i][j+1] - 2 * tmp[i+1][j+1] - tmp[i+1][j];
            r[4] = tmp[i+1][j-1] + 2 * tmp[i][j-1] + tmp[i-1][j-1] - tmp[i-1][j+1] - 2 * tmp[i][j+1] - tmp[i+1][j+1];
            r[5] = tmp[i+1][j] + 2 * tmp[i+1][j-1] + tmp[i][j-1] - tmp[i-1][j] - 2 * tmp[i-1][j+1] - tmp[i][j+1];
            r[6] = tmp[i+1][j+1] + 2 * tmp[i+1][j] + tmp[i+1][j-1] - tmp[i-1][j-1] - 2 * tmp[i-1][j] - tmp[i-1][j+1];
            r[7] = tmp[i][j+1] + 2 * tmp[i+1][j+1] + tmp[i+1][j] - tmp[i][j-1] - 2 * tmp[i-1][j-1] - tmp[i-1][j];
            BMPoutput_data[i-1][j-1].color = (*max_element(r, r+8) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("robinson_" + string(buf) + ".bmp", BMPoutput_data);
}

void Nevatia_Babu(int threshold)
{
    int i, j, n[6];
    unsigned char tmp[bmpInfo.biHeight+4][bmpInfo.biWidth+4]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+2][j+2] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=4; i<bmpInfo.biHeight+2; ++i)
        for(j=4; j<bmpInfo.biWidth; ++j)
        {
            n[0] = 0 - 100 * tmp[i-2][j-2] - 100 * tmp[i-2][j-1] - 100 * tmp[i-2][j] - 100 * tmp[i-2][j+1] - 100 * tmp[i-2][j+2] -
                   100 * tmp[i-1][j-2] - 100 * tmp[i-1][j-1] - 100 * tmp[i-1][j] - 100 * tmp[i-1][j+1] - 100 * tmp[i-1][j+2] +
                   100 * tmp[i+1][j-2] + 100 * tmp[i+1][j-1] + 100 * tmp[i+1][j] + 100 * tmp[i+1][j+1] + 100 * tmp[i+1][j+2] +
                   100 * tmp[i+2][j-2] + 100 * tmp[i+2][j-1] + 100 * tmp[i+2][j] + 100 * tmp[i+2][j+1] + 100 * tmp[i+2][j+2];

            n[1] = 0 - 100 * tmp[i-2][j-2] - 100 * tmp[i-2][j-1] - 100 * tmp[i-2][j] - 100 * tmp[i-2][j+1] - 100 * tmp[i-2][j+2] +
                   32 * tmp[i-1][j-2] - 78 * tmp[i-1][j-1] - 100 * tmp[i-1][j] - 100 * tmp[i-1][j+1] - 100 * tmp[i-1][j+2] +
                   100 * tmp[i][j-2] + 92 * tmp[i][j-1] - 92 * tmp[i][j+1] - 100 * tmp[i][j+2] +
                   100 * tmp[i+1][j-2] + 100 * tmp[i+1][j-1] + 100 * tmp[i+1][j] + 78 * tmp[i+1][j+1] - 32 * tmp[i+1][j+2] +
                   100 * tmp[i+2][j-2] + 100 * tmp[i+2][j-1] + 100 * tmp[i+2][j] + 100 * tmp[i+2][j+1] + 100 * tmp[i+2][j+2];

            n[2] = 100 * tmp[i-2][j-2] - 32 * tmp[i-2][j-1] - 100 * tmp[i-2][j] - 100 * tmp[i-2][j+1] - 100 * tmp[i-2][j+2] +
                   100 * tmp[i-1][j-2] + 78 * tmp[i-1][j-1] - 92 * tmp[i-1][j] - 100 * tmp[i-1][j+1] - 100 * tmp[i-1][j+2] +
                   100 * tmp[i][j-2] + 100 * tmp[i][j-1] - 100 * tmp[i][j+1] - 100 * tmp[i][j+2] +
                   100 * tmp[i+1][j-2] + 100 * tmp[i+1][j-1] + 92 * tmp[i+1][j] - 78 * tmp[i+1][j+1] - 100 * tmp[i+1][j+2] +
                   100 * tmp[i+2][j-2] + 100 * tmp[i+2][j-1] + 100 * tmp[i+2][j] + 32 * tmp[i+2][j+1] - 100 * tmp[i+2][j+2];

            n[3] = 0 - 100 * tmp[i-2][j-2] - 100 * tmp[i-2][j-1] + 100 * tmp[i-2][j+1] + 100 * tmp[i-2][j+2] -
                   100 * tmp[i-1][j-2] - 100 * tmp[i-1][j-1] + 100 * tmp[i-1][j+1]+ 100 * tmp[i-1][j+2] -
                   100 * tmp[i][j-2] - 100 * tmp[i][j-1] + 100 * tmp[i][j+1] + 100 * tmp[i][j+2] -
                   100 * tmp[i+1][j-2] - 100 * tmp[i+1][j-1] + 100 * tmp[i+1][j+1] + 100 * tmp[i+1][j+2] -
                   100 * tmp[i+2][j-2] + -100 * tmp[i+2][j-1] + 0 * tmp[i+2][j] + 100 * tmp[i+2][j+1]+ 100 * tmp[i+2][j+2];

            n[4] = 0 - 100 * tmp[i-2][j-2] - 100 * tmp[i-2][j-1] - 100 * tmp[i-2][j] - 32 * tmp[i-2][j+1] + 100 * tmp[i-2][j+2] -
                   100 * tmp[i-1][j-2] - 100 * tmp[i-1][j-1] - 92 * tmp[i-1][j] + 78 * tmp[i-1][j+1] + 100 * tmp[i-1][j+2] -
                   100 * tmp[i][j-2] - 100 * tmp[i][j-1] + 100 * tmp[i][j+1]+ 100 * tmp[i][j+2] -
                   100 * tmp[i+1][j-2] - 78 * tmp[i+1][j-1] + 92 * tmp[i+1][j] + 100 * tmp[i+1][j+1] + 100 * tmp[i+1][j+2] -
                   100 * tmp[i+2][j-2] + 32 * tmp[i+2][j-1] + 100 * tmp[i+2][j] + 100 * tmp[i+2][j+1] + 100 * tmp[i+2][j+2];

            n[5] = 0 - 100 * tmp[i-2][j-2] - 100 * tmp[i-2][j-1] - 100 * tmp[i-2][j] - 100 * tmp[i-2][j+1] - 100 * tmp[i-2][j+2] -
                   100 * tmp[i-1][j-2] - 100 * tmp[i-1][j-1] - 100 * tmp[i-1][j] - 78 * tmp[i-1][j+1] + 32 * tmp[i-1][j+2] -
                   100 * tmp[i][j-2] - 92 * tmp[i][j-1] + 92 * tmp[i][j+1] + 100 * tmp[i][j+2] -
                   32 * tmp[i+1][j-2] + 78 * tmp[i+1][j-1] + 100 * tmp[i+1][j] + 100 * tmp[i+1][j+1] + 100 * tmp[i+1][j+2] +
                   100 * tmp[i+2][j-2] + 100 * tmp[i+2][j-1] + 100 * tmp[i+2][j] + 100 * tmp[i+2][j+1] + 100 * tmp[i+2][j+2];

            BMPoutput_data[i-2][j-2].color = (*max_element(n, n+6) > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("nevatia_babu_5X5_" + string(buf) + ".bmp", BMPoutput_data);
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
    BMPoutput_data = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    Robert(12);
    Prewitt(24);
    Sobel(38);
    Frei_Chen(30);
    Kirsch(135);
    Robinson(43);
    Nevatia_Babu(12500);

    return 0;
}

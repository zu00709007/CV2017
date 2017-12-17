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

void Laplace1(int threshold)
{
    int i, j, p;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=1; i<=bmpInfo.biHeight; ++i)
        for(j=1; j<=bmpInfo.biWidth; ++j)
        {
            p = tmp[i-1][j] + tmp[i+1][j] + tmp[i][j-1] + tmp[i][j+1] - 4 * tmp[i][j];
            BMPoutput_data[i-1][j-1].color = (p > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("laplace1_" + string(buf) + ".bmp", BMPoutput_data);
}

void Laplace2(int threshold)
{
    int i, j, p;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=1; i<=bmpInfo.biHeight; ++i)
        for(j=1; j<=bmpInfo.biWidth; ++j)
        {
            p = tmp[i-1][j-1] + tmp[i-1][j] + tmp[i-1][j+1] + tmp[i][j-1] - 8 * tmp[i][j] + tmp[i][j+1] + tmp[i+1][j-1] + tmp[i+1][j] + tmp[i+1][j+1];
            BMPoutput_data[i-1][j-1].color = (p / 3 > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("laplace2_" + string(buf) + ".bmp", BMPoutput_data);
}

void Minimum_Variance_Laplacian(int threshold)
{
    int i, j, p;
    unsigned char tmp[bmpInfo.biHeight+2][bmpInfo.biWidth+2]= {0};
    char buf[10];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            tmp[i+1][j+1] = BMPdata[i][j].color;
            BMPoutput_data[i][j].color = 255;
        }

    for(i=1; i<=bmpInfo.biHeight; ++i)
        for(j=1; j<=bmpInfo.biWidth; ++j)
        {
            p = 2 * tmp[i-1][j-1] - tmp[i-1][j] + 2 * tmp[i-1][j+1] - tmp[i][j-1] - 4 * tmp[i][j] - tmp[i][j+1] + 2 * tmp[i+1][j-1] - tmp[i+1][j] + 2 * tmp[i+1][j+1];
            BMPoutput_data[i-1][j-1].color = (p / 3 > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("minimum_variance_laplacian_" + string(buf) + ".bmp", BMPoutput_data);
}

double Gaussian(double x, double y, double sigma)
{
    return exp(x * x / (-2 * sigma * sigma)) * exp(y * y / (-2 * sigma * sigma)) / (2.0 * M_PI * sigma * sigma);
}

void Difference_Gaussian(int threshold, int kernal, double inhibitory_sigma, double excitatory_sigma)
{
    int i, j, k, l, total, border = kernal >> 1, DoG[kernal][kernal];
    char buf[10];
    unsigned char tmp[bmpInfo.biHeight+kernal-1][bmpInfo.biWidth+kernal-1]= {0};

    for(i=0-border; i<=border; ++i)
        for(j=0-border; j<=border; ++j)
            DoG[i+border][j+border] = (2000.0 * (Gaussian(i, j, inhibitory_sigma) - Gaussian(i, j, excitatory_sigma))) + 0.5;

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            tmp[i+border][j+border] = BMPdata[i][j].color;

    for(i=border; i<bmpInfo.biHeight+border; ++i)
        for(j=border; j<bmpInfo.biWidth+border; ++j)
        {
            total = 0;
            for(k=0-border; k<=border; ++k)
                for(l=0-border; l<=border; ++l)
                    total += DoG[border+k][border+l] * tmp[i+k][j+l];
            BMPoutput_data[i-border][j-border].color = (total > threshold) ? 255 : 0;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("difference_gaussian_" + string(buf) + ".bmp", BMPoutput_data);
}

void Laplacian_Gaussian(int threshold, int kernal, double sigma)
{
    int i, j, k, l, total, border = kernal >> 1, LoG[kernal][kernal];
    char buf[10];
    unsigned char tmp[bmpInfo.biHeight+kernal-1][bmpInfo.biWidth+kernal-1]= {0};

    for(i=0-border; i<=border; ++i)
        for(j=0-border; j<=border; ++j)
        {
            double tmp1 = -175 * (((double)(i * i) + (j * j) - 2 * sigma * sigma) * exp(((double)(i * i) + (j * j)) / (-2 * sigma * sigma)) / (sigma * sigma * sigma * sigma));
            LoG[i+border][j+border] = tmp1 > 0 ? tmp1 + 0.5 : tmp1 - 0.5;
        }
    --LoG[border][border];

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            tmp[i+border][j+border] = BMPdata[i][j].color;

    for(i=border; i<bmpInfo.biHeight+border; ++i)
        for(j=border; j<bmpInfo.biWidth+border; ++j)
        {
            total = 0;
            for(k=0-border; k<=border; ++k)
                for(l=0-border; l<=border; ++l)
                    total += tmp[i+k][j+l] * LoG[border+k][border+l];
            BMPoutput_data[i-border][j-border].color = (total > threshold) ? 0 : 255;
        }

    sprintf(buf, "%d", threshold);
    saveBMP("laplacian_gaussian_" + string(buf) + ".bmp", BMPoutput_data);
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

    Laplace1(15);
    Laplace2(15);
    Minimum_Variance_Laplacian(20);
    Laplacian_Gaussian(6000, 11, 1.4);
    Difference_Gaussian(20000, 11, 1, 3);
    return 0;
}

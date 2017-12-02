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
RGBTRIPLE **BMPoutput2_data = NULL;
RGBTRIPLE **BMPoutput3_data = NULL;

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

double SNR(RGBTRIPLE **compareimg)
{
    int i, j, n = bmpInfo.biHeight * bmpInfo.biWidth;
    double VS = 0, u  = 0, VN = 0, uN = 0;
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            u  += BMPdata[i][j].color;
            uN += compareimg[i][j].color - BMPdata[i][j].color;
        }
    u  /= n;
    uN /= n;
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            VS += ((double)BMPdata[i][j].color - u) * ((double)BMPdata[i][j].color - u);
            VN += ((double)compareimg[i][j].color - (double)BMPdata[i][j].color - uN) * ((double)compareimg[i][j].color - (double)BMPdata[i][j].color - uN);
        }
    VS /= n;
    VN /= n;
    return 20 * log10(sqrt(VS / VN));
}

string Salt_Pepper_Noise(double probability)
{
    int i, j;
    double n;
    char buf[10];
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            n = (double)rand() / RAND_MAX;
            if(fabs(n) < probability)
                BMPoutput_data[i][j].color = 0;
            else if(1 < fabs(n) + probability)
                BMPoutput_data[i][j].color = 255;
            else
                BMPoutput_data[i][j].color = BMPdata[i][j].color;
        }
    sprintf(buf,"%g",probability);
    saveBMP("salt_pepper_noise_"+ string(buf) +".bmp", BMPoutput_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput_data));
    return "salt_pepper_noise_"+ string(buf);
}

string Gaussian_Noise(double amplitude)
{
    int i, j;
    double n;
    char buf[10];
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            n = sqrt(-2 * log((double)rand() / RAND_MAX )) * cos(2 * M_PI * ((double)rand() / RAND_MAX ));
            BMPoutput_data[i][j].color = BMPdata[i][j].color + amplitude * n;
        }
    sprintf(buf,"%g",amplitude);
    saveBMP("gaussian_noise_" + string(buf) + ".bmp", BMPoutput_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput_data));
    return "gaussian_noise_"+ string(buf);
}

void Box(string output_name, int filter)
{
    int i, j, k, l, total, border = filter >> 1;
    char buf[10];
    unsigned char tmp[bmpInfo.biHeight+filter-1][bmpInfo.biWidth+filter-1]= {0};
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            tmp[i+border][j+border] = BMPoutput_data[i][j].color;

    for(i=border; i<bmpInfo.biHeight+border; ++i)
        for(j=border; j<bmpInfo.biWidth+border; ++j)
        {
            total = 0;
            for(k=0-border; k<=border; ++k)
                for(l=0-border; l<=border; ++l)
                    total += tmp[i+k][j+l];
            BMPoutput2_data[i-border][j-border].color = total / (filter * filter);
        }
    sprintf(buf,"%d",filter);
    saveBMP(output_name + "_box" + string(buf) + "x" + string(buf) + ".bmp", BMPoutput2_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput2_data));
}

void Median(string output_name, int filter)
{
    int i, j, k, l, total, border = filter >> 1;
    char buf[10];
    unsigned char tmp[bmpInfo.biHeight+filter-1][bmpInfo.biWidth+filter-1]= {0}, sorted_data[filter*filter];
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            tmp[i+border][j+border] = BMPoutput_data[i][j].color;

    for(i=border; i<bmpInfo.biHeight+border; ++i)
        for(j=border; j<bmpInfo.biWidth+border; ++j)
        {
            total = 0;
            for(k=0-border; k<=border; ++k)
                for(l=0-border; l<=border; ++l)
                    sorted_data[total++] = tmp[i+k][j+l];
            total = filter * filter;
            nth_element(sorted_data, sorted_data + (total >> 1), sorted_data + total);
            BMPoutput2_data[i-border][j-border].color = sorted_data[total>>1];
        }
    sprintf(buf,"%d",filter);
    saveBMP(output_name + "_median" + string(buf) + "x" + string(buf) + ".bmp", BMPoutput2_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput2_data));
}

void dilation(RGBTRIPLE **source, RGBTRIPLE **destination)
{
    int i, j, x, y;
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            destination[i][j].color = 0;
        }

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
}

void erosion(RGBTRIPLE **source, RGBTRIPLE **destination)
{
    int i, j, x, y, _min = 255;
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            destination[i][j].color = 0;
        }

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
}

void Opening_Closing(string output_name)
{
    erosion(BMPoutput_data, BMPoutput2_data);
    dilation(BMPoutput2_data, BMPoutput3_data);
    dilation(BMPoutput3_data, BMPoutput2_data);
    erosion(BMPoutput2_data, BMPoutput3_data);
    saveBMP(output_name + "_opening_closing.bmp", BMPoutput3_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput3_data));
}

void Closing_Opening(string output_name)
{
    dilation(BMPoutput_data, BMPoutput2_data);
    erosion(BMPoutput2_data, BMPoutput3_data);
    erosion(BMPoutput3_data, BMPoutput2_data);
    dilation(BMPoutput2_data, BMPoutput3_data);
    saveBMP(output_name + "_closing_opening.bmp", BMPoutput3_data);
    printf("SNR = %.4lf\n", SNR(BMPoutput3_data));
}

void operation(string (*generating_function)(double), double num)
{
    string output_name = (*generating_function)(num);
    Box(output_name, 3);
    Box(output_name, 5);
    Median(output_name, 3);
    Median(output_name, 5);
    Opening_Closing(output_name);
    Closing_Opening(output_name);
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
    BMPoutput2_data = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
    BMPoutput3_data = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    operation(Gaussian_Noise, 10);
    operation(Gaussian_Noise, 30);
    operation(Salt_Pepper_Noise, 0.05);
    operation(Salt_Pepper_Noise, 0.1);

    return 0;
}

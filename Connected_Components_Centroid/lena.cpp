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

struct pixelDATA
{
    int xc, yc, pc, top, buttom, left, right;
};

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
    int i, j, max = 0, histogram[256] = {0};
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            ++histogram[BMPdata[i][j].color];
            if(BMPdata[i][j].color < 128)
                BMPdata[i][j].color = 0;
            else
                BMPdata[i][j].color = 255;
        }
    saveBMP("binary_" + outfileName);

    for(i=0; i<256; ++i)
        max = max > histogram[i] ? max : histogram[i];
    for(i=0; i<256; ++i)
        histogram[i] = histogram[i] * 512 / max ;
    for(i=0; i<512; ++i)
        for(j=0; j<256; ++j)
        {
            if(histogram[j] > 0)
            {
                BMPdata[i][j*2].color = 0;
                BMPdata[i][j*2+1].color = 0;
                --histogram[j];
            }
            else
            {
                BMPdata[i][j*2].color = 255;
                BMPdata[i][j*2+1].color = 255;
            }
        }
    saveBMP("histogram_" + outfileName);
}

void Connected_Components(string infileName)
{
    int i, j, index = 1, cnt[bmpInfo.biHeight][bmpInfo.biWidth], count = 1;
    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            if(BMPdata[i][j].color)
            {
                cnt[i][j] = 1;
            }
            else
            {
                cnt[i][j] = 0;
                BMPdata[i][j].color = 200;
            }
        }

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            if(cnt[i][j])
                cnt[i][j] = count;
            ++count;
        }
    do
    {
        count = 0;
        for(i=0; i<bmpInfo.biHeight; ++i)
            for(j=0; j<bmpInfo.biWidth; ++j)
            {
                if(cnt[i][j])
                {
                    if(i != 0 && cnt[i-1][j] && cnt[i-1][j] < cnt[i][j])
                    {
                        cnt[i][j] = cnt[i-1][j];
                        count = 1;
                    }
                    if(j != 0 && cnt[i][j-1] && cnt[i][j-1] < cnt[i][j])
                    {
                        cnt[i][j] = cnt[i][j-1];
                        count = 1;
                    }
                }
            }

        for(i=bmpInfo.biHeight-1; i>-1; --i)
            for(j=bmpInfo.biWidth-1; j>-1; --j)
            {
                if(cnt[i][j])
                {
                    if(i != bmpInfo.biHeight-1 && cnt[i+1][j] && cnt[i+1][j] < cnt[i][j])
                    {
                        cnt[i][j] = cnt[i+1][j];
                        count = 1;
                    }
                    if(j != bmpInfo.biWidth-1  && cnt[i][j+1] && cnt[i][j+1] < cnt[i][j])
                    {
                        cnt[i][j] = cnt[i][j+1];
                        count = 1;
                    }
                }
            }
    }while(count);

    count = bmpInfo.biHeight * bmpInfo.biWidth;
    int label[count];
    for(i=0; i<count; ++i)
        label[i] = 0;

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
            ++label[cnt[i][j]];

    label[0] = 0;
    for(i=1; i<count; ++i)
    {
        if(label[i] > 500)
        {
            label[i] = index;
            ++index;
        }
        else
            label[i] = 0;
    }

    struct pixelDATA chosen[--index];
    for(i=0; i<index; ++i)
    {
        chosen[i].xc = 0;
        chosen[i].yc = 0;
        chosen[i].pc = 0;
        chosen[i].top = bmpInfo.biHeight - 1;
        chosen[i].buttom = 0;
        chosen[i].left = bmpInfo.biWidth - 1;
        chosen[i].right = 0;
    }

    for(i=0; i<bmpInfo.biHeight; ++i)
        for(j=0; j<bmpInfo.biWidth; ++j)
        {
            if(label[cnt[i][j]])
            {
                chosen[label[cnt[i][j]]-1].xc = chosen[label[cnt[i][j]]-1].xc + i;
                chosen[label[cnt[i][j]]-1].yc = chosen[label[cnt[i][j]]-1].yc + j;
                ++chosen[label[cnt[i][j]]-1].pc;

                if(i > chosen[label[cnt[i][j]]-1].buttom)
                    chosen[label[cnt[i][j]]-1].buttom = i;
                if(i < chosen[label[cnt[i][j]]-1].top)
                    chosen[label[cnt[i][j]]-1].top = i;
                if (j > chosen[label[cnt[i][j]]-1].right)
                    chosen[label[cnt[i][j]]-1].right = j;
                if (j < chosen[label[cnt[i][j]]-1].left)
                    chosen[label[cnt[i][j]]-1].left = j;
            }
        }

    for(i=0; i<index; ++i)
    {
        int k, l;
        int cntx = chosen[i].xc / chosen[i].pc;
        int cnty = chosen[i].yc / chosen[i].pc;

        for(j=-4; j<5; ++j)
        {
            BMPdata[cntx+j][cnty].color = 0;
			BMPdata[cntx+j][cnty-1].color = 0;
			BMPdata[cntx+j][cnty+1].color = 0;
            BMPdata[cntx][cnty+j].color = 0;
			BMPdata[cntx-1][cnty+j].color = 0;
			BMPdata[cntx+1][cnty+j].color = 0;
        }

        for(k=0; k<bmpInfo.biHeight; ++k)
        {
            for(l=0; l<bmpInfo.biWidth; ++l)
            {
                if(chosen[i].top <= k && k <= chosen[i].buttom)
                {
                    if(l == chosen[i].left)
					{
						BMPdata[k][l].color=0;
						BMPdata[k][l+1].color=0;
					}                        
                    if(l == chosen[i].right)
					{
						BMPdata[k][l].color=0;
						BMPdata[k][l-1].color=0;
					}                        
                }
                if(chosen[i].left <= l && l <= chosen[i].right)
                {
                    if(k == chosen[i].top)
					{
						BMPdata[k][l].color=0;
						BMPdata[k+1][l].color=0;
					}                        
                    if(k == chosen[i].buttom)
					{
						BMPdata[k][l].color=0;
						BMPdata[k-1][l].color=0;
					}                        
                }
            }
        }
    }
    saveBMP("connected_components_" + infileName);
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
    readBMP("binary_" + infileName, 2);
    Connected_Components(infileName);
    return 0;
}

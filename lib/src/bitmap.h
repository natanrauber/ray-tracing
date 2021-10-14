//============================================================================
// Name        : arquivoPPM.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

const int bytesPerPixel = 3; /// red, green, blue
const int fileHeaderSize = 14;
const int infoHeaderSize = 40;
unsigned char *img;
int line_size;
int cel_size;

char *createBitmapFileHeader(int height, int width, int paddingSize)
{
	int fileSize = fileHeaderSize + infoHeaderSize + (bytesPerPixel * width + paddingSize) * height;

	static unsigned char fileHeader[] = {
		0, 0,		/// signature
		0, 0, 0, 0, /// image file size in bytes
		0, 0, 0, 0, /// reserved
		0, 0, 0, 0, /// start of pixel array
	};

	fileHeader[0] = (unsigned char)('B');
	fileHeader[1] = (unsigned char)('M');
	fileHeader[2] = (unsigned char)(fileSize);
	fileHeader[3] = (unsigned char)(fileSize >> 8);
	fileHeader[4] = (unsigned char)(fileSize >> 16);
	fileHeader[5] = (unsigned char)(fileSize >> 24);
	fileHeader[10] = (unsigned char)(fileHeaderSize + infoHeaderSize);

	return (char *)fileHeader;
}

char *createBitmapInfoHeader(int height, int width)
{
	static unsigned char infoHeader[] = {
		0, 0, 0, 0, /// header size
		0, 0, 0, 0, /// image width
		0, 0, 0, 0, /// image height
		0, 0,		/// number of color planes
		0, 0,		/// bits per pixel
		0, 0, 0, 0, /// compression
		0, 0, 0, 0, /// image size
		0, 0, 0, 0, /// horizontal resolution
		0, 0, 0, 0, /// vertical resolution
		0, 0, 0, 0, /// colors in color table
		0, 0, 0, 0, /// important color count
	};

	infoHeader[0] = (unsigned char)(infoHeaderSize);
	infoHeader[4] = (unsigned char)(width);
	infoHeader[5] = (unsigned char)(width >> 8);
	infoHeader[6] = (unsigned char)(width >> 16);
	infoHeader[7] = (unsigned char)(width >> 24);
	infoHeader[8] = (unsigned char)(height);
	infoHeader[9] = (unsigned char)(height >> 8);
	infoHeader[10] = (unsigned char)(height >> 16);
	infoHeader[11] = (unsigned char)(height >> 24);
	infoHeader[12] = (unsigned char)(1);
	infoHeader[14] = (unsigned char)(bytesPerPixel * 8);

	return (char *)infoHeader;
}

void writeBitmapFile(ofstream &out, int width, int height)
{
	char padding[3] = {0, 0, 0};
	char color[3] = {0, 0, 0};
	int paddingSize = (4 - (width * bytesPerPixel) % 4) % 4;

	char *fileHeader = createBitmapFileHeader(height, width, paddingSize);
	char *infoHeader = createBitmapInfoHeader(height, width);

	out.write((char *)fileHeader, fileHeaderSize);
	out.write((char *)infoHeader, infoHeaderSize);

	for (int y = height; y > 0; y--)
	{
		for (int x = 0; x < width; x++)
		{
			int p = (height - y) * line_size + x * cel_size;
			color[2] = img[p];
			color[1] = img[p + 1];
			color[0] = img[p + 2];
			out.write(color, sizeof(color));
		}
		out.write(padding, paddingSize);
	}
}
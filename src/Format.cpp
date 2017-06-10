
#include <ctime>
#include <string>
#include <stdexcept>
#include <iostream>
#include <iterator>
#include <zlib.h>
#include <cstdio>
#include "Format.h"
#include "graphics/Graphics.h"

std::string format::URLEncode(std::string source)
{
	char * src = (char *)source.c_str();
	char * dst = new char[(source.length()*3)+2];
	std::fill(dst, dst+(source.length()*3)+2, 0);

	char *d;
	for (d = dst; *d; d++) ;

	for (unsigned char *s = (unsigned char *)src; *s; s++)
	{
		if ((*s>='0' && *s<='9') ||
		        (*s>='a' && *s<='z') ||
		        (*s>='A' && *s<='Z'))
			*(d++) = *s;
		else
		{
			*(d++) = '%';
			*(d++) = hex[*s>>4];
			*(d++) = hex[*s&15];
		}
	}
	*d = 0;

	std::string finalString(dst);
	delete[] dst;
	return finalString;
}

std::string format::UnixtimeToDate(time_t unixtime, std::string dateFormat)
{
	struct tm * timeData;
	char buffer[128];

	timeData = localtime(&unixtime);

	strftime(buffer, 128, dateFormat.c_str(), timeData);
	return std::string(buffer);
}

std::string format::UnixtimeToDateMini(time_t unixtime)
{
	time_t currentTime = time(NULL);
	struct tm currentTimeData = *localtime(&currentTime);
	struct tm timeData = *localtime(&unixtime);

	if(currentTimeData.tm_year != timeData.tm_year)
	{
		return UnixtimeToDate(unixtime, "%b %Y");
	}
	else if(currentTimeData.tm_mon != timeData.tm_mon || currentTimeData.tm_mday != timeData.tm_mday)
	{
		return UnixtimeToDate(unixtime, "%d %B");
	}
	else
	{
		return UnixtimeToDate(unixtime, "%H:%M:%S");
	}
}

std::string format::CleanString(std::string dirtyString, bool ascii, bool color, bool newlines, bool numeric)
{
	for (size_t i = 0; i < dirtyString.size(); i++)
	{
		switch(dirtyString[i])
		{
		case '\b':
			if (color)
			{
				dirtyString.erase(i, 2);
				i--;
			}
			else
				i++;
			break;
		case '\x0E':
			if (color)
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		case '\x0F':
			if (color)
			{
				dirtyString.erase(i, 4);
				i--;
			}
			else
				i += 3;
			break;
		case '\r':
		case '\n':
			if (newlines)
				dirtyString[i] = ' ';
			break;
		default:
			if (numeric && (dirtyString[i] < '0' || dirtyString[i] > '9'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			// if less than ascii 20 or greater than ascii 126, delete
			else if (ascii && (dirtyString[i] < ' ' || dirtyString[i] > '~'))
			{
				dirtyString.erase(i, 1);
				i--;
			}
			break;
		}
	}
	return dirtyString;
}

std::vector<char> format::VideoBufferToPTI(const VideoBuffer & vidBuf)
{
	std::vector<char> data;
	int dataSize = 0;
	char * buffer = (char*)Graphics::ptif_pack(vidBuf.Buffer, vidBuf.Width, vidBuf.Height, &dataSize);

	if(buffer)
	{
		data.insert(data.end(), buffer, buffer+dataSize);
		free(buffer);
	}

	return data;
}

VideoBuffer * format::PTIToVideoBuffer(std::vector<char> & data)
{

	int newWidth, newHeight;
	pixel * buffer = Graphics::ptif_unpack(&data[0], data.size(), &newWidth, &newHeight);

	if(buffer)
	{
		VideoBuffer * vb = new VideoBuffer(buffer, newWidth, newHeight);
		free(buffer);
		return vb;
	}
	return NULL;
}

std::vector<char> format::VideoBufferToBMP(const VideoBuffer & vidBuf)
{
	std::vector<char> data;
	char buffer[54] = "BM";
	int padding = 3 - (vidBuf.Width * 3 + 3) % 4;
	unsigned int fileSize = (vidBuf.Width * 3 + padding) * vidBuf.Height + 54;
	*(int *)(buffer + 2) = fileSize;
	*(short int *)(buffer + 6) = 0; // reserved;
	*(short int *)(buffer + 8) = 0; // reserved;
	*(int *)(buffer + 10) = 0x36; // 54 bytes from start to data
	*(int *)(buffer + 14) = 0x28; // 40 bytes in info header
	*(int *)(buffer + 18) = vidBuf.Width;
	*(int *)(buffer + 22) = vidBuf.Height;
	*(short int *)(buffer + 26) = 1; // 1 plane
	*(short int *)(buffer + 28) = 24; // 24 bits per pixel
	*(int *)(buffer + 30) = 0; // no compression
	*(int *)(buffer + 34) = fileSize - 54; // bitmap size
	*(int *)(buffer + 38) = 0xB13; // 72dpi
	*(int *)(buffer + 42) = 0xB13; // 72dpi
	*(int *)(buffer + 46) = 0; // all colors used
	*(int *)(buffer + 50) = 0; // all colors important

	data.insert(data.end(), buffer, buffer+54);

	char *currentRow = (char *)malloc((vidBuf.Width * 3) + padding);
	for(int y = vidBuf.Height - 1; y >= 0; y--)
	{
		int rowPos = 0;
		for(int x = 0; x < vidBuf.Width; x++)
		{
			currentRow[rowPos++] = PIXB(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXG(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXR(vidBuf.Buffer[(y*vidBuf.Width)+x]);
		}
		data.insert(data.end(), currentRow, currentRow+(vidBuf.Width*3)+padding);
	}
	free(currentRow);

	return data;
}

std::vector<char> format::VideoBufferToPPM(const VideoBuffer & vidBuf)
{
	std::vector<char> data;
	char buffer[256];
	sprintf(buffer, "P6\n%d %d\n255\n", vidBuf.Width, vidBuf.Height);
	data.insert(data.end(), buffer, buffer+strlen(buffer));

	unsigned char * currentRow = new unsigned char[vidBuf.Width*3];
	for(int y = 0; y < vidBuf.Height; y++)
	{
		int rowPos = 0;
		for(int x = 0; x < vidBuf.Width; x++)
		{
			currentRow[rowPos++] = PIXR(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXG(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXB(vidBuf.Buffer[(y*vidBuf.Width)+x]);
		}
		data.insert(data.end(), currentRow, currentRow+(vidBuf.Width*3));
	}
	delete [] currentRow;

	return data;
}

struct PNGChunk
{
	int Length;
	char Name[4];
	char * Data;

	//char[4] CRC();

	PNGChunk(int length, std::string name)
	{
		if (name.length()!=4)
			throw std::runtime_error("Invalid chunk name");
		std::copy(name.begin(), name.begin()+4, Name);
		Length = length;
		if (length)
		{
			Data = new char[length];
			std::fill(Data, Data+length, 0);
		}
		else
		{
			Data = NULL;
		}
	}
	unsigned long CRC()
	{
		if (!Data)
		{
			return format::CalculateCRC((unsigned char*)Name, 4);
		}
		else
		{
			unsigned char * temp = new unsigned char[4+Length];
			std::copy(Name, Name+4, temp);
			std::copy(Data, Data+Length, temp+4);
			unsigned long tempRet = format::CalculateCRC(temp, 4+Length);
			delete[] temp;
			return tempRet;
		}
	}
	~PNGChunk()
	{
		delete[] Data;
	}
};

std::vector<char> format::VideoBufferToPNG(const VideoBuffer & vidBuf)
{
	std::vector<PNGChunk*> chunks;

	//Begin IHDR (Image header) chunk (Image size and depth)
	PNGChunk *IHDRChunk = new PNGChunk(13, "IHDR");

	//Image Width
	IHDRChunk->Data[0] = (vidBuf.Width>>24)&0xFF;
	IHDRChunk->Data[1] = (vidBuf.Width>>16)&0xFF;
	IHDRChunk->Data[2] = (vidBuf.Width>>8)&0xFF;
	IHDRChunk->Data[3] = (vidBuf.Width)&0xFF;

	//Image Height
	IHDRChunk->Data[4] = (vidBuf.Height>>24)&0xFF;
	IHDRChunk->Data[5] = (vidBuf.Height>>16)&0xFF;
	IHDRChunk->Data[6] = (vidBuf.Height>>8)&0xFF;
	IHDRChunk->Data[7] = (vidBuf.Height)&0xFF;

	//Bit depth
	IHDRChunk->Data[8] = 8; //8bits per channel or 24bpp

	//Colour type
	IHDRChunk->Data[9] = 2; //RGB triple

	//Everything else is default
	chunks.push_back(IHDRChunk);

	//Begin image data, format is 8bit RGB (24bit pixel)
	int dataPos = 0;
	unsigned char * uncompressedData = new unsigned char[(vidBuf.Width*vidBuf.Height*3)+vidBuf.Height];

	//Byte ordering and filtering
	unsigned char * previousRow = new unsigned char[vidBuf.Width*3];
	std::fill(previousRow, previousRow+(vidBuf.Width*3), 0);
	unsigned char * currentRow = new unsigned char[vidBuf.Width*3];
	for(int y = 0; y < vidBuf.Height; y++)
	{
		int rowPos = 0;
		for(int x = 0; x < vidBuf.Width; x++)
		{
			currentRow[rowPos++] = PIXR(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXG(vidBuf.Buffer[(y*vidBuf.Width)+x]);
			currentRow[rowPos++] = PIXB(vidBuf.Buffer[(y*vidBuf.Width)+x]);
		}

		uncompressedData[dataPos++] = 2; //Up Sub(x) filter 
		for(int b = 0; b < rowPos; b++)
		{
			int filteredByte = (currentRow[b]-previousRow[b])&0xFF;
			uncompressedData[dataPos++] = filteredByte;
		}

		unsigned char * tempRow = previousRow;
		previousRow = currentRow;
		currentRow = tempRow;
	}
	delete[] currentRow;
	delete[] previousRow;

	//Compression
	int compressedBufferSize = (vidBuf.Width*vidBuf.Height*3)*2;
	unsigned char * compressedData = new unsigned char[compressedBufferSize];

	int result;
    z_stream zipStream; 
    zipStream.zalloc = Z_NULL;
    zipStream.zfree = Z_NULL;
    zipStream.opaque = Z_NULL;

    result = deflateInit2(&zipStream,
            9,              // level
            Z_DEFLATED,     // method
            10,             // windowBits
            1,              // memLevel
            Z_DEFAULT_STRATEGY // strategy
    );

    if (result != Z_OK) exit(result);

    zipStream.next_in = uncompressedData;
    zipStream.avail_in = dataPos;

    zipStream.next_out = compressedData;
    zipStream.avail_out = compressedBufferSize;


    result = deflate(&zipStream, Z_FINISH);
    if (result != Z_STREAM_END) exit(result);

    int compressedSize = compressedBufferSize-zipStream.avail_out;
	PNGChunk *IDATChunk = new PNGChunk(compressedSize, "IDAT");
	std::copy(compressedData, compressedData+compressedSize, IDATChunk->Data);
	chunks.push_back(IDATChunk);

	deflateEnd(&zipStream);

	delete[] compressedData;
	delete[] uncompressedData;

	PNGChunk *IENDChunk = new PNGChunk(0, "IEND");
	chunks.push_back(IENDChunk);

	//Write chunks to output buffer
	int finalDataSize = 8;
	for(std::vector<PNGChunk*>::iterator iter = chunks.begin(), end = chunks.end(); iter != end; ++iter)
	{
		PNGChunk * cChunk = *iter;
		finalDataSize += 4 + 4 + 4;
		finalDataSize += cChunk->Length;
	}
	unsigned char * finalData = new unsigned char[finalDataSize];
	int finalDataPos = 0;

	//PNG File header
	finalData[finalDataPos++] = 0x89;
	finalData[finalDataPos++] = 0x50;
	finalData[finalDataPos++] = 0x4E;
	finalData[finalDataPos++] = 0x47;
	finalData[finalDataPos++] = 0x0D;
	finalData[finalDataPos++] = 0x0A;
	finalData[finalDataPos++] = 0x1A;
	finalData[finalDataPos++] = 0x0A;

	for(std::vector<PNGChunk*>::iterator iter = chunks.begin(), end = chunks.end(); iter != end; ++iter)
	{
		PNGChunk * cChunk = *iter;

		//Chunk length
		finalData[finalDataPos++] = (cChunk->Length>>24)&0xFF;
		finalData[finalDataPos++] = (cChunk->Length>>16)&0xFF;
		finalData[finalDataPos++] = (cChunk->Length>>8)&0xFF;
		finalData[finalDataPos++] = (cChunk->Length)&0xFF;

		//Chunk name
		std::copy(cChunk->Name, cChunk->Name+4, finalData+finalDataPos);
		finalDataPos += 4;

		//Chunk data
		if(cChunk->Data)
		{
			std::copy(cChunk->Data, cChunk->Data+cChunk->Length, finalData+finalDataPos);
			finalDataPos += cChunk->Length;
		}

		//Chunk CRC
		unsigned long tempCRC = cChunk->CRC();
		finalData[finalDataPos++] = (tempCRC>>24)&0xFF;
		finalData[finalDataPos++] = (tempCRC>>16)&0xFF;
		finalData[finalDataPos++] = (tempCRC>>8)&0xFF;
		finalData[finalDataPos++] = (tempCRC)&0xFF;

		delete cChunk;
	}

	std::vector<char> outputData(finalData, finalData+finalDataPos);

	delete[] finalData;

	return outputData;
}

//CRC functions, copypasta from W3 PNG spec.

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
 unsigned long c;
 int n, k;

 for (n = 0; n < 256; n++) {
   c = (unsigned long) n;
   for (k = 0; k < 8; k++) {
     if (c & 1)
       c = 0xedb88320L ^ (c >> 1);
     else
       c = c >> 1;
   }
   crc_table[n] = c;
 }
 crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
  should be initialized to all 1's, and the transmitted value
  is the 1's complement of the final running CRC (see the
  crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf, int len)
{
	unsigned long c = crc;
	int n;

	if (!crc_table_computed)
		make_crc_table();
	for (n = 0; n < len; n++)
	{
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}

unsigned long format::CalculateCRC(unsigned char * data, int len)
{
	return update_crc(0xffffffffL, data, len) ^ 0xffffffffL;
}

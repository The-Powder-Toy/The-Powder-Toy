#include "bz2wrap.h"

#include "bzlib.h"

#include <memory>
#include <functional>
#include <vector>
#include <algorithm>

static size_t outputSizeIncrement = 0x100000U;

BZ2WCompressResult BZ2WCompress(std::vector<char> &dest, const char *srcData, size_t srcSize, size_t maxSize)
{
	bz_stream stream;
	stream.bzalloc = NULL;
	stream.bzfree = NULL;
	stream.opaque = NULL;
	if (BZ2_bzCompressInit(&stream, 9, 0, 0) != BZ_OK)
	{
		return BZ2WCompressNomem;
	}
	std::unique_ptr<bz_stream, std::function<int (bz_stream *)>> bz2Data(&stream, BZ2_bzCompressEnd);
	stream.next_in = const_cast<char *>(srcData); // I hope bz2 doesn't actually write anything here...
	stream.avail_in = srcSize;
	dest.resize(0);
	bool done = false;
	while (!done)
	{
		size_t oldSize = dest.size();
		size_t newSize = oldSize + outputSizeIncrement;
		if (maxSize && newSize > maxSize)
		{
			newSize = maxSize;
		}
		if (oldSize == newSize)
		{
			return BZ2WCompressLimit;
		}
		dest.resize(newSize);
		stream.next_out = &dest[stream.total_out_lo32];
		stream.avail_out = dest.size() - stream.total_out_lo32;
		if (BZ2_bzCompress(&stream, BZ_FINISH) == BZ_STREAM_END)
		{
			done = true;
		}
	}
	dest.resize(stream.total_out_lo32);
	return BZ2WCompressOk;
}

BZ2WDecompressResult BZ2WDecompress(std::vector<char> &dest, const char *srcData, size_t srcSize, size_t maxSize)
{
	bz_stream stream;
	stream.bzalloc = NULL;
	stream.bzfree = NULL;
	stream.opaque = NULL;
	if (BZ2_bzDecompressInit(&stream, 0, 0) != BZ_OK)
	{
		return BZ2WDecompressNomem;
	}
	std::unique_ptr<bz_stream, std::function<int (bz_stream *)>> bz2Data(&stream, BZ2_bzDecompressEnd); 
	stream.next_in = const_cast<char *>(srcData); // I hope bz2 doesn't actually write anything here...
	stream.avail_in = srcSize;
	dest.resize(0);
	bool done = false;
	while (!done)
	{
		size_t oldSize = dest.size();
		size_t newSize = oldSize + outputSizeIncrement;
		if (maxSize && newSize > maxSize)
		{
			newSize = maxSize;
		}
		if (oldSize == newSize)
		{
			return BZ2WDecompressLimit;
		}
		dest.resize(newSize);
		stream.next_out = &dest[stream.total_out_lo32];
		stream.avail_out = dest.size() - stream.total_out_lo32;
		switch (BZ2_bzDecompress(&stream))
		{
		case BZ_OK:
			if (!stream.avail_in && stream.avail_out)
			{
				return BZ2WDecompressEof;
			}
			break;

		case BZ_MEM_ERROR:
			return BZ2WDecompressNomem;

		case BZ_DATA_ERROR:
			return BZ2WDecompressBad;

		case BZ_DATA_ERROR_MAGIC:
			return BZ2WDecompressType;

		case BZ_STREAM_END:
			done = true;
			break;
		}
	}
	dest.resize(stream.total_out_lo32);
	return BZ2WDecompressOk;
}

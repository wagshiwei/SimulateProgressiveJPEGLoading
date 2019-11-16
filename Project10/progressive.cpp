

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MARKER_FIRST_BYTE  0xFF
#define MARKER_ESCAPE_BYTE  0x00
#define MARKER_SOI  0xD8
#define MARKER_TEM  0x01
#define MARKER_EOI  0xD9
#define MARKER_SOS  0xDA
#define MARKER_APP1  0xE1
#define MARKER_SOFn  0xC0
#define MARKER_RST0  0xD0
#define MARKER_RST7  0xD7
//#define MARKER_SCAN_BEGIN 0xDA
#define APP1_EXIF_MAGIC  0x45786966

enum {
	READ_FIRST_JPEG_BYTE = 0,

	/**
	 * Parser saw only one byte so far (0xFF). Next byte should be second byte of SOI marker
	 */
	READ_SECOND_JPEG_BYTE = 1,

	/**
	 * Next byte is either entropy coded data or first byte of a marker. First byte of marker
	 * cannot appear in entropy coded data, unless it is followed by 0x00 escape byte.
	 */
	READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA = 2,

	/**
	 * Last read byte is 0xFF, possible start of marker (possible, because next byte might be
	 * "escape byte" or 0xFF again)
	 */
	READ_MARKER_SECOND_BYTE = 3,

	/**
	 * Last two bytes constitute a marker that indicates start of a segment, the following two bytes
	 * denote 16bit size of the segment
	 */
	READ_SIZE_FIRST_BYTE = 4,

	/**
	 * Last three bytes are marker and first byte of segment size, after reading next byte, bytes
	 * constituting remaining part of segment will be skipped
	 */
	READ_SIZE_SECOND_BYTE = 5,

	/**
	 * Parsed data is not a JPEG file
	 */
	NOT_A_JPEG = 6,

	DIRECTLY_END = 7,


};

bool doesMarkerStartSegment(int markerSecondByte) {
	if (markerSecondByte == MARKER_TEM) {
		return false;
	}

	if (markerSecondByte >= MARKER_RST0 && markerSecondByte <= MARKER_RST7) {
		return false;
	}

	return markerSecondByte != MARKER_EOI && markerSecondByte != MARKER_SOI;
}

typedef struct ByteData {
	unsigned char *buffer;
	int current;
}ByteData;

int testProgressive(int index, unsigned char* data, int size);

void newScanOrImageEndFound(int index, ByteData *data, bool isover = false) {

	unsigned char *mBaos = data->buffer;
	if (!isover) {

		unsigned char C1 = mBaos[data->current - 2];
		unsigned char C2 = mBaos[data->current - 1];
		mBaos[data->current - 2] = MARKER_FIRST_BYTE;
		mBaos[data->current - 1] = MARKER_EOI;
		printf("DA ======\n");
		testProgressive(index, mBaos, data->current - 1);

		mBaos[data->current - 2] = C1;
		mBaos[data->current - 1] = C2;

	}
	else {
		testProgressive(index, mBaos, data->current - 1);
	}

}

#if 0
int file_size(char* filename)
{
	FILE *fp = fopen(filename, "r");
	if (!fp) return -1;
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fclose(fp);

	return size;
}
#endif

int file_size(FILE *fp)
{
	//FILE *fp = fopen(filename, "r");

	if (!fp) return -1;
	int csize = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	fseek(fp, csize, SEEK_SET);
	//fclose(fp);

	return size;
}


void writeToBaos(ByteData *data, unsigned char byte) {
	data->buffer[data->current] = byte;
	data->current++;
}

void writeToBaos(ByteData *data, FILE*f, int length) {


	unsigned char c;
	int count = 0;
	//while ((c = fgetc(f)) != EOF&&count<length) {
	while (count < length&&fread(&c, 1, 1, f) > 0) {
		
		data->buffer[data->current] = (unsigned char)c;
		data->current++;
		count++;
	}
}

void writeToBaos(ByteData *data, FILE*f) {
	
	unsigned char c;

	//while ((c = fgetc(f))!= EOF) {
	while (fread(&c, 1, 1, f) > 0) {
		
		data->buffer[data->current] = (unsigned char)c;
		data->current++;
	}
}

bool doParseMoreData(FILE* inputStream, int index) {
	int filesize = file_size(inputStream);
	ByteData *byteData = new ByteData();
	byteData->buffer = (unsigned char*)malloc(filesize + 2);
	memset(byteData->buffer, 0, filesize + 2);
	byteData->current = 0;

	int csize = ftell(inputStream);
	
	int mLastByteRead;
	int mParserState;
	mLastByteRead = 0;
	mParserState = READ_FIRST_JPEG_BYTE;

	unsigned char nextByte;

	int size;
	int bytesToSkip;
	bool isFirst = true;
	bool scanBegin = false;
	int scanSize = 0;

	//while ((nextByte = fgetc(inputStream)) != EOF) {
	while (fread(&nextByte, 1, 1, inputStream) > 0) {
		writeToBaos(byteData, (unsigned char)nextByte);
		if (scanBegin) {
			scanSize++;
		}
		if (scanSize > 0 && scanSize % 1000 == 0) {
			newScanOrImageEndFound(index, byteData);
		}

		switch (mParserState) {
		case READ_FIRST_JPEG_BYTE:
			if (nextByte == MARKER_FIRST_BYTE) {
				mParserState = READ_SECOND_JPEG_BYTE;
			}
			else {
				mParserState = NOT_A_JPEG;
			}
			break;

		case READ_SECOND_JPEG_BYTE:
			if (nextByte == MARKER_SOI) {
				mParserState = READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA;
			}
			else {
				mParserState = NOT_A_JPEG;
			}
			break;

		case READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA:
			if (nextByte == MARKER_FIRST_BYTE) {
				mParserState = READ_MARKER_SECOND_BYTE;
			}
			break;

		case READ_MARKER_SECOND_BYTE:
			if (nextByte == MARKER_FIRST_BYTE) {
				mParserState = READ_MARKER_SECOND_BYTE;
			}
			else if (nextByte == MARKER_ESCAPE_BYTE) {
				mParserState = READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA;
			}
			else {
				/*if (nextByte == MARKER_SOS || nextByte == MARKER_EOI) {
					if (!isFirst) {
						newScanOrImageEndFound();
						isFirst = false;
					}
				}*/
				if (nextByte == MARKER_SOS) {
					scanBegin = true;
					scanSize = 0;
					if (!isFirst) {
						newScanOrImageEndFound(index, byteData);

					}
					isFirst = false;
				}

				if (doesMarkerStartSegment(nextByte)) {
					mParserState = READ_SIZE_FIRST_BYTE;
				}
				else {
					mParserState = READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA;
				}
			}
			break;

		case READ_SIZE_FIRST_BYTE:
			mParserState = READ_SIZE_SECOND_BYTE;
			break;

		case READ_SIZE_SECOND_BYTE:
			size = (mLastByteRead << 8) + nextByte;
			// We need to jump after the end of the segment - skip size-2 next bytes.
			// We might want to skip more data than is available to read, in which case we will
			// consume entire data in inputStream and exit this function before entering another
			// iteration of the loop.
			bytesToSkip = size - 2;
			// StreamUtil.skip(inputStream, bytesToSkip);

			// Todo by lsy: Save the skip data in Buffer
			writeToBaos(byteData, inputStream, bytesToSkip);
			mParserState = READ_MARKER_FIRST_BYTE_OR_ENTROPY_DATA;
			break;

		case NOT_A_JPEG:
			writeToBaos(byteData, inputStream);
			break;
		case DIRECTLY_END:
			writeToBaos(byteData, inputStream);
			break;
		default:
			break;
		}

		mLastByteRead = nextByte;
	}

	newScanOrImageEndFound(index, byteData, true);
	free(byteData->buffer);
	delete(byteData);
	return true;
}

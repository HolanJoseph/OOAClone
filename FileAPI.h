#include "Types.h"

struct GetFileSizeReturnType
{
	U64 fileSize;
	bool fileExists;
};
GetFileSizeReturnType GetFileSize(char* filename);

struct ReadFileReturnType
{
	U64 numberOfBytesRead;
	bool errorEncountered;
};
ReadFileReturnType ReadFile(char* filename, char* fileBuffer, U64 numberOfBytesToRead, U64 readPosition = 0);
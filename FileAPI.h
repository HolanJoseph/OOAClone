#pragma once
#include "Types.h"

bool CreateFile(char* filename, bool overwriteIfExists);

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

struct WriteFileReturnType
{
	U64 numberOfBytesWritten;
	bool errorEncountered;
};
WriteFileReturnType WriteFile(char* filename, char* fileBuffer, U64 numberOfBytesToWrite, U64 writePosition = 0);
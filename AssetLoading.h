#pragma once

#include "stb_image.h"

#include "Types.h"
#include "FileAPI.h"
#include "StringAPI.h"

U8* LoadTexture(char* filename, I32* textureWidth, I32* textureHeight, I32* textureImageComponents, I32 textureNumberOfImageComponentsDesired)
{
	stbi_set_flip_vertically_on_load(1);
	return stbi_load(filename, textureWidth, textureHeight, textureImageComponents, textureNumberOfImageComponentsDesired);
}

//NOTE: Loads as vec3
//NOTE: indices are stored as U32s if larger is needed change this
//pointer to vertices
//numVerts
//pointer to indices
//numIndices
bool Load3DModel(char* filename, F32** out_vertices, U64* out_numberOfVertices, U32** out_indices, U64* out_numberOfIndices)
{
	GetFileSizeReturnType fs = GetFileSize(filename);
	char* fileContents = (char*)malloc(sizeof(char)*fs.fileSize);
	ReadFileReturnType fr = ReadFile(filename, fileContents, fs.fileSize, 0);

	if (!fs.fileExists || fr.errorEncountered)
	{
		return false;
	}

	U64 numberOfLines = 0;
	U64* fileLineLengths = NULL;
	char** fileLines = SplitStringOnCharacter(fileContents, fr.numberOfBytesRead, '\n', &numberOfLines, &fileLineLengths);

	F32 * vertices = (F32*)malloc(sizeof(F32)* numberOfLines * 3); // 3 components per vertex
	U64 numberOfVertices = 0;

	U32* indices = (U32*)malloc(sizeof(U32)* numberOfLines * 3); // 3 indices per line
	U64 numberOfIndices = 0;

	for (U64 i = 0; i < numberOfLines; ++i)
	{
		if (fileLineLengths[i] > 0)
		{
			U64 numberOfComponents = 0;
			U64* lineComponentLengths = NULL;
			char** lineComponents = SplitStringOnCharacter(fileLines[i], fileLineLengths[i], ' ', &numberOfComponents, &lineComponentLengths);
			if (lineComponents[0][0] == 'v' && lineComponentLengths[0] == 2)
			{
				// Process vertex
				vertices[numberOfVertices++] = StringToF32(lineComponents[1]); // x
				vertices[numberOfVertices++] = StringToF32(lineComponents[2]); // y
				vertices[numberOfVertices++] = StringToF32(lineComponents[3]); // z
			}
			else if (lineComponents[0][0] == 'f')
			{
				// Process face indices
				U64 faceVertex1NumberOfIndices = 0;
				U64* faceVertex1IndiceLengths = NULL;
				char** faceVertex1Indices = SplitStringOnCharacter(lineComponents[1], lineComponentLengths[1], '/', &faceVertex1NumberOfIndices, &faceVertex1IndiceLengths);
				indices[numberOfIndices++] = StringToU32(faceVertex1Indices[0]) - 1;

				U64 faceVertex2NumberOfIndices = 0;
				U64* faceVertex2IndiceLengths = NULL;
				char** faceVertex2Indices = SplitStringOnCharacter(lineComponents[2], lineComponentLengths[2], '/', &faceVertex2NumberOfIndices, &faceVertex2IndiceLengths);
				indices[numberOfIndices++] = StringToU32(faceVertex2Indices[0]) - 1;

				U64 faceVertex3NumberOfIndices = 0;
				U64* faceVertex3IndiceLengths = NULL;
				char** faceVertex3Indices = SplitStringOnCharacter(lineComponents[3], lineComponentLengths[3], '/', &faceVertex3NumberOfIndices, &faceVertex3IndiceLengths);
				indices[numberOfIndices++] = StringToU32(faceVertex3Indices[0]) - 1;

				free(faceVertex1Indices);
				free(faceVertex1IndiceLengths);

				free(faceVertex2Indices);
				free(faceVertex2IndiceLengths);

				free(faceVertex3Indices);
				free(faceVertex3IndiceLengths);
			}

			free(lineComponents);
			free(lineComponentLengths);

		}
	}

	vertices = (F32*)realloc(vertices, sizeof(F32)* numberOfVertices);
	indices = (U32*)realloc(indices, sizeof(U32)* numberOfIndices);

	*out_vertices = vertices;
	*out_numberOfVertices = numberOfVertices;
	*out_indices = indices;
	*out_numberOfIndices = numberOfIndices;

	free(fileLines);
	free(fileLineLengths);

	free(fileContents);
	return true;
}
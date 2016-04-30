#pragma once

#include "3rdParty/stb_image.h"

#include "Types.h"
#include "_SystemAPI.h"
#include "String.h"

// NOTE: User is responsible for freeing vertices and indices
struct TextureData
{
	U8* data;
	I32 width;
	I32 height;
	I32 numberOfComponents;
};
TextureData LoadTexture(char* filename, I32 textureNumberOfImageComponentsDesired = 4)
{
	TextureData result;

	stbi_set_flip_vertically_on_load(1);
	result.data = stbi_load(filename, &result.width, &result.height, &result.numberOfComponents, textureNumberOfImageComponentsDesired);
	return result;
}

// NOTE: despite being F32s each vertex is is a triplet so for any process looking to use the vertex data typecast
//			to vec3 and divide numberOfVertices by 3
// NOTE: indices are stored as U32s if larger is needed change this
// NOTE: User is responsible for freeing vertices and indices
struct ModelData
{
	F32* vertices;
	size_t numberOfVertices;

	U32* indices;
	size_t numberOfIndices;
};
ModelData Load3DModel(char* filename)
{
	ModelData result;

	GetFileSizeReturnType fs = GetFileSize(filename);
	char* fileContents = (char*)malloc(sizeof(char)*fs.fileSize);
	ReadFileReturnType fr = ReadFile(filename, fileContents, fs.fileSize, 0);

	if (!fs.fileExists || fr.errorEncountered)
	{
		free(fileContents);
		result.vertices = NULL;
		result.numberOfVertices = 0;
		result.indices = NULL;
		result.numberOfIndices = 0;
		return result;
	}

	SplitResult fileLines = Split(fileContents, fr.numberOfBytesRead, '\n');

	F32 * vertices = (F32*)malloc(sizeof(F32)* fileLines.numberOfComponents * 3); // 3 components per vertex
	size_t numberOfVertices = 0;

	U32* indices = (U32*)malloc(sizeof(U32)* fileLines.numberOfComponents * 3); // 3 indices per line
	size_t numberOfIndices = 0;

	for (U64 i = 0; i < fileLines.numberOfComponents; ++i)
	{
		if (fileLines.componentLengths[i] > 0)
		{
			SplitResult lineComponents = Split(fileLines.components[i], fileLines.componentLengths[i], ' ');
			if (lineComponents.components[0][0] == 'v' && lineComponents.componentLengths[0] == 2)
			{
				// Process vertex
				vertices[numberOfVertices++] = ToF32(lineComponents.components[1]); // x
				vertices[numberOfVertices++] = ToF32(lineComponents.components[2]); // y
				vertices[numberOfVertices++] = ToF32(lineComponents.components[3]); // z
			}
			else if (lineComponents.components[0][0] == 'f')
			{
				// Process face indices
				SplitResult faceVertex1Indices = Split(lineComponents.components[1], lineComponents.componentLengths[1], '/');
				indices[numberOfIndices++] = ToU32(faceVertex1Indices.components[0]) - 1;

				SplitResult faceVertex2Indices = Split(lineComponents.components[2], lineComponents.componentLengths[2], '/');
				indices[numberOfIndices++] = ToU32(faceVertex2Indices.components[0]) - 1;

				SplitResult faceVertex3Indices = Split(lineComponents.components[3], lineComponents.componentLengths[3], '/');
				indices[numberOfIndices++] = ToU32(faceVertex3Indices.components[0]) - 1;

				free(faceVertex1Indices.components);
				free(faceVertex1Indices.componentLengths);

				free(faceVertex2Indices.components);
				free(faceVertex2Indices.componentLengths);

				free(faceVertex3Indices.components);
				free(faceVertex3Indices.componentLengths);
			}

			free(lineComponents.components);
			free(lineComponents.componentLengths);

		}
	}

	vertices = (F32*)realloc(vertices, sizeof(F32) * numberOfVertices);
	indices = (U32*)realloc(indices, sizeof(U32) * numberOfIndices);

	result.vertices = vertices;
	result.numberOfVertices = numberOfVertices;
	result.indices = indices;
	result.numberOfIndices = numberOfIndices;

	free(fileLines.components);
	free(fileLines.componentLengths);

	free(fileContents);
	return result;
}
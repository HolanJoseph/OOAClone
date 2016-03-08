#include "stb_image.h"

#include "glew/GL/glew.h"
#include <gl/GL.h>

#include "Types.h"
#include "Math.h"
#include "BitManip.h"
#include "Util.h"
#include "RandomNumberGenerator.h"

#include "GameAPI.h"
#include "DebugAPI.h"
#include "InputAPI.h"
#include "FileAPI.h"
#include "StringAPI.h"

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

	F32 * vertices = (F32*)malloc(sizeof(F32) * numberOfLines * 3); // 3 components per vertex
	U64 numberOfVertices = 0;

	U32* indices = (U32*)malloc(sizeof(U32) * numberOfLines * 3); // 3 indices per line
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

struct Rectangle
{
	mat3 transform;
 
 	vec2 origin; // NOTE: this is in "model" space
 	vec2 halfDim;
};

vec2 Support(Rectangle* A, vec2 direction)
{
	vec3 AiPoints[4] = {
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, 1.0f),
		A->transform * vec3(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, 1.0f)
	};
	F32 AiDots[4] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y)),
		dot(direction, vec2(AiPoints[3].x, AiPoints[3].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 4; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);
	
	return maxA;
}



struct Circle
{
	vec2 origin;
	F32  radius;
};

vec2 Support(Circle* A, vec2 direction)
{
	direction = normalize(direction);
	vec2 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Triangle
{
	mat3 transform;
	
	vec2 origin; // NOTE: this is in "model" space
	vec2 points[3];
};

vec2 Support(Triangle* A, vec2 direction)
{
	vec3 AiPoints[3] = {
		A->transform * vec3(A->origin.x + A->points[0].x, A->origin.y + A->points[0].y, 1.0f),
		A->transform * vec3(A->origin.x + A->points[1].x, A->origin.y + A->points[1].y, 1.0f),
		A->transform * vec3(A->origin.x + A->points[2].x, A->origin.y + A->points[2].y, 1.0f)
	};
	F32 AiDots[3] = {
		dot(direction, vec2(AiPoints[0].x, AiPoints[0].y)),
		dot(direction, vec2(AiPoints[1].x, AiPoints[1].y)),
		dot(direction, vec2(AiPoints[2].x, AiPoints[2].y))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec2 maxA = vec2(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y);

	return maxA;
}



struct OrientedBoundingBox
{
	mat4 transform;
	
	vec3 origin; // NOTE: this is in "model" space
	vec3 halfDim;
};

vec3 Support(OrientedBoundingBox* A, vec3 direction)
{
	vec4 AiPoints[8] = {
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z + A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x - A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y - A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
		A->transform * vec4(A->origin.x + A->halfDim.x, A->origin.y + A->halfDim.y, A->origin.z - A->halfDim.z, 1.0f),
	};
	F32 AiDots[8] = {
		dot(direction, vec3(AiPoints[0].x, AiPoints[0].y, AiPoints[0].z)),
		dot(direction, vec3(AiPoints[1].x, AiPoints[1].y, AiPoints[1].z)),
		dot(direction, vec3(AiPoints[2].x, AiPoints[2].y, AiPoints[2].z)),
		dot(direction, vec3(AiPoints[3].x, AiPoints[3].y, AiPoints[3].z)),
		dot(direction, vec3(AiPoints[4].x, AiPoints[4].y, AiPoints[4].z)),
		dot(direction, vec3(AiPoints[5].x, AiPoints[5].y, AiPoints[5].z)),
		dot(direction, vec3(AiPoints[6].x, AiPoints[6].y, AiPoints[6].z)),
		dot(direction, vec3(AiPoints[7].x, AiPoints[7].y, AiPoints[7].z))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 8; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec3 maxA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);

	return maxA;
}



struct Sphere
{
	vec3 origin;
	F32 radius;
};

vec3 Support(Sphere* A, vec3 direction)
{
	direction = normalize(direction);
	vec3 maxA = A->origin + (A->radius * direction);

	return maxA;
}



struct Capsule
{
	mat4 transform;

	vec3 points[2]; // NOTE: these are in "model" space
	F32 radius;
};

vec3 Support(Capsule* A, vec3 direction)
{
	vec4 AiPoints[2] = {
		A->transform * vec4(A->points[0].x, A->points[0].y, A->points[0].z, 1.0f),
		A->transform * vec4(A->points[1].x, A->points[1].y, A->points[1].z, 1.0f)
	};
	F32 AiDots[2] = {
		dot(direction, vec3(AiPoints[0])),
		dot(direction, vec3(AiPoints[1]))
	};

	U32 maxPositionAi = 0;
	F32 maxDotAi = -1000000.0f;
	for (U32 i = 0; i < 3; ++i)
	{
		if (AiDots[i] > maxDotAi)
		{
			maxPositionAi = i;
			maxDotAi = AiDots[i];
		}
	}

	vec3 maxLinePointA = vec3(AiPoints[maxPositionAi].x, AiPoints[maxPositionAi].y, AiPoints[maxPositionAi].z);
	vec3 maxA = maxLinePointA + (normalize(direction) * A->radius);

	return maxA;
}



/*
	2D COMBO SUPPORTS
*/
vec2 Support(Rectangle* A, Rectangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Rectangle* A, Circle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Rectangle* A, Triangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Circle* A, Circle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Circle* A, Rectangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Circle* A, Triangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Triangle* A, Triangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Triangle* A, Rectangle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}

vec2 Support(Triangle* A, Circle* B, vec2* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec2 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec2 maxB = Support(B, -*direction);
	vec2 result = maxA - maxB;
	return result;
}



/*
	3D COMBO SUPPORTS
*/
vec3 Support(OrientedBoundingBox* A, OrientedBoundingBox* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(OrientedBoundingBox* A, Sphere* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(OrientedBoundingBox* A, Capsule* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Sphere* A, Sphere* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Sphere* A, OrientedBoundingBox* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Sphere* A, Capsule* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Capsule* A, Capsule* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Capsule* A, OrientedBoundingBox* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}

vec3 Support(Capsule* A, Sphere* B, vec3* direction)
{
	// find the point in A that has the largest value with dot(Ai, Direction)
	vec3 maxA = Support(A, *direction);

	// find the point in B that has the largest value with dot(Bj, Direction)
	vec3 maxB = Support(B, -*direction);
	vec3 result = maxA - maxB;
	return result;
}






enum SimplexType
{
	Simplex_Point = 0,
	Simplex_Line = 1,
	Simplex_Triangle = 2,
	Simplex_Tetrahedron = 3
};

struct Simplex
{
	SimplexType type;
	vec3 A;
	vec3 B;
	vec3 C;
	vec3 D;
};

struct DoSimplexResult
{
	bool containsGoal;
	Simplex simplex;
	vec3 d;
};

bool DoSimplexLine(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3(simplex[0].x - simplex[1].x, simplex[0].y - simplex[1].y, 0);
	vec3 ag = vec3(0 - simplex[1].x, 0 - simplex[1].y, 0);
	
	vec3 DinR3 = cross(cross(ab, ag), ab);
	*D = vec2(DinR3.x, DinR3.y);

	return result;
}

bool DoSimplexLineCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3(simplex[0].x - simplex[1].x, simplex[0].y - simplex[1].y, 0);
	vec3 ag = vec3(0-simplex[1].x, 0-simplex[1].y, 0);
	if (dot(ab, ag) > 0)
	{
		vec3 DinR3 = cross(cross(ab, ag), ab);
		*D = vec2(DinR3.x, DinR3.y);
	}
	else
	{
		Assert(false);
		
		simplex[0] = simplex[1];
		simplex[1] = vec2();
		*simplexType = Simplex_Point;
	}

	return result;
}

DoSimplexResult DoSimplexLineAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 ab = simplex.B - simplex.A;
	vec3 ag = vec3(0,0,0) - simplex.A;
	vec3 ba = simplex.A - simplex.B;
	vec3 bg = vec3(0,0,0) - simplex.B;

	if ( dot(ab, ag) < 0 )
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if ( dot(ba, bg) < 0 )
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}

DoSimplexResult DoSimplexLineAllVoronoiDouble(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	dvec3 ab = dvec3(simplex.B) - dvec3(simplex.A);
	dvec3 ag = dvec3(0, 0, 0) - dvec3(simplex.A);
	dvec3 ba = dvec3(simplex.A) - dvec3(simplex.B);
	dvec3 bg = dvec3(0, 0, 0) - dvec3(simplex.B);

	if (dot(ab, ag) < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (dot(ba, bg) < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}

	return result;
}



bool DoSimplexTriangle(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3((simplex[1] - simplex[2]), 0);
	vec3 ac = vec3((simplex[0] - simplex[2]), 0);
	vec3 ag = vec3((vec2(0, 0) - simplex[2]), 0);
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		// NOTE:	  1  0
		// simplex = [A, C]
		simplex[1] = simplex[2]; 
		simplex[2] = vec2();
		*simplexType = Simplex_Line;
		vec3 dInR3 = cross(cross(ac, ag), ac);
		*D = vec2(dInR3.x, dInR3.y);
	}
	else
	{
		if (dot(cross(ab, abc), ag) > 0)
		{
			// NOTE:	  1  0
			// simplex = [A, B]
			simplex[0] = simplex[1];
			simplex[1] = simplex[2];
			simplex[2] = vec2();
			*simplexType = Simplex_Line;
			vec3 dInR3 = cross(cross(ab, ag), ab);
			*D = vec2(dInR3.x, dInR3.y);
		}
		else
		{
			// NOTE: In the 2D case this means the origin is within the triangle.
			result = true;
		}
	}

	return result;
}

bool DoSimplexTriangleCasey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	vec3 ab = vec3((simplex[1] - simplex[2]), 0);
	vec3 ac = vec3((simplex[0] - simplex[2]), 0);
	vec3 ag = vec3((vec2(0,0) - simplex[2]), 0);
	vec3 abc = cross(ab, ac);

	if (dot(cross(abc, ac), ag) > 0)
	{
		if (dot(ac, ag) > 0)
		{
			// CASE 1

			// NOTE		  1  0
			// simplex = [A, C]
			simplex[1] = simplex[2];
			simplex[2] = vec2();
			*simplexType = Simplex_Line;

			vec3 dInR3 = cross(cross(ac, ag), ac);
			*D = vec2(dInR3.x, dInR3.y);
		}
		else
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				// CASE 4

				// NOTE		  1  0
				// simplex = [A, B]
				simplex[0] = simplex[1];
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				*simplexType = Simplex_Line;

				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				// CASE 5
				Assert(false);

				// NOTE		  0
				// simplex = [A]
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*simplexType = Simplex_Point;

				*D = vec2(ag.x, ag.y);
			}
		}
	}
	else
	{
		if (dot(cross(ab, abc), ag) > 0)
		{
			// Magical special fun times case
			if (dot(ab, ag) > 0)
			{
				// CASE 4

				// NOTE		  1  0
				// simplex = [A, B]
				simplex[0] = simplex[1];
				simplex[1] = simplex[2];
				simplex[2] = vec2();
				*simplexType = Simplex_Line;

				vec3 dInR3 = cross(cross(ab, ag), ab);
				*D = vec2(dInR3.x, dInR3.y);
			}
			else
			{
				// CASE 5
				Assert(false);

				// NOTE		  0
				// simplex = [A]
				simplex[0] = simplex[2];
				simplex[1] = vec2();
				simplex[2] = vec2();
				*simplexType = Simplex_Point;

				*D = vec2(ag.x, ag.y);
			}
		}
		else
		{
			// CASE 2 AND CASE 3

			// NOTE: In the 2D case this means the origin is within the triangle.
			result = true;
			// NOTE: This is for the 3D case.
			/*if (dot(abc, ag) > 0)
			{
				*D = abc;
			}
			else
			{
				// NOTE: Swap points so plane normal will be in the direction of the new point.
				vec3 t = simplex[1];
				simplex[1] = simplex[0];
				simplex[0] = t;
				*D = -abc;
			}*/
		}
	}

	return result;
}

DoSimplexResult DoSimplexTriangleAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	// Vectors
	vec3 ab = simplex.B - simplex.A;
	vec3 ac = simplex.C - simplex.A;
 	vec3 ag = vec3(0, 0, 0) - simplex.A;

	vec3 bc = simplex.C - simplex.B;
	vec3 ba = simplex.A - simplex.B;
	vec3 bg = vec3(0, 0, 0) - simplex.B;

	vec3 ca = simplex.A - simplex.C;
	vec3 cb = simplex.B - simplex.C;
	vec3 cg = vec3(0, 0, 0) - simplex.C;

	vec3 sn = cross(ab, ac);

	
	// Tests
	F32 ab_DOT_ag = dot(ab, ag);
	F32 ac_DOT_ag = dot(ac, ag);

	F32 ba_DOT_bg = dot(ba, bg);
	F32 bc_DOT_bg = dot(bc, bg);

	F32 ca_DOT_cg = dot(ca, cg);
	F32 cb_DOT_cg = dot(cb, cg);

	F32 perpAB_DOT_ag = dot(cross(ab, sn), ag);
	F32 perpAC_DOT_ag = dot(cross(sn, ac), ag);
	F32 perpBC_DOT_bg = dot(cross(bc, sn), bg);
	F32 perpCB_DOT_cg = dot(cross(sn, cb), cg);
	F32 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
			 bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
			 cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
			 ba_DOT_bg >= 0 &&
			 perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}
	else if (ac_DOT_ag >= 0 &&
			 ca_DOT_cg >= 0 &&
			 perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = cross(cross(ac, ag), ac);
	}
	else if (bc_DOT_bg >= 0 &&
			 cb_DOT_cg >= 0 &&
			 perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(bc, bg), bc);
	}
	else if (perpAB_DOT_ag <= 0 &&
			 perpAC_DOT_ag <= 0 &&
			 perpBC_DOT_bg <= 0 &&
			 sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.B;
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
			 perpAC_DOT_ag <= 0 &&
			 perpBC_DOT_bg <= 0 &&
			 sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.d = -sn;
	}

	//result.d = normalize(result.d);
	return result;
}

DoSimplexResult DoSimplexTriangleAllVoronoiDouble(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	// Vectors
	dvec3 ab = dvec3(simplex.B) - dvec3(simplex.A);
	dvec3 ac = dvec3(simplex.C) - dvec3(simplex.A);
	dvec3 ag = dvec3(0, 0, 0) - dvec3(simplex.A);

	dvec3 bc = dvec3(simplex.C) - dvec3(simplex.B);
	dvec3 ba = dvec3(simplex.A) - dvec3(simplex.B);
	dvec3 bg = dvec3(0, 0, 0) - dvec3(simplex.B);

	dvec3 ca = dvec3(simplex.A) - dvec3(simplex.C);
	dvec3 cb = dvec3(simplex.B) - dvec3(simplex.C);
	dvec3 cg = dvec3(0, 0, 0) - dvec3(simplex.C);

	dvec3 sn = cross(ab, ac);


	// Tests
	F64 ab_DOT_ag = dot(ab, ag);
	F64 ac_DOT_ag = dot(ac, ag);

	F64 ba_DOT_bg = dot(ba, bg);
	F64 bc_DOT_bg = dot(bc, bg);

	F64 ca_DOT_cg = dot(ca, cg);
	F64 cb_DOT_cg = dot(cb, cg);

	F64 perpAB_DOT_ag = dot(cross(ab, sn), ag);
	F64 perpAC_DOT_ag = dot(cross(sn, ac), ag);
	F64 perpBC_DOT_bg = dot(cross(bc, sn), bg);
	F64 perpCB_DOT_cg = dot(cross(sn, cb), cg);
	F64 sn_DOT_ag = dot(sn, ag);

	if (ab_DOT_ag < 0 &&
		ac_DOT_ag < 0)
	{
		// Voronoi Region A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = ag;
	}
	else if (ba_DOT_bg < 0 &&
		bc_DOT_bg < 0)
	{
		// Voronoi Region B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = bg;
	}
	else if (ca_DOT_cg < 0 &&
		cb_DOT_cg < 0)
	{
		// Voronoi Region C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = cg;
	}
	else if (ab_DOT_ag >= 0 &&
		ba_DOT_bg >= 0 &&
		perpAB_DOT_ag > 0)
	{
		// Voronoi Region AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(ab, ag), ab);
	}
	else if (ac_DOT_ag >= 0 &&
		ca_DOT_cg >= 0 &&
		perpAC_DOT_ag > 0)
	{
		// Voronoi Region AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = normalize(cross(cross(ac, ag), ac));
	}
	else if (bc_DOT_bg >= 0 &&
		cb_DOT_cg >= 0 &&
		perpBC_DOT_bg > 0)
	{
		// Voronoi Region BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(bc, bg), bc);
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag > 0)
	{
		// Voronoi Region ABC Above
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.B;
		result.d = sn;
	}
	else if (perpAB_DOT_ag <= 0 &&
		perpAC_DOT_ag <= 0 &&
		perpBC_DOT_bg <= 0 &&
		sn_DOT_ag <= 0)
	{
		// Voronoi Region ABC Below
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.d = -sn;
	}

	//result.d = normalize(result.d);
	return result;
}



bool DoSimplexTetrahedron(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	return result;
}

DoSimplexResult DoSimplexTetrahedronAllVoronoi(Simplex simplex, vec3 D)
{
	DoSimplexResult result;
	result.containsGoal = false;

	vec3 G = vec3(0,0,0);
	vec3 AG = G - simplex.A;
	vec3 AB = simplex.B - simplex.A;
	vec3 AC = simplex.C - simplex.A;
	vec3 AD = simplex.D - simplex.A;

	vec3 BG = G - simplex.B;
	vec3 BA = simplex.A - simplex.B;
	vec3 BC = simplex.C - simplex.B;
	vec3 BD = simplex.D - simplex.B;
	
	vec3 CG = G - simplex.C;
	vec3 CA = simplex.A - simplex.C;
	vec3 CB = simplex.B - simplex.C;
	vec3 CD = simplex.D - simplex.C;

	vec3 DG = G - simplex.D;
	vec3 DA = simplex.A - simplex.D;
	vec3 DB = simplex.B - simplex.D;
	vec3 DC = simplex.C - simplex.D;

	vec3 normalABC = cross(AB,AC);
	vec3 normalADB = cross(AD,AB);
	vec3 normalACD = cross(AC,AD);

	vec3 normalBCA = cross(BC,BA);
	vec3 normalBDC = cross(BD,BC);
	vec3 normalBAD = cross(BA,BD);

	vec3 normalCDA = cross(CD,CA);
	vec3 normalCBD = cross(CB,CD);

	if ( dot(AG,AB)<0 &&
		 dot(AG,AC)<0 &&
		 dot(AG,AD)<0)
	{
		// Voronoi A
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.A;
		result.d = AG;
	}
	else if ( dot(BG,BA)<0 &&
			  dot(BG,BC)<0 &&
			  dot(BG,BD)<0)
	{
		// Voronoi B
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.B;
		result.d = BG;
	}
	else if ( dot(CG,CA)<0 &&
			  dot(CG,CB)<0 &&
			  dot(CG,CD)<0)
	{
		// Voronoi C
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.C;
		result.d = CG;
	}
	else if ( dot(DG,DA)<0 &&
			  dot(DG,DB)<0 &&
			  dot(DG,DC)<0)
	{
		// Voronoi D
		result.simplex.type = Simplex_Point;
		result.simplex.A = simplex.D;
		result.d = DG;
	}
	// NOTE: Sign on the plane tests may need to be >= but i dont think so, investigate if errors crop up
	else if (dot(AG,AB)>=0 &&
			 dot(BG,BA)>=0 &&
			 dot(AG,cross(AB,normalABC))>0 &&
			 dot(AG,cross(normalADB,AB))>0)
	{
		// Voronoi AB
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.d = cross(cross(AB,AG),AB);
	}
	else if (dot(AG,AC)>=0 &&
			 dot(CG,CA)>=0 &&
			 dot(AG,cross(AC,normalACD))>0 &&
			 dot(AG,cross(normalABC,AC))>0)
	{
		// Voronoi AC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.d = cross(cross(AC,AG),AC);
	}
	else if (dot(AG,AD)>=0 &&
			 dot(DG,DA)>=0 &&
			 dot(AG,cross(AD,normalADB))>0 &&
			 dot(AG,cross(normalACD,AD))>0)
	{
		// Voronoi AD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.D;
		result.d = cross(cross(AD,AG),AD);
	}
	else if (dot(BG,BC)>=0 &&
			 dot(CG,CB)>=0 &&
			 dot(BG,cross(BC,normalBCA))>0 &&
			 dot(BG,cross(normalBDC,BC))>0)
	{
		// Voronoi BC
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.d = cross(cross(BC,BG),BC);
	}
	else if (dot(BG,BD)>=0 &&
			 dot(DG,DB)>=0 &&
			 dot(BG,cross(BD,normalBDC))>0 &&
			 dot(BG,cross(normalBAD,BD))>0 )
	{
		// Voronoi BD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.D;
		result.d = cross(cross(BD,BG),BD);
	}
	else if (dot(CG,CD)>=0 &&
			 dot(DG,DC)>=0 &&
			 dot(BG,cross(BD,normalBDC))>0 &&
			 dot(BG,cross(normalBAD,BD))>0)
	{
		// Voronoi CD
		result.simplex.type = Simplex_Line;
		result.simplex.A = simplex.C;
		result.simplex.B = simplex.D;
		result.d = cross(cross(CD,CG),CD);
	}
	else if ((dot(AG,normalABC))*(dot(AD,normalABC))<0)
	{
		// Voronoi ABC
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		if (dot(AG, normalABC)>0)
		{
			result.d = normalABC;
		}
		else
		{
			result.d = -normalABC;
		}
	}
	else if ((dot(AG,normalADB))*(dot(AC,normalADB))<0)
	{
		// Voronoi ABD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.D;
		if (dot(AG, normalADB)>0)
		{
			result.d = normalADB;
		}
		else
		{
			result.d = -normalADB;
		}
	}
	else if ((dot(AG,normalACD))*(dot(AB,normalACD))<0)
	{
		// Voronoi ACD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.D;
		if (dot(AG, normalACD)>0)
		{
			result.d = normalACD;
		}
		else
		{
			result.d = -normalACD;
		}

	}
	else if ((dot(BG,normalBDC))*(dot(BA,normalBDC))<0)
	{
		// Voronoi BCD
		result.simplex.type = Simplex_Triangle;
		result.simplex.A = simplex.B;
		result.simplex.B = simplex.C;
		result.simplex.C = simplex.D;
		if (dot(BG, normalBDC)>0)
		{
			result.d = normalBDC;
		}
		else
		{
			result.d = -normalBDC;
		}
	}
	else
	{
		// Voronoi ABCD
		result.containsGoal = true;
		result.simplex.type = Simplex_Tetrahedron;
		result.simplex.A = simplex.A;
		result.simplex.B = simplex.B;
		result.simplex.C = simplex.C;
		result.simplex.D = simplex.D;
		// NOTE: If d is needed from here remember it is not set.
	}

	return result;
}



bool DoSimplex__(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 vec2 tsimplex[4];
						 SimplexType tsimplexType;
						 vec2 tD;

						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;
						 result = DoSimplexLine(tsimplex, &tsimplexType, &tD);
						 
						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;
						 bool resultCasey = DoSimplexLineCasey(tsimplex, &tsimplexType, &tD);

						 tsimplex[0] = simplex[0];
						 tsimplex[1] = simplex[1];
						 tsimplex[2] = simplex[2];
						 tsimplex[3] = simplex[3];
						 tsimplexType = *simplexType;
						 tD = *D;

						 Simplex s;
						 s.type = tsimplexType;
						 s.A = vec3(tsimplex[1].x, tsimplex[1].y, 0);
						 s.B = vec3(tsimplex[0].x, tsimplex[0].y, 0);

						 DoSimplexResult resultAllVoronoi = DoSimplexLineAllVoronoi( s, vec3(tD.x, tD.y, 0));

						 tsimplexType = resultAllVoronoi.simplex.type;
						 if (tsimplexType == Simplex_Point)
						 {
							 tsimplex[0] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
							 tsimplex[1] = vec2(0, 0);
							 tsimplex[2] = vec2(0, 0);
							 tsimplex[3] = vec2(0, 0);
						 }
						 else if (tsimplexType == Simplex_Line)
						 {
							 tsimplex[0] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
							 tsimplex[1] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
							 tsimplex[2] = vec2(0, 0);
							 tsimplex[3] = vec2(0, 0);
						 }
						 tD = vec2(resultAllVoronoi.d.x, resultAllVoronoi.d.y);



						 Assert(resultCasey == result);
						 Assert(resultCasey == resultAllVoronoi.containsGoal);
						 Assert(result == resultAllVoronoi.containsGoal);



						 simplex[0] = tsimplex[0];
						 simplex[1] = tsimplex[1];
						 simplex[2] = tsimplex[2];
						 simplex[3] = tsimplex[3];
						 *simplexType = tsimplexType;
						 *D = tD;

						 result = resultCasey;
						 break;
	}

	case Simplex_Triangle:
	{
							 vec2 tsimplex[4];
							 SimplexType tsimplexType;
							 vec2 tD;

							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;
							 result = DoSimplexTriangle(tsimplex, &tsimplexType, &tD);
							 
							 
							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;
							 bool resultCasey = DoSimplexTriangleCasey(tsimplex, &tsimplexType, &tD);


							 //
							 tsimplex[0] = simplex[0];
							 tsimplex[1] = simplex[1];
							 tsimplex[2] = simplex[2];
							 tsimplex[3] = simplex[3];
							 tsimplexType = *simplexType;
							 tD = *D;

							 Simplex s;
							 s.type = tsimplexType;
							 s.A = vec3(tsimplex[2].x, tsimplex[2].y, 0);
							 s.B = vec3(tsimplex[1].x, tsimplex[1].y, 0);
							 s.C = vec3(tsimplex[0].x, tsimplex[0].y, 0);

							 DoSimplexResult resultAllVoronoi = DoSimplexTriangleAllVoronoi(s, vec3(tD.x, tD.y, 0));

							 tsimplexType = resultAllVoronoi.simplex.type;
							 if (tsimplexType == Simplex_Point)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[1] = vec2(0, 0);
								 tsimplex[2] = vec2(0, 0);
								 tsimplex[3] = vec2(0, 0);
							 }
							 else if (tsimplexType == Simplex_Line)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
								 tsimplex[1] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[2] = vec2(0, 0);
								 tsimplex[3] = vec2(0, 0);
							 }
							 else if (tsimplexType == Simplex_Triangle)
							 {
								 tsimplex[0] = vec2(resultAllVoronoi.simplex.C.x, resultAllVoronoi.simplex.C.y);
								 tsimplex[1] = vec2(resultAllVoronoi.simplex.B.x, resultAllVoronoi.simplex.B.y);
								 tsimplex[2] = vec2(resultAllVoronoi.simplex.A.x, resultAllVoronoi.simplex.A.y);
								 tsimplex[3] = vec2(0, 0);
								 resultAllVoronoi.containsGoal = true;
							 }
							 tD = vec2(resultAllVoronoi.d.x, resultAllVoronoi.d.y);



							 Assert(resultCasey == result);
							 Assert(resultCasey == resultAllVoronoi.containsGoal);
							 Assert(result == resultAllVoronoi.containsGoal);
							 //




							 //Assert(resultCasey == result);

							 simplex[0] = tsimplex[0];
							 simplex[1] = tsimplex[1];
							 simplex[2] = tsimplex[2];
							 simplex[3] = tsimplex[3];
							 *simplexType = tsimplexType;
							 *D = tD;

							 result = resultCasey;
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

bool DoSimplex(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{

						 result = DoSimplexLine(simplex, simplexType, D);
						 break;
	}

	case Simplex_Triangle:
	{

							 result = DoSimplexTriangle(simplex, simplexType, D);
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

bool DoSimplex_Casey(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 
						 result = DoSimplexLineCasey(simplex, simplexType, D);
						 break;
	}

	case Simplex_Triangle:
	{
							 
							 result = DoSimplexTriangleCasey(simplex, simplexType, D);
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}

//#define DOUBLES 1
bool DoSimplex_AllVoronoi(vec2* simplex, SimplexType* simplexType, vec2* D)
{
	bool result = false;

	switch (*simplexType)
	{
	case Simplex_Line:
	{
						 Simplex s;
						 s.type = *simplexType;
						 s.A = vec3(simplex[1].x, simplex[1].y, 0);
						 s.B = vec3(simplex[0].x, simplex[0].y, 0);

#ifndef DOUBLES
						 DoSimplexResult rav = DoSimplexLineAllVoronoi(s, vec3(D->x, D->y, 0));
#else
						 DoSimplexResult rav = DoSimplexLineAllVoronoiDouble(s, vec3(D->x, D->y, 0));
#endif

						 if (rav.simplex.type == Simplex_Point)
						 {
							 simplex[0] = vec2(rav.simplex.A.x, rav.simplex.A.y);
							 simplex[1] = vec2(0, 0);
							 simplex[2] = vec2(0, 0);
							 simplex[3] = vec2(0, 0);
						 }
						 else if (rav.simplex.type == Simplex_Line)
						 {
							 simplex[0] = vec2(rav.simplex.B.x, rav.simplex.B.y);
							 simplex[1] = vec2(rav.simplex.A.x, rav.simplex.A.y);
							 simplex[2] = vec2(0, 0);
							 simplex[3] = vec2(0, 0);
						 }

						 *simplexType = rav.simplex.type;
						 *D = vec2(rav.d.x, rav.d.y);
						 result = rav.containsGoal;
						 break;
	}

	case Simplex_Triangle:
	{
							 Simplex s;
							 s.type = *simplexType;
							 s.A = vec3(simplex[2].x, simplex[2].y, 0);
							 s.B = vec3(simplex[1].x, simplex[1].y, 0);
							 s.C = vec3(simplex[0].x, simplex[0].y, 0);

#ifndef DOUBLES
							 DoSimplexResult rav = DoSimplexTriangleAllVoronoi(s, vec3(D->x, D->y, 0));
#else
							 DoSimplexResult rav = DoSimplexTriangleAllVoronoiDouble(s, vec3(D->x, D->y, 0));
#endif

							 if (rav.simplex.type == Simplex_Point)
							 {
								 simplex[0] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[1] = vec2(0, 0);
								 simplex[2] = vec2(0, 0);
								 simplex[3] = vec2(0, 0);
							 }
							 else if (rav.simplex.type == Simplex_Line)
							 {
								 simplex[0] = vec2(rav.simplex.B.x, rav.simplex.B.y);
								 simplex[1] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[2] = vec2(0, 0);
								 simplex[3] = vec2(0, 0);
							 }
							 else if (rav.simplex.type == Simplex_Triangle)
							 {
								 simplex[0] = vec2(rav.simplex.C.x, rav.simplex.C.y);
								 simplex[1] = vec2(rav.simplex.B.x, rav.simplex.B.y);
								 simplex[2] = vec2(rav.simplex.A.x, rav.simplex.A.y);
								 simplex[3] = vec2(0, 0);
								 rav.containsGoal = true;
							 }

							 *simplexType = rav.simplex.type;
							 *D = vec2(rav.d.x, rav.d.y);
							 result = rav.containsGoal;
							 break;
	}

	case Simplex_Tetrahedron:
	{
								result = DoSimplexTetrahedron(simplex, simplexType, D);
								break;
	}

	default:
	{
			   break;
	}
	}

	return result;
}






template<typename S1, typename S2>
bool GJK(S1 shapeA, S2 shapeB,  U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (;loopCounter < 10;)
	{
		++loopCounter;
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}

template<typename S1, typename S2>
bool GJK_Casey(S1 shapeA, S2 shapeB, U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex_Casey(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}

template<typename S1, typename S2>
bool GJK_AllVoronoi(S1 shapeA, S2 shapeB, U32* numberOfLoopsCompleted = NULL)
{
	bool collisionDetected = false;

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	U32 loopCounter = 0;
	for (; loopCounter < 10;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (A == vec2(0.0f, 0.0f))
		{
			collisionDetected = true;
			break;
		}
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex_AllVoronoi(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}

	if (numberOfLoopsCompleted)
	{
		*numberOfLoopsCompleted = loopCounter;
	}

	return collisionDetected;
}





// r rotated 0 degrees
void CollisionTestsRectRect1()
{
	Rectangle g;
	g.origin = vec2(0, 0);
	g.halfDim = vec2(.5, .5);
	g.transform = mat3(1,0,0,   0,1,0,   .5,.5,1);


	F32 xs[7] = {-1, -.5, 0, .5, 1, 1.5, 2};
	F32 ys[7] = { 2, 1.5, 1, .5, 0, -.5, -1 };
	U32 hits[7][7];

	for (U32 y = 0; y < 7; ++y)
	{
		for (U32 x = 0; x < 7; ++x)
		{
			Rectangle r;
			r.origin = vec2(0,0);
			r.halfDim = vec2(.5, .5);
			r.transform = mat3(1,0,0,   0,1,0,   xs[x],ys[y],1);

			bool collision = GJK(g, r);
			if (collision)
			{
				hits[y][x] = 1;
			}
			else
			{
				hits[y][x] = 0;
			}
		}
	}

	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits %u %u %u %u %u %u %u\n", hits[i][0], hits[i][1], hits[i][2], hits[i][3], hits[i][4], hits[i][5], hits[i][6]);
	}
}

// r rotated 45 degrees 
void CollisionTestsRectRect2()
{
	Rectangle g;
	g.origin = vec2(0, 0);
	g.halfDim = vec2(.5, .5);
	g.transform = mat3(1, 0, 0, 0, 1, 0, .5, .5, 1);

	F32 angle = DegreesToRadians(45.0f);

	F32 xs[7] = { -1, -.5, 0, .5, 1, 1.5, 2 };
	F32 ys[7] = { 2, 1.5, 1, .5, 0, -.5, -1 };
	U32 hits_Simple[7][7];
	U32 hits_Casey[7][7];
	U32 hits_AllVoronoi[7][7];

	for (U32 y = 0; y < 7; ++y)
	{
		for (U32 x = 0; x < 7; ++x)
		{
			Rectangle r;
			r.origin = vec2(0, 0);
			r.halfDim = vec2(.5, .5);
			r.transform = mat3(1, 0, 0, 0, 1, 0, xs[x], ys[y], 1) * mat3(cos(angle),sin(angle),0,   -sin(angle),cos(angle),0,   0,0,1);

			bool collisionSimple = GJK(g, r);
			bool collisionCasey = GJK_Casey(g, r);
			bool collisionAllVoronoi = GJK_AllVoronoi(g, r);

			Assert(collisionSimple == collisionCasey);
			Assert(collisionSimple == collisionAllVoronoi);
			Assert(collisionCasey == collisionAllVoronoi);

			if (collisionSimple)
			{
				hits_Simple[y][x] = 1;
			}
			else
			{
				hits_Simple[y][x] = 0;
			}

			if (collisionCasey)
			{
				hits_Casey[y][x] = 1; 
			}
			else
			{
				hits_Casey[y][x] = 0;
			}

			if (collisionAllVoronoi)
			{
				hits_AllVoronoi[y][x] = 1;
			}
			else
			{
				hits_AllVoronoi[y][x] = 0;
			}
		}
	}

	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_Simple %u %u %u %u %u %u %u\n", hits_Simple[i][0], hits_Simple[i][1], hits_Simple[i][2], hits_Simple[i][3], hits_Simple[i][4], hits_Simple[i][5], hits_Simple[i][6]);
	}
	DebugPrint("\n\n");
	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_Casey %u %u %u %u %u %u %u\n", hits_Casey[i][0], hits_Casey[i][1], hits_Casey[i][2], hits_Casey[i][3], hits_Casey[i][4], hits_Casey[i][5], hits_Casey[i][6]);
	}
	DebugPrint("\n\n");
	for (U32 i = 0; i < 7; ++i)
	{
		DebugPrintf(1024, "hits_AllVoronoi %u %u %u %u %u %u %u\n", hits_AllVoronoi[i][0], hits_AllVoronoi[i][1], hits_AllVoronoi[i][2], hits_AllVoronoi[i][3], hits_AllVoronoi[i][4], hits_AllVoronoi[i][5], hits_AllVoronoi[i][6]);
	}
}






// NOTE: These tests are to test whether simplified GJK provides the same results as Casey's implementation.
void CollisionSupportsTests()
{
	RandomNumberGenerator shapeRandomGenerator;
	SeedRandomNumberGenerator(&shapeRandomGenerator, 1);
	const U32 numShapes = 100;
	Rectangle rectangles[numShapes];
	Circle    circles[numShapes];
	Triangle  triangles[numShapes];

	// Init randomly sized and positioned rectangles
	for (U32 i = 0; i < numShapes; ++i)
	{
		Rectangle r;

		F32 angle = RandomF32Between(&shapeRandomGenerator, 0.0f, 360.0f);
		F32 scaleXY = RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f);
		F32 positionX = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);
		F32 positionY = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);
		mat3 scale = mat3(scaleXY, 0.0f, 0.0f, 0.0f, scaleXY, 0.0f, 0.0f, 0.0f, 1.0f);
		mat3 rotation = mat3(cos(angle), sin(angle), 0.0f, -sin(angle), cos(angle), 0.0f, 0.0f, 0.0f, 1.0f);
		mat3 translation = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, positionX, positionY, 1.0f);
		r.transform = translation * rotation * scale;

		r.halfDim.x = RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f);
		r.halfDim.y = RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f);
		
		r.origin = vec2(0.0f, 0.0f);

		rectangles[i] = r;
	}

	// Init randomly sized and positioned circles
	for (U32 i = 0; i < numShapes; ++i)
	{
		Circle c;

		c.origin.x = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);
		c.origin.y = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);

		c.radius = RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f);

		circles[i] = c;
	}

	// Init randomly sized and positioned triangles
	for (U32 i = 0; i < numShapes; ++i)
	{
		Triangle t;

		F32 angle = RandomF32Between(&shapeRandomGenerator, 0.0f, 360.0f);
		F32 scaleXY = RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f);
		F32 positionX = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);
		F32 positionY = RandomF32Between(&shapeRandomGenerator, -100.0f, 100.0f);
		mat3 scale = mat3(scaleXY, 0.0f, 0.0f, 0.0f, scaleXY, 0.0f, 0.0f, 0.0f, 1.0f);
		mat3 rotation = mat3(cos(angle), sin(angle), 0.0f, -sin(angle), cos(angle), 0.0f, 0.0f, 0.0f, 1.0f);
		mat3 translation = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, positionX, positionY, 1.0f);
		t.transform = translation * rotation * scale;

		t.origin = vec2(0.0f, 0.0f);

		// NOTE: Counter clockwise winding 
		// NOTE:     2
		// NOTE:
		// NOTE:  0      1
		t.points[0] = vec2(0.0f, 0.0f);
		t.points[1] = t.points[0] + vec2(RandomF32Between(&shapeRandomGenerator, 0.1f, 100.0f), RandomF32Between(&shapeRandomGenerator, -50.0f, 0.0f));
		t.points[2] = vec2(RandomF32Between(&shapeRandomGenerator, 0.0f, 100.0f), RandomF32Between(&shapeRandomGenerator, 0.0f, 50.0f));
	
		triangles[i] = t;
	}

	U32 numberOfCollisions = 0;
	U32 numberOfNoncollisions = 0;

	U32 numberOfRectRectCollisions = 0;
	U32 numberOfRectCircleCollisions = 0;
	U32 numberOfRectTriangleCollisions = 0;
	U32 numberOfRectRectNonCollisions = 0;
	U32 numberOfRectCircleNonCollisions = 0;
	U32 numberOfRectTriangleNonCollisions = 0;

	U32 numberOfCircleCircleCollisions = 0;
	U32 numberOfCircleTriangleCollisions = 0;
	U32 numberOfCircleCircleNonCollisions = 0;
	U32 numberOfCircleTriangleNonCollisions = 0;

	U32 numberOfTriangleTriangleCollisions = 0;
	U32 numberOfTriangleTriangleNonCollisions = 0;

	// Check collision against all shapes
	for (U32 i = 0; i < numShapes; ++i)
	{
		for (U32 j = 0; j < numShapes; ++j)
		{
			bool collision = GJK(rectangles[i], rectangles[j]);
			if (collision)
			{
				++numberOfCollisions;
				++numberOfRectRectCollisions;
			}
			else
			{
				++numberOfNoncollisions;
				++numberOfRectRectNonCollisions;
			}
		}

// 		for (U32 j = 0; j < numShapes; ++j)
// 		{
// 			bool collision = GJK(rectangles[i], circles[j]);
// 			if (collision)
// 			{
// 				++numberOfCollisions;
// 				++numberOfRectCircleCollisions;
// 			}
// 			else
// 			{
// 				++numberOfNoncollisions;
// 				++numberOfRectCircleNonCollisions;
// 			}
// 		}
// 
// 		for (U32 j = 0; j < numShapes; ++j)
// 		{
// 			bool collision = GJK(rectangles[i], triangles[j]);
// 			if (collision)
// 			{
// 				++numberOfCollisions;
// 				++numberOfRectTriangleCollisions;
// 			}
// 			else
// 			{
// 				++numberOfNoncollisions;
// 				++numberOfRectTriangleNonCollisions;
// 			}
// 		}
	}

// 	for (U32 i = 0; i < numShapes; ++i)
// 	{
// 		for (U32 j = 0; j < numShapes; ++j)
// 		{
// 			bool collision = GJK(circles[i], circles[j]);
// 			if (collision)
// 			{
// 				++numberOfCollisions;
// 				++numberOfCircleCircleCollisions;
// 			}
// 			else
// 			{
// 				++numberOfNoncollisions;
// 				++numberOfCircleCircleNonCollisions;
// 			}
// 		}
// 
// 		for (U32 j = 0; j < numShapes; ++j)
// 		{
// 			bool collision = GJK(circles[i], triangles[j]);
// 			if (collision)
// 			{
// 				++numberOfCollisions;
// 				++numberOfCircleTriangleCollisions;
// 			}
// 			else
// 			{
// 				++numberOfNoncollisions;
// 				++numberOfCircleTriangleNonCollisions;
// 			}
// 		}
// 	}
// 
// 	for (U32 i = 0; i < numShapes; ++i)
// 	{
// 		bool collision = GJK(triangles[i], triangles[i]);
// 		if (collision)
// 		{
// 			++numberOfCollisions;
// 			++numberOfTriangleTriangleCollisions;
// 		}
// 		else
// 		{
// 			++numberOfNoncollisions;
// 			++numberOfTriangleTriangleNonCollisions;
// 		}
// 	}

	DebugPrintf(512, "There were %u collisions in the set.\n", numberOfCollisions);
	DebugPrintf(512, "There were %u non-collisions in the set.\n\n", numberOfNoncollisions);

	DebugPrintf(512, "There were %u rect rect collisions in the set.\n", numberOfRectRectCollisions);
	DebugPrintf(512, "There were %u rect rect non-collisions in the set.\n", numberOfRectRectNonCollisions);
	DebugPrintf(512, "There were %u rect circle collisions in the set.\n", numberOfRectCircleCollisions);
	DebugPrintf(512, "There were %u rect circle non-collisions in the set.\n", numberOfRectCircleNonCollisions);
	DebugPrintf(512, "There were %u rect triangle collisions in the set.\n", numberOfRectTriangleCollisions);
	DebugPrintf(512, "There were %u rect triangle non-collisions in the set.\n\n", numberOfRectTriangleNonCollisions);

	DebugPrintf(512, "There were %u circle circle collisions in the set.\n", numberOfCircleCircleCollisions);
	DebugPrintf(512, "There were %u circle circle non-collisions in the set.\n", numberOfCircleCircleNonCollisions);
	DebugPrintf(512, "There were %u circle triangle collisions in the set.\n", numberOfCircleTriangleCollisions);
	DebugPrintf(512, "There were %u circle triangle non-collisions in the set.\n\n", numberOfCircleTriangleNonCollisions);

	DebugPrintf(512, "There were %u triangle triangle collisions in the set.\n", numberOfTriangleTriangleCollisions);
	DebugPrintf(512, "There were %u triangle triangle non-collisions in the set.\n", numberOfTriangleTriangleNonCollisions);
}






void CollisionDetection()
{
	bool collisionDetected = false;

	Rectangle shapeA;
	shapeA.transform = mat3(1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f);
	shapeA.origin = vec2(2.0f, 2.0f);
	shapeA.halfDim = vec2(1.0f, 1.0f);

	Rectangle shapeB;
	shapeB.transform = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	shapeB.origin = vec2(3.0f, 3.0f);
	shapeB.halfDim = vec2(1.0f, 1.0f);

	vec2 S = Support(&shapeA, &shapeB, &vec2(1.0f, -1.0f));
	vec2 simplex[4];
	SimplexType simplexType = Simplex_Point;
	simplex[0] = S;
	vec2 D = -S;

	for (;;)
	{
		vec2 A = Support(&shapeA, &shapeB, &D);
		if (dot(A, D) < 0)
		{
			collisionDetected = false;
			break;
		}
		simplexType = (SimplexType)(simplexType + 1);
		simplex[simplexType] = A;
		if (DoSimplex(simplex, &simplexType, &D))
		{
			collisionDetected = true;
			break;
		}
	}
}



struct verifyShaderReturnResult
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

};
verifyShaderReturnResult verifyShader(GLuint shader);

struct verifyProgramReturnResult
{
	bool compiled;
	char* infoLog;
	I64 infoLogLength;

};
verifyProgramReturnResult verifyProgram(GLuint program);






const GLuint numVertices = 4;
const GLuint vertexDimensionality = 2;
const GLuint textureSpaceDimensionality = 2;
GLuint texturedQuadVAO;


// Textured Quad Shader Things
GLuint texturedQuadShaderProgram;

GLuint spriteSamplerLocation;
GLuint PCMLocation;

GLuint texture;
GLuint textureSampler;

// Solid Color Quad Shader Things
GLuint solidColorQuadShaderProgram;
GLuint solidColorQuadPCMLocation;
GLuint solidColorQuadQuadColorLocation;

GLuint solidColorCircleInPointShaderProgram;
GLuint solidColorCircleInPointPCMLocation;
GLuint solidColorCircleInPointCircleColorLocation;

GLuint solidColorTriangleShaderProgram;
GLuint solidColorTrianglePCMLocation;
GLuint solidColorTriangleTriangleColorLocation;



struct Entity
{
	vec2 position;
	vec2 scale;
	F32 rotationAngle;
	GLuint texture;
};

struct Camera
{
	vec2 position;
	vec2 viewArea;
};

U32 numCollisionEntities = 2;
Rectangle cs2Rectangle;
Rectangle cs1Rectangle;
Triangle  cs1Triangle;
Circle    cs1Circle;
Camera collisionCamera;

GLuint gridVAO;
#define numGridLines 10
#define gridLinePointDimensionality 2
#define pointsPerLine 2

GLuint equalateralTriangleVAO;
#define numPointsInTriangle 3
#define equalateralTrianglePointDimensionality 2

GLuint circleVAO;
#define numPointsInCircle 1
#define circlePointDimensionality 2

void Init2DCollisionTestScene()
{
	collisionCamera.position = vec2(0,0);
	collisionCamera.viewArea = vec2(10,10);

	cs2Rectangle.origin = vec2(0,0);
	cs2Rectangle.halfDim = vec2(.5f, .5f);
	cs2Rectangle.transform = mat3(1,0,0,   0,1,0,   .5f,.5f,1);

	cs1Rectangle.origin = vec2(0, 0);
	cs1Rectangle.halfDim = vec2(.5f, .5f);

	// Grid
	glGenVertexArrays(1, &gridVAO);
	glBindVertexArray(gridVAO);
	GLfloat gridPositions[(numGridLines + numGridLines + 2) * pointsPerLine * gridLinePointDimensionality] = {
		// varying y values
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f,
		-numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f, numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f,

		// varying x values
		(-numGridLines / 2.0f) + 0.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 0.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 1.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 1.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 2.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 2.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 3.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 3.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 4.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 4.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 5.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 5.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 6.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 6.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 7.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 7.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 8.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 8.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 9.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 9.0f, numGridLines / 2.0f,
		(-numGridLines / 2.0f) + 10.0f, -numGridLines / 2.0f, (-numGridLines / 2.0f) + 10.0f, numGridLines / 2.0f,
	};
	GLuint gridVertexBuffer;
	glGenBuffers(1, &gridVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gridVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridPositions), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gridPositions), gridPositions);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);


	// Equilateral triangle
	glGenVertexArrays(1, &equalateralTriangleVAO);
	glBindVertexArray(equalateralTriangleVAO);

	GLfloat equalateralTrianglePoints[numPointsInTriangle * equalateralTrianglePointDimensionality] = {
		-0.6f, -0.3f,
		 0.6f, -0.3f,
		 0.0f,  0.6f
	};
	GLuint equalateralTriangleVertexBuffer;
	glGenBuffers(1, &equalateralTriangleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, equalateralTriangleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(equalateralTrianglePoints), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(equalateralTrianglePoints), equalateralTrianglePoints);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	cs1Triangle.origin = vec2(0, 0);
	cs1Triangle.points[0] = vec2(equalateralTrianglePoints[0], equalateralTrianglePoints[1]);
	cs1Triangle.points[1] = vec2(equalateralTrianglePoints[2], equalateralTrianglePoints[3]);
	cs1Triangle.points[2] = vec2(equalateralTrianglePoints[4], equalateralTrianglePoints[5]);

	// Circle
	glGenVertexArrays(1, &circleVAO);
	glBindVertexArray(circleVAO);

	GLfloat circleOrigin[2] = {0.0f, 0.0f};
	GLuint circleVertexBuffer;
	glGenBuffers(1, &circleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, circleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(circleOrigin), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(circleOrigin), circleOrigin);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	cs1Circle.origin = vec2(0, 0);
	cs1Circle.radius = 0.5f;
}






struct ViewFrustum
{
	F32 left;
	F32 right;
	F32 top;
	F32 bottom;
	F32 near;
	F32 far;
};

ViewFrustum CreateViewFrustum(F32 xFOV, F32 yFOV, F32 near, F32 far)
{
	ViewFrustum result;

	result.near = near;
	result.far = far;

	result.right = near * tan(DegreesToRadians(xFOV)/2.0f);
	result.left = -result.right;

	result.top = near * tan(DegreesToRadians(yFOV)/2.0f);
	result.bottom = -result.top;

	return result;
}

mat4 GetPerspectiveProjection(ViewFrustum frustum)
{
	mat4 result;

	result[0] = vec4((2.0f*frustum.near)/(frustum.right-frustum.left), 0.0f, 0.0f, 0.0f);
	result[1] = vec4(0.0f, (2.0f*frustum.near)/(frustum.top-frustum.bottom), 0.0f, 0.0f);
	result[2] = vec4((frustum.right+frustum.left)/(frustum.right-frustum.left), (frustum.top+frustum.bottom)/(frustum.top-frustum.bottom), -(frustum.far+frustum.near)/(frustum.far-frustum.near), -1.0f);
	result[3] = vec4(0.0f, 0.0f, -(2.0f*frustum.far*frustum.near)/(frustum.far-frustum.near), 0.0f);

	return result;
}

GLuint cubeVAO;
U64 numberOfCubeVertices;
U64 numberOfCubeIndices;

GLuint capsuleVAO;
U64 numberOfCapsuleWallVertices;
U64 numberOfCapsuleWallIndices;

GLuint sphereVAO;
U64 numberOfSphereVertices;
U64 numberOfSphereIndices;

struct OrientedBoundingBoxPoints
{
	vec4 points[8];
};

OrientedBoundingBoxPoints operator*(const mat4 &A, const OrientedBoundingBoxPoints &x)
{
	OrientedBoundingBoxPoints result;
	for (U32 i = 0; i < 8; ++i)
	{
		result.points[i] = A * x.points[i];
	}
	return result;
}

OrientedBoundingBoxPoints Homogenize(OrientedBoundingBoxPoints points)
{
	OrientedBoundingBoxPoints result;

	for (U32 i = 0; i < 8; ++i)
	{
		result.points[i] = points.points[i]/points.points[i].w;
	}

	return result;
}

OrientedBoundingBoxPoints GetOBBPoints(OrientedBoundingBox box)
{
	OrientedBoundingBoxPoints result;

	result.points[0] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[1] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[2] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[3] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z + box.halfDim.z, 1.0f);
	result.points[4] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[5] = box.transform * vec4(box.origin.x - box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[6] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y - box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);
	result.points[7] = box.transform * vec4(box.origin.x + box.halfDim.x, box.origin.y + box.halfDim.y, box.origin.z - box.halfDim.z, 1.0f);

	return result;
}

vec3 camera3DPosition;
vec3 camera3DRotations;
ViewFrustum camera3DViewFrustum;

OrientedBoundingBox collisionShape1;
OrientedBoundingBox collisionShape2;

void Init3DCollisionTestScene()
{
	F32* cubeVertices = NULL;
	U32* cubeIndices = NULL;
	Load3DModel("Assets/3D/Cube.obj", &cubeVertices, &numberOfCubeVertices, &cubeIndices, &numberOfCubeIndices);
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	GLuint cubeVertexBuffer;
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices[0])*numberOfCubeVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cubeVertices[0])*numberOfCubeVertices, cubeVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint cubeIndexBuffer;
	glGenBuffers(1, &cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices[0])*numberOfCubeIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(cubeIndices[0])*numberOfCubeIndices, cubeIndices);
	glBindVertexArray(NULL);



	F32* capsuleWallVertices = NULL;
	U32* capsuleWallIndices = NULL;
	Load3DModel("Assets/3D/CapsuleWall.obj", &capsuleWallVertices, &numberOfCapsuleWallVertices, &capsuleWallIndices, &numberOfCapsuleWallIndices);
	glGenVertexArrays(1, &capsuleVAO);
	glBindVertexArray(capsuleVAO);
	GLuint capsuleVertexBuffer;
	glGenBuffers(1, &capsuleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, capsuleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(capsuleWallVertices[0])*numberOfCapsuleWallVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(capsuleWallVertices[0])*numberOfCapsuleWallVertices, capsuleWallVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint capsuleIndexBuffer;
	glGenBuffers(1, &capsuleIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, capsuleIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(capsuleWallIndices[0])*numberOfCapsuleWallIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(capsuleWallIndices[0])*numberOfCapsuleWallIndices, capsuleWallIndices);
	glBindVertexArray(NULL);


	F32* sphereVertices = NULL;
	U32* sphereIndices = NULL;
	Load3DModel("Assets/3D/sphere.obj", &sphereVertices, &numberOfSphereVertices, &sphereIndices, &numberOfSphereIndices);
	glGenVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	GLuint sphereVertexBuffer;
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices[0])*numberOfSphereVertices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sphereVertices[0])*numberOfSphereVertices, sphereVertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	GLuint sphereIndexBuffer;
	glGenBuffers(1, &sphereIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphereIndices[0])*numberOfSphereIndices, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(sphereIndices[0])*numberOfSphereIndices, sphereIndices);
	glBindVertexArray(NULL);

	// Create 3D Camera
	camera3DPosition = vec3(0, 0, -5);
	camera3DRotations = vec3(0, 0, 0);
	//camera3DViewFrustum = CreateViewFrustum(55.64f, 33.07f, 1.0f / 3.0f, 100.0f); // NOTE: These FOV values are for fullscreen
	camera3DViewFrustum = CreateViewFrustum(2*24.866f, 2*24.866f, 1.0f / 3.0f, 100.0f);
	/*mat4 Pp = GetPerspectiveProjection(camera3DViewFrustum);

	vec4 csNearZero = Pp * vec4(0.0f, 0.0f, -camera3DViewFrustum.near, 1.0f); csNearZero /= csNearZero.w;
	vec4 csNearMin = Pp * vec4(camera3DViewFrustum.left, camera3DViewFrustum.bottom, -camera3DViewFrustum.near, 1.0f); csNearMin /= csNearMin.w;
	vec4 csNearMax = Pp * vec4(camera3DViewFrustum.right, camera3DViewFrustum.top, -camera3DViewFrustum.near, 1.0f); csNearMax /= csNearMax.w;

	vec4 csFarZero = Pp * vec4(0.0f, 0.0f, -camera3DViewFrustum.far, 1.0f); csFarZero /= csFarZero.w;
	vec4 csFarMin = Pp * vec4(-camera3DViewFrustum.far*tan(DegreesToRadians(55.64) / 2.0f), -camera3DViewFrustum.far*tan(DegreesToRadians(33.07f) / 2.0f), -camera3DViewFrustum.far, 1.0f); csFarMin /= csFarMin.w;
	vec4 csFarMax = Pp * vec4(camera3DViewFrustum.far*tan(DegreesToRadians(55.64) / 2.0f), camera3DViewFrustum.far*tan(DegreesToRadians(33.07f) / 2.0f), -camera3DViewFrustum.far, 1.0f); csFarMax /= csFarMax.w;

	collisionShape1.origin = vec3(0,0,0);
	collisionShape1.halfDim = vec3(1,1,1);
	collisionShape1.transform = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0, 0,0,1,1);

	collisionShape2.origin = vec3(0,0,0);
	collisionShape2.halfDim = vec3(1,1,1);
	collisionShape2.transform = mat4(1,0,0,0,   0,1,0,0,   0,0,1,0,   -1,-1,1,1);

	OrientedBoundingBoxPoints collisionShape1WorldSpacePoints = GetOBBPoints(collisionShape1);

	mat4 Ccamera = inverse(mat4(1,0,0,0,   0,1,0,0,   0,0,-1,0,   0,0,-5,1));
	OrientedBoundingBoxPoints collisionShape1CameraSpacePoints = Ccamera * collisionShape1WorldSpacePoints;

	ViewFrustum cvf = CreateViewFrustum(55.64f, 33.07f, 1.0f, 100.0f);
	mat4 Pproj = GetPerspectiveProjection(cvf);
	OrientedBoundingBoxPoints collisionShape1ClipSpacePoints = Homogenize(Pproj * collisionShape1CameraSpacePoints);

	int x = 5;*/

	/*vec3 collisionShape1_Rotation = vec3(0, 0, DegreesToRadians(45.0f));
	mat4 Pprojection = GetPerspectiveProjection(camera3DViewFrustum);
	mat4 Ccamera = mat4(cos(collisionShape1_Rotation.z), sin(collisionShape1_Rotation.z), 0, 0, -sin(collisionShape1_Rotation.z), cos(collisionShape1_Rotation.z), 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);
	mat4 Bmodel = collisionShape1.transform;
	mat4 PCM = Pprojection * inverse(Ccamera) * Bmodel;

	OrientedBoundingBoxPoints wsp;
	for (U32 i = 0; i < 8; ++i)
	{
		wsp.points[i] = PCM * vec4(cubeVertices[(i * 3) + 0], cubeVertices[(i * 3) + 1], cubeVertices[(i * 3) + 2], 1.0f);
	}
	OrientedBoundingBoxPoints csp = Homogenize(wsp);*/
}






U32 numEntities = (10 * 9) + 1;
Entity* entities;
Camera  camera;
U32 linkEntityLocation = numEntities - 1;

void InitScene()
{
	camera.position = vec2(35, -12.5f);
	camera.viewArea.x = 10;
	camera.viewArea.y = 9;

	entities = (Entity*)malloc(sizeof(Entity) * numEntities);

	char* tileFilenames[] = {
		// System Bar
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",
		"Assets/x60/sysBar.bmp",

		// row 1
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 2
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/greenFringeUR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 3
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",
		
		// row 4
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenWeeds.bmp",
		"Assets/x60/greenFringeU.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",
		
		// row 5
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 6
		"Assets/x60/treeHighRight.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/greenFringeUL.bmp",
		"Assets/x60/green.bmp",
		"Assets/x60/greenFlowers.bmp",
		"Assets/x60/greenFringeR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeHighLeft.bmp",
		"Assets/x60/treeHighRight.bmp",

		// row 7
		"Assets/x60/treeLowRight.bmp",
		"Assets/x60/yellow.bmp",
		"Assets/x60/greenFringeBL.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeB.bmp",
		"Assets/x60/greenFringeBR.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/weed.bmp",
		"Assets/x60/treeLowLeft.bmp",
		"Assets/x60/treeLowRight.bmp",

		// row 8
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
		"Assets/x60/rails.bmp",
	};

	for (U32 j = 0; j < 9; ++j)
	{
		for (U32 i = 0; i < 10; ++i)
		{
			U32 entityLocation = 10 * j + i;

			vec2 pos(30.5f + i, -8.5f - j);
			char* filename = tileFilenames[entityLocation];

			entities[entityLocation].position = pos;
			entities[entityLocation].scale = vec2(1.0f, 1.0f);

			I32 textureWidth = 0;
			I32 textureHeight = 0;
			I32 textureImageComponents = 0;
			I32 textureNumImageComponentsDesired = 4;
			U8 * textureData = stbi_load(filename, &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &(entities[entityLocation].texture));
			glBindTexture(GL_TEXTURE_2D, entities[entityLocation].texture);
			glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
		}
	}

	
	entities[linkEntityLocation].position = camera.position;
	entities[linkEntityLocation].scale = vec2(0.75f, 1.0f);

	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = stbi_load("Assets/x60/link.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &(entities[linkEntityLocation].texture));
	glBindTexture(GL_TEXTURE_2D, entities[linkEntityLocation].texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

}

GLuint LoadShaderProgram(char* vertexShaderFilename, char* fragmentShaderFilename)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	U64 vertexShaderFileSize = GetFileSize(vertexShaderFilename).fileSize;
	char* vertexShaderSource = (char *)malloc(sizeof(char)* vertexShaderFileSize);
	const GLint glSizeRead = ReadFile(vertexShaderFilename, vertexShaderSource, vertexShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(vertexShader, 1, &vertexShaderSource, &glSizeRead);
	glCompileShader(vertexShader);
	verifyShaderReturnResult vertexVerification = verifyShader(vertexShader);
	if (!vertexVerification.compiled)
	{
		DebugPrint(vertexVerification.infoLog);
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	U64 fragmentShaderFileSize = GetFileSize(fragmentShaderFilename).fileSize;
	char* fragmentShaderSource = (char *)malloc(sizeof(char)* fragmentShaderFileSize);
	const GLint glFragmentShaderSize = ReadFile(fragmentShaderFilename, fragmentShaderSource, fragmentShaderFileSize, 0).numberOfBytesRead;
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, &glFragmentShaderSize);
	glCompileShader(fragmentShader);
	verifyShaderReturnResult fragmentVerification = verifyShader(fragmentShader);
	if (!fragmentVerification.compiled)
	{
		DebugPrint(fragmentVerification.infoLog);
		return false;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	verifyProgramReturnResult programVerification = verifyProgram(shaderProgram);
	if (!programVerification.compiled)
	{
		DebugPrint(programVerification.infoLog);
		return false;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

bool GameInit()
{
	CollisionTestsRectRect2();
	CollisionDetection();

	glClearColor(0.32f, 0.18f, 0.66f, 0.0f);

	glGenVertexArrays(1, &texturedQuadVAO);
	glBindVertexArray(texturedQuadVAO);
	// NOTE: quad for testing, corrected for aspect ratio
	GLfloat vertices[numVertices * vertexDimensionality] =
	{
		-0.50f, -0.50f,
		 0.50f, -0.50f,
		 0.50f,  0.50f,
		-0.50f,  0.50f
	};

	GLfloat textureCoordinates[numVertices * textureSpaceDimensionality] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	GLuint Buffers[1];
	glGenBuffers(1, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(textureCoordinates), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(textureCoordinates), textureCoordinates);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices) /* NOTE: THIS IS A BYTE OFFSET*/);
	glEnableVertexAttribArray(1);

	texturedQuadShaderProgram = LoadShaderProgram("texturedQuad.vert", "texturedQuad.frag");
	solidColorQuadShaderProgram = LoadShaderProgram("solidColorQuad.vert", "solidColorQuad.frag");
	solidColorCircleInPointShaderProgram = LoadShaderProgram("solidColorCircleInPoint.vert", "solidColorCircleInPoint.frag");
	solidColorTriangleShaderProgram = LoadShaderProgram("solidColorTriangle.vert", "solidColorTriangle.frag");
	glUseProgram(texturedQuadShaderProgram);


	stbi_set_flip_vertically_on_load(1);
	// textures
	// read in the texture
	I32 textureWidth = 0;
	I32 textureHeight = 0;
	I32 textureImageComponents = 0;
	I32 textureNumImageComponentsDesired = 4;
	U8 * textureData = stbi_load("Assets/tile1.bmp", &textureWidth, &textureHeight, &textureImageComponents, textureNumImageComponentsDesired); // Channel order?
	
	// send the texture data to OpenGL memory
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, textureWidth, textureHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, textureData);

	glGenSamplers(1, &textureSampler);
	glBindSampler(0, textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(textureSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Set the spriteSampler sampler variable in the shader to fetch data from the 0th texture location
	spriteSamplerLocation = glGetUniformLocation(texturedQuadShaderProgram, "spriteSampler");
	glUniform1i(spriteSamplerLocation, 0);
	PCMLocation = glGetUniformLocation(texturedQuadShaderProgram, "PCM");

	solidColorQuadPCMLocation = glGetUniformLocation(solidColorQuadShaderProgram, "PCM");
	solidColorQuadQuadColorLocation = glGetUniformLocation(solidColorQuadShaderProgram, "quadColor");

	solidColorCircleInPointPCMLocation = glGetUniformLocation(solidColorCircleInPointShaderProgram, "PCM");
	solidColorCircleInPointCircleColorLocation = glGetUniformLocation(solidColorCircleInPointShaderProgram, "circleColor");

	solidColorTrianglePCMLocation = glGetUniformLocation(solidColorTriangleShaderProgram, "PCM");
	solidColorTriangleTriangleColorLocation = glGetUniformLocation(solidColorTriangleShaderProgram, "triangleColor");
	//InitScene();
	//Init2DCollisionTestScene();
	Init3DCollisionTestScene();

	return true;
}


vec4 darkRed = vec4(0.898f, 0.224f, 0.208f, 1.0f);
vec4 lightRed = vec4(0.957f, 0.263f, 0.212f, 1.0f);

vec4 darkGreen = vec4(0.263f, 0.627f, 0.278f, 1.0f);
vec4 lightGreen = vec4(0.298f, 0.686f, 0.314f, 1.0f);

enum csType
{
	cs_Rectangle,
	cs_Triangle,
	cs_Circle,

	cs_COUNT
};
csType cs1Type = cs_Rectangle;
vec2 cs1Pos = vec2(-1,2);
F32 cs1RotationAngle = 0;
vec4 cs1Color;

vec4 cs2Color;

U32 sampleNumber = 0;
bool detailsMode = true;
bool showcs2 = false;
bool smoothMode = true;

enum GJKMode
{
	gjk_Simple,
	gjk_Casey,
	gjk_AllVoronoi,

	gjk_COUNT
};
GJKMode gjkMode;

void collisionScene2DUpdate(F32 deltaTime)
{
// 	if (GetKey(KeyCode_W))
// 	{
// 		entities[linkEntityLocation].position += vec2(0.0f, 2*deltaTime);
// 	}
// 	if (GetKey(KeyCode_S))
// 	{
// 		entities[linkEntityLocation].position += vec2(0.0f, 2 * -deltaTime);
// 	}
// 	if (GetKey(KeyCode_A))
// 	{
// 		entities[linkEntityLocation].position += vec2(2 * -deltaTime, 0.0f);
// 	}
// 	if (GetKey(KeyCode_D))
// 	{
// 		entities[linkEntityLocation].position += vec2(2 * deltaTime, 0.0f);
// 	}
// 
// 	camera.position = entities[linkEntityLocation].position;


	// NOTE: Setting the viewport each frame shouldnt happen
	//glViewport(0, 0, 600, 540);
	glViewport(0, 0, 800, 800);

	// NOTE: Clear the buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);




	// NOTE: Draw grid
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(gridVAO);
	mat3 Ccamera = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, collisionCamera.position.x, collisionCamera.position.y, 1.0f);
	mat3 Oprojection = mat3((2.0f / collisionCamera.viewArea.x), 0.0f, 0.0f, 0.0f, (2.0f / collisionCamera.viewArea.y), 0.0f, 0.0f, 0.0f, 1.0f);
	mat3 PCM = Oprojection * inverse(Ccamera) * mat3(1, 0, 0, 0, 1, 0, 0, 0, 1);
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &vec4(.933, .933, .933, 1)[0]);
	glDrawArrays(GL_LINES, 0, (numGridLines + numGridLines + 2) * pointsPerLine * gridLinePointDimensionality);



	// NOTE: Bind the VAO that holds the vertex information for the current object.
	glBindVertexArray(texturedQuadVAO);

	// NOTE: Bind the shader that will be used to draw it.
	//glUseProgram(texturedQuadShaderProgram);
	//glUseProgram(solidColorQuadShaderProgram);

	// NOTE: Bind the texture that represents the gameobject, also make sure the texture is active.
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);

	// NOTE: Draw this bitch.
	//glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);

// 	for (U32 i = 0; i < numEntities; ++i)
// 	{
// 		glActiveTexture(GL_TEXTURE0);
// 		glBindTexture(GL_TEXTURE_2D, entities[i].texture);
// 		
// 		mat3 Bmodel = mat3(1.0f*entities[i].scale.x, 0.0f, 0.0f, 0.0f, 1.0f*entities[i].scale.y, 0.0f, entities[i].position.x, entities[i].position.y, 1.0f);
// 		mat3 Ccamera = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, camera.position.x, camera.position.y, 1.0f);
// 		mat3 Oprojection = mat3((2.0f / camera.viewArea.x), 0.0f, 0.0f, 0.0f, (2.0f / camera.viewArea.y), 0.0f, 0.0f, 0.0f, 1.0f);
// 		mat3 PCM = Oprojection * inverse(Ccamera) * Bmodel;
// 		glUniformMatrix3fv(PCMLocation, 1, GL_FALSE, &PCM[0][0]);
// 
// 		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
// 	}

	if (GetMouseButtonDown(MouseCode_Right))
	{
		smoothMode = !smoothMode;
	}

	F32 angle = DegreesToRadians(45.0f);
 	if (GetKey(KeyCode_W))
 	{
		if (smoothMode)
		{
			cs1Pos.y += deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_W))
		{
			cs1Pos.y += 0.5f;
		}
 	}
	if (GetKey(KeyCode_S))
 	{
		if (smoothMode)
		{
			cs1Pos.y -= deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_S))
		{
			cs1Pos.y -= 0.5f;
		}
 	}
	if (GetKey(KeyCode_A))
 	{
		if (smoothMode)
		{
			cs1Pos.x -= deltaTime * .5f;
		}
		else if(GetKeyDown(KeyCode_A))
		{
			cs1Pos.x -= 0.5f;
		}
 	}
	if (GetKey(KeyCode_D))
 	{
		if (smoothMode)
		{
			cs1Pos.x += deltaTime * .5f;
		}
		else if (GetKeyDown(KeyCode_D))
		{
			cs1Pos.x += 0.5f;
		}
 	}
	if (GetKey(KeyCode_K))
	{
		if (smoothMode)
		{
			cs1RotationAngle += deltaTime * angle;
		}
		else if (GetKeyDown(KeyCode_K))
		{
			cs1RotationAngle += angle;
		}
	}
	if (GetKey(KeyCode_L))
	{
		if (smoothMode)
		{
			cs1RotationAngle -= deltaTime * angle;
		}
		else if (GetKeyDown(KeyCode_L))
		{
			cs1RotationAngle -= angle;
		}
	}
	if (GetMouseButtonDown(MouseCode_Left))
	{
		cs1Type = (csType)(cs1Type + 1);
		if (cs1Type == cs_COUNT)
		{
			cs1Type = cs_Rectangle;
		}
		DebugPrintf(512, "crRType = %i\n", cs1Type);
	}
	if (GetKeyDown(KeyCode_M))
	{
		detailsMode = !detailsMode;
	}
	if (GetKeyDown(KeyCode_N))
	{
		showcs2 = !showcs2;
	}
	if (GetKeyDown(KeyCode_B))
	{
		gjkMode = (GJKMode)(gjkMode + 1);
		if (gjkMode == cs_COUNT)
		{
			gjkMode = gjk_Simple;
		}
		DebugPrintf(512, "gjkMode = %i\n", gjkMode);
	}

	
	
	
	mat3 Bmodelcs2 = cs2Rectangle.transform;
	mat3 Bmodelcs1 = mat3(1, 0, 0, 0, 1, 0, cs1Pos.x, cs1Pos.y, 1) * mat3(cos(cs1RotationAngle), sin(cs1RotationAngle), 0, -sin(cs1RotationAngle), cos(cs1RotationAngle), 0, 0, 0, 1);
	cs1Rectangle.transform = Bmodelcs1;
	cs1Triangle.transform = Bmodelcs1;
	cs1Circle.origin = cs1Pos;
	bool collision = false;
	if (cs1Type == cs_Rectangle)
	{
		if (gjkMode == gjk_Simple)
		{
			collision = GJK(cs1Rectangle, cs2Rectangle);
		}
		else if (gjkMode == gjk_Casey)
		{
			collision = GJK_Casey(cs1Rectangle, cs2Rectangle);
		}
		else if (gjkMode == gjk_AllVoronoi)
		{
			collision = GJK_AllVoronoi(cs1Rectangle, cs2Rectangle);
		}
	}
	if (cs1Type == cs_Triangle)
	{
		collision = GJK(cs1Triangle, cs2Rectangle);
	}
	if (cs1Type == cs_Circle)
	{
		collision = GJK(cs1Circle, cs2Rectangle);
	}

	if (collision)
	{
		cs2Color = lightGreen;
		cs1Color = darkGreen;
	}
	else
	{
		cs2Color = lightRed;
		cs1Color = darkRed;
	}


	// cs2
	glUseProgram(solidColorQuadShaderProgram);
	glBindVertexArray(texturedQuadVAO);
	PCM = Oprojection * inverse(Ccamera) * Bmodelcs2;
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &cs2Color[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	
	// cs1
	glUseProgram(solidColorQuadShaderProgram);
	PCM = Oprojection * inverse(Ccamera) * Bmodelcs1;
	glUniformMatrix3fv(solidColorQuadPCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorQuadQuadColorLocation, 1, &cs1Color[0]);
	if (cs1Type == cs_Rectangle)
	{
		// When cs1 is a rectangle
		glBindVertexArray(texturedQuadVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);
	}
	if (cs1Type == cs_Triangle)
	{
		// When cs1 is a triangle
		glBindVertexArray(equalateralTriangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, numPointsInTriangle*equalateralTrianglePointDimensionality);
	}
	if (cs1Type == cs_Circle)
	{
		glUseProgram(solidColorCircleInPointShaderProgram);
		glBindVertexArray(circleVAO);
		glUniformMatrix3fv(solidColorCircleInPointPCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glUniform4fv(solidColorCircleInPointCircleColorLocation, 1, &cs1Color[0]);
		F32 unitCircleSize = 800.0f / ((F32)numGridLines/2.0f);
		glPointSize(cs1Circle.radius * unitCircleSize);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	// Print out collision information
	if (GetKeyDown(KeyCode_Spacebar))
	{
		DebugPrintf(512, "\n\n\nCOLLISION SAMPLE %u\n\n", sampleNumber++);
		(collision) ? DebugPrint("collision = TRUE\n\n") : DebugPrint("collision = FALSE\n\n");

		if (cs1Type == cs_Rectangle)
		{
			DebugPrint("COLLISION SHAPE 1(RECTANGLE)\n");

			if (detailsMode)
			{
				DebugPrintf(1024, "origin = (%f, %f)\n", cs1Rectangle.origin.x, cs1Rectangle.origin.y);
				DebugPrintf(1024, "halfDim = (%f, %f)\n\n", cs1Rectangle.halfDim.x, cs1Rectangle.halfDim.y);
				mat3 t = cs1Rectangle.transform;
				DebugPrintf(2048, "transform = %f %f %f\n            %f %f %f\n            %f %f %f\n\n", t[0][0], t[1][0], t[2][0], t[0][1], t[1][1], t[2][1], t[0][2], t[1][2], t[2][2]);
			}

			vec3 cs1p0 = cs1Rectangle.transform * vec3(cs1Rectangle.origin.x - cs1Rectangle.halfDim.x, cs1Rectangle.origin.y - cs1Rectangle.halfDim.y, 1.0f);
			vec3 cs1p1 = cs1Rectangle.transform * vec3(cs1Rectangle.origin.x - cs1Rectangle.halfDim.x, cs1Rectangle.origin.y + cs1Rectangle.halfDim.y, 1.0f);
			vec3 cs1p2 = cs1Rectangle.transform * vec3(cs1Rectangle.origin.x + cs1Rectangle.halfDim.x, cs1Rectangle.origin.y - cs1Rectangle.halfDim.y, 1.0f);
			vec3 cs1p3 = cs1Rectangle.transform * vec3(cs1Rectangle.origin.x + cs1Rectangle.halfDim.x, cs1Rectangle.origin.y + cs1Rectangle.halfDim.y, 1.0f);
			DebugPrintf(1024, "Transformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\np[3] = (%f,%f)\n\n", cs1p0.x, cs1p0.y, cs1p2.x, cs1p2.y, cs1p3.x, cs1p3.y, cs1p1.x, cs1p1.y);
		}
		else if (cs1Type == cs_Triangle)
		{
			DebugPrint("COLLISION SHAPE 1(TRIANGLE)\n");

			if (detailsMode)
			{
				DebugPrintf(1024, "origin = (%f, %f)\n\n", cs1Triangle.origin.x, cs1Triangle.origin.y);
				mat3 t = cs1Triangle.transform;
				DebugPrintf(2048, "transform = %f %f %f\n            %f %f %f\n            %f %f %f\n", t[0][0], t[1][0], t[2][0], t[0][1], t[1][1], t[2][1], t[0][2], t[1][2], t[2][2]);
				vec2 p0 = cs1Triangle.points[0];
				vec2 p1 = cs1Triangle.points[1];
				vec2 p2 = cs1Triangle.points[2];
				DebugPrintf(2048, "Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\n\n", p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
			}

			vec3 cs1p0 = cs1Triangle.transform * vec3(cs1Triangle.origin.x + cs1Triangle.points[0].x, cs1Triangle.origin.y + cs1Triangle.points[0].y, 1.0f);
			vec3 cs1p1 = cs1Triangle.transform * vec3(cs1Triangle.origin.x + cs1Triangle.points[1].x, cs1Triangle.origin.y + cs1Triangle.points[1].y, 1.0f);
			vec3 cs1p2 = cs1Triangle.transform * vec3(cs1Triangle.origin.x + cs1Triangle.points[2].x, cs1Triangle.origin.y + cs1Triangle.points[2].y, 1.0f);
			DebugPrintf(1024, "Transformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\n\n", cs1p0.x, cs1p0.y, cs1p1.x, cs1p1.y, cs1p2.x, cs1p2.y);
		}
		else if (cs1Type == cs_Circle)
		{
			DebugPrint("COLLISION SHAPE 1(CIRCLE)\n");
			DebugPrintf(1024, "origin = (%f, %f)\n", cs1Circle.origin.x, cs1Circle.origin.y);
			DebugPrintf(512, "radius = %f\n\n", cs1Circle.radius);
		}
		
		if (showcs2)
		{
			vec3 cs2p0 = cs2Rectangle.transform * vec3(cs2Rectangle.origin.x - cs2Rectangle.halfDim.x, cs2Rectangle.origin.y - cs2Rectangle.halfDim.y, 1.0f);
			vec3 cs2p1 = cs2Rectangle.transform * vec3(cs2Rectangle.origin.x - cs2Rectangle.halfDim.x, cs2Rectangle.origin.y + cs2Rectangle.halfDim.y, 1.0f);
			vec3 cs2p2 = cs2Rectangle.transform * vec3(cs2Rectangle.origin.x + cs2Rectangle.halfDim.x, cs2Rectangle.origin.y - cs2Rectangle.halfDim.y, 1.0f);
			vec3 cs2p3 = cs2Rectangle.transform * vec3(cs2Rectangle.origin.x + cs2Rectangle.halfDim.x, cs2Rectangle.origin.y + cs2Rectangle.halfDim.y, 1.0f);
			DebugPrintf(1024, "COLLISION SHAPE 2(RECTANGLE)\nTransformed Points\np[0] = (%f,%f)\np[1] = (%f,%f)\np[2] = (%f,%f)\np[3] = (%f,%f)\n", cs2p0.x, cs2p0.y, cs2p2.x, cs2p2.y, cs2p3.x, cs2p3.y, cs2p1.x, cs2p1.y);
		}
	}
}


F32 movementSpeed = 2.0f;
F32 rotationSpeed = 45.0f; // In degrees

bool controllingShape1 = true;

enum csType3D
{
	cs3D_OrientedBoundingBox,
	cs3D_Sphere,
	cs3D_Capsule,

	cs3D_COUNT
};

struct CollisionShape_Data
{
	vec4 color;
	vec3 position;
	vec3 rotation;
	csType3D type;
};

CollisionShape_Data* controlledCollisionShape = NULL;
CollisionShape_Data collisionShape1_Data;
CollisionShape_Data collisionShape2_Data;

F32 cameraMovementSpeed = 2.0f;
F32 cameraRotationSpeed = 30.0f;

F32 cameraDistance = 5.0f;
F32 cameraAzimuthAngle = 0.0f;
F32 cameraPolarAngle = 0.0f;
F32 cameraHeight = 0.0f;

void collisionScene3DUpdate(F32 deltaTime)
{
	if (GetKeyDown(KeyCode_1))
	{
		controllingShape1 = !controllingShape1;
	}
	if (controllingShape1)
	{
		controlledCollisionShape = &collisionShape1_Data;
	}
	else
	{
		controlledCollisionShape = &collisionShape2_Data;
	}

	if (GetKey(KeyCode_W))
	{
		controlledCollisionShape->position.z += deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_S))
	{
		controlledCollisionShape->position.z -= deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_A))
	{
		controlledCollisionShape->position.x -= deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_D))
	{
		controlledCollisionShape->position.x += deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_R))
	{
		controlledCollisionShape->position.y += deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_F))
	{
		controlledCollisionShape->position.y -= deltaTime * movementSpeed;
	}
	if (GetKey(KeyCode_I))
	{
		controlledCollisionShape->rotation.x += deltaTime * rotationSpeed;
	}
	if (GetKey(KeyCode_K))
	{
		controlledCollisionShape->rotation.x -= deltaTime * rotationSpeed;
	}
	if (GetKey(KeyCode_J))
	{
		controlledCollisionShape->rotation.y -= deltaTime * rotationSpeed;
	}
	if (GetKey(KeyCode_L))
	{
		controlledCollisionShape->rotation.y += deltaTime * rotationSpeed;
	}
	if (GetKey(KeyCode_U))
	{
		controlledCollisionShape->rotation.z += deltaTime * rotationSpeed;
	}
	if (GetKey(KeyCode_O))
	{
		controlledCollisionShape->rotation.z -= deltaTime * rotationSpeed;
	}
	if (GetKeyDown(KeyCode_T))
	{
		controlledCollisionShape->type = (csType3D)(controlledCollisionShape->type + 1);
		if (controlledCollisionShape->type == cs3D_COUNT)
		{
			controlledCollisionShape->type = cs3D_OrientedBoundingBox;
		}
		DebugPrintf(512, "cs3d %i\n", controlledCollisionShape->type);
	}

	if (GetKey(KeyCode_Up))
	{
		cameraPolarAngle += deltaTime * cameraRotationSpeed;
		cameraHeight += deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Down))
	{
		cameraPolarAngle -= deltaTime * cameraRotationSpeed;
		cameraHeight -= deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Left))
	{
		cameraAzimuthAngle -= deltaTime * cameraRotationSpeed;
	}
	if (GetKey(KeyCode_Right))
	{
		cameraAzimuthAngle += deltaTime * cameraRotationSpeed;
	}
	if (GetKey(KeyCode_Minus))
	{
		cameraDistance -= deltaTime * cameraMovementSpeed;
	}
	if (GetKey(KeyCode_Equal))
	{
		cameraDistance += deltaTime * cameraMovementSpeed;
	}

// 	if (abs(cameraPolarAngle) > 80.0f)
// 	{
// 		cameraPolarAngle = 80.0f;
// 	}

	glViewport(0, 0, 800, 800);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	mat4 Pprojection = GetPerspectiveProjection(camera3DViewFrustum);
	
	// Cylindrical Coordinates
	camera3DPosition.x = cameraDistance * sin(DegreesToRadians(cameraAzimuthAngle));
	camera3DPosition.y = cameraHeight;
	camera3DPosition.z = cameraDistance * -cos(DegreesToRadians(cameraAzimuthAngle));

	// Camera Basis
	vec3 cb_z = normalize(-camera3DPosition);
	vec3 cb_x = normalize(cross(cb_z, vec3(0,1,0)));
	vec3 cb_y = normalize(cross(cb_x, cb_z));

	//mat4 Ccamera = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);
	mat4 cBasis = mat4(cb_x.x, cb_x.y, cb_x.z, 0, cb_y.x, cb_y.y, cb_y.z, 0, cb_z.x, cb_z.y, cb_z.z, 0, 0, 0, 0, 1);
	mat4 Ccamera = TranslationMatrix(camera3DPosition) * cBasis * ScaleMatrix(vec3(1, 1, -1));


	mat4 t1 = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);;
	mat4 t2 = TranslationMatrix(camera3DPosition);
	bool eq1 = t1 == t2;

	mat4 s1 = ScaleMatrix(vec3(1,1,-1));

	mat4 f1 = s1 * t1;
	mat4 f2 = t1 * s1;
	mat4 wanted = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, camera3DPosition.x, camera3DPosition.y, camera3DPosition.z, 1);

	bool eq2 = f1 == wanted;
	bool eq3 = f2 == wanted;

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	glUseProgram(solidColorTriangleShaderProgram);


	// COLLISION SHAPE 1
	mat4 Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x); //collisionShape1.transform;
	mat4 PCM = Pprojection * inverse(Ccamera) * Bmodel;
	glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorTriangleTriangleColorLocation, 1, &controlledCollisionShape->color[0]);
	
	if (collisionShape1_Data.type == cs3D_OrientedBoundingBox)
	{
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, numberOfCubeIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape1_Data.type == cs3D_Sphere)
	{
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape1_Data.type == cs3D_Capsule)
	{
		// Capsule Wall
		glBindVertexArray(capsuleVAO);
		glDrawElements(GL_TRIANGLES, numberOfCapsuleWallIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 1
		glBindVertexArray(sphereVAO);
		Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);
		PCM = Pprojection * inverse(Ccamera) * Bmodel;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 2
		glBindVertexArray(sphereVAO);
		Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape1_Data.position.x, collisionShape1_Data.position.y, collisionShape1_Data.position.z, 1) * RotationMatrix_Z(collisionShape1_Data.rotation.z) * RotationMatrix_Y(collisionShape1_Data.rotation.y) * RotationMatrix_X(collisionShape1_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1);
		PCM = Pprojection * inverse(Ccamera) * Bmodel;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(NULL);



	// COLLISION SHAPE 2
	Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x); //collisionShape1.transform;
	PCM = Pprojection * inverse(Ccamera) * Bmodel;
	glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
	glUniform4fv(solidColorTriangleTriangleColorLocation, 1, &controlledCollisionShape->color[0]);

	if (collisionShape2_Data.type == cs3D_OrientedBoundingBox)
	{
		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, numberOfCubeIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape2_Data.type == cs3D_Sphere)
	{
		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	else if (collisionShape2_Data.type == cs3D_Capsule)
	{
		// Capsule Wall
		glBindVertexArray(capsuleVAO);
		glDrawElements(GL_TRIANGLES, numberOfCapsuleWallIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 1
		glBindVertexArray(sphereVAO);
		Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1);
		PCM = Pprojection * inverse(Ccamera) * Bmodel;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);

		// Capsule Sphere 2
		glBindVertexArray(sphereVAO);
		Bmodel = mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, collisionShape2_Data.position.x, collisionShape2_Data.position.y, collisionShape2_Data.position.z, 1) * RotationMatrix_Z(collisionShape2_Data.rotation.z) * RotationMatrix_Y(collisionShape2_Data.rotation.y) * RotationMatrix_X(collisionShape2_Data.rotation.x) * mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1);
		PCM = Pprojection * inverse(Ccamera) * Bmodel;
		glUniformMatrix4fv(solidColorTrianglePCMLocation, 1, GL_FALSE, &PCM[0][0]);
		glDrawElements(GL_TRIANGLES, numberOfSphereIndices, GL_UNSIGNED_INT, NULL);
	}
	glBindVertexArray(NULL);
}

void GameUpdate(F32 deltaTime)
{
	//collisionScene2DUpdate(deltaTime);
	collisionScene3DUpdate(deltaTime);
}

bool GameShutdown()
{
	glDeleteProgram(texturedQuadShaderProgram);
	return true;
}





verifyShaderReturnResult verifyShader(GLuint shader)
{
	verifyShaderReturnResult result = { true, NULL, 0 };

	GLint shaderCompileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderCompileStatus);
	if (shaderCompileStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar *infoLog = (GLchar *)malloc(sizeof(GLchar)* infoLogLength);
		glGetShaderInfoLog(shader, infoLogLength, &returnedInfoLogLength, infoLog);

		result.compiled = false;
		result.infoLog = infoLog;
		result.infoLogLength = returnedInfoLogLength;
	}

	return result;
}

verifyProgramReturnResult verifyProgram(GLuint program)
{
	verifyProgramReturnResult result = { true, NULL, 0 };

	GLint programLinkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &programLinkStatus);
	if (programLinkStatus != GL_TRUE)
	{
		GLint infoLogLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLsizei returnedInfoLogLength = 0;
		GLchar *infoLog = (GLchar *)malloc(sizeof(GLchar)* infoLogLength);
		glGetShaderInfoLog(program, infoLogLength, &returnedInfoLogLength, infoLog);

		result.compiled = false;
		result.infoLog = infoLog;
		result.infoLogLength = returnedInfoLogLength;
	}

	return result;
}
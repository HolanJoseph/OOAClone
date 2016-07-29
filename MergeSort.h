#pragma once

template <typename T>
void CopyArray(T* copyFrom, T* copyTo, size_t startIndex, size_t endIndex)
{
	for (size_t i = startIndex; i < endIndex; ++i)
	{
		copyTo[i] = copyFrom[i];
	}
}

template <typename T>
void Merge(T* toMerge, size_t A_StartIndex, size_t B_StartIndex, size_t A_Count, size_t B_Count, T* workspace, bool(*ComparisonFunction)(T, T))
{
	size_t i = 0;
	size_t j = 0;
	size_t k = 0;
	while (i < A_Count || j < B_Count)
	{
		if (i < A_Count && (j >= B_Count || ComparisonFunction(toMerge[A_StartIndex + i], toMerge[B_StartIndex + j])))
		{
			workspace[A_StartIndex + k] = toMerge[A_StartIndex + i];
			++k;
			++i;
		}
		else
		{
			workspace[A_StartIndex + k] = toMerge[B_StartIndex + j];
			++k;
			++j;
		}
	}
}

template <typename T>
void SplitThenMerge(T* toSort, size_t startIndex, size_t count, T* workspace, bool(*ComparisonFunction)(T, T))
{
	if (count <= 1)
	{
		return;
	}

	size_t firstHalfCount = count / 2;
	size_t secondHalfCount = count - firstHalfCount;
	size_t middleIndex = startIndex + firstHalfCount;
	SplitThenMerge(toSort, startIndex, firstHalfCount, workspace, ComparisonFunction);
	SplitThenMerge(toSort, middleIndex, secondHalfCount, workspace, ComparisonFunction);
	Merge(toSort, startIndex, middleIndex, firstHalfCount, secondHalfCount, workspace, ComparisonFunction);
	CopyArray(workspace, toSort, startIndex, startIndex + count);
}

template <typename T>
void MergeSort(T* toSort, size_t count, bool (*ComparisonFunction)(T, T))
{
	T* workspace = new T[count];
	SplitThenMerge(toSort, 0, count, workspace, ComparisonFunction);
	delete workspace;
}
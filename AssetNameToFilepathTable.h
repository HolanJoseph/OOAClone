#pragma once



#include "Types.h"
#include "Util.h"

#include "_SystemAPI.h"



struct StringStringHashTable
{
	struct SSHashLink
	{
		const char* k;
		const char* v;

		SSHashLink* next;
	};

	U32 numberOfIndices;
	SSHashLink** table;

	StringStringHashTable() : numberOfIndices(0), table(NULL) {};

	void Initialize(U32 numberOfIndices)
	{
		this->numberOfIndices = numberOfIndices;
		this->table = (SSHashLink**)malloc(sizeof(SSHashLink*) * numberOfIndices);

		for (size_t i = 0; i < numberOfIndices; ++i)
		{
			this->table[i] = NULL;
		}
	}

	const char* GetValue(const char* key)
	{
		const char* result = NULL;

		// NOTE: How is this better?
		U32 i = (String_HashFunction(key, numberOfIndices) & 0x7fffffff) % numberOfIndices;
		SSHashLink* l = this->table[i];

		while (l)
		{
			// if key and l->key match up to key length
			const char* k = key;
			const char* lk = l->k;
			bool match = true;
			while (*k && *lk)
			{
				if (*k != *lk)
				{
					match = false;
					break;
				}

				++k;
				++lk;
			}

			// Make sure lk is not longer than k
			if (match && (*k != NULL || *lk != NULL))
			{
				match = false;
			}

			if (match)
			{
				result = l->v;
				break;
			}

			l = l->next;
		}

		return result;
	}

	void AddKVPair(const char* key, const char* value)
	{
		const char* keyCopy = Copy(key);
		const char* valueCopy = Copy(value);

		if (GetValue(key) == NULL)
		{
			SSHashLink* newLink = (SSHashLink*)malloc(sizeof(SSHashLink));
			newLink->k = keyCopy;
			newLink->v = valueCopy;
			newLink->next = NULL;
			
			U32 i = (String_HashFunction(key, numberOfIndices) & 0x7fffffff) % numberOfIndices;
			SSHashLink* l =this->table[i];
			// If there are no links at that position in the table, make the new link the first.
			if (l == NULL)
			{
				this->table[i] = newLink;
			}
			// Else traverse the list of links and add the new link to the end of the list.
			else
			{
				while (l->next)
				{
					l = l->next;
				}
				l->next = newLink;
			}
		}
	}

	void Destroy()
	{
		for (size_t i = 0; i < numberOfIndices; ++i)
		{
			SSHashLink* link = this->table[i];
			while (link != NULL)
			{
				free((void*)link->k);
				free((void*)link->v);
				SSHashLink* t = link->next;
				free(link);
				link = t;
			}
		}
		free(this->table);

		this->table = NULL;
		this->numberOfIndices = 0;
	}
};
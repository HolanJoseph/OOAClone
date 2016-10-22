#pragma once

#include "Types.h"
#include "Math.h"
#include "_SystemAPI.h"

/*
* Events
*/
enum ArgumentType
{
	AT_NULL,

	AT_Pointer,
	AT_I32,
	AT_U32,
	AT_F32,
	AT_Bool,
	AT_Vec2,
	AT_SystemTime,

	AT_COUNT
};

struct EventArgument
{
	ArgumentType type;
	union
	{
		void* asPointer;
		I32 asI32;
		U32 asU32;
		F32 asF32;
		bool asBool;
		F32 asVec2[2];
		SystemTime asSystemTime;
	};

	EventArgument()
	{
		this->type = AT_NULL;
	}

	EventArgument(void* pointer)
	{
		this->type = AT_Pointer;
		this->asPointer = pointer;
	}

	EventArgument(I32 i32)
	{
		this->type = AT_I32;
		this->asI32 = i32;
	}

	EventArgument(U32 u32)
	{
		this->type = AT_U32;
		this->asU32 = u32;
	}

	EventArgument(F32 f32)
	{
		this->type = AT_F32;
		this->asF32 = f32;
	}

	EventArgument(bool b)
	{
		this->type = AT_Bool;
		this->asBool = b;
	}

	EventArgument(vec2 vec)
	{
		this->type = AT_Vec2;
		this->asVec2[0] = vec.x;
		this->asVec2[1] = vec.y;
	}

	EventArgument(SystemTime time)
	{
		this->type = AT_SystemTime;
		this->asSystemTime = time;
	}

	ArgumentType GetType()
	{
		ArgumentType result;
		result = this->type;
		return result;
	}

	void* AsPointer()
	{
		void* result;
		result = this->asPointer;
		return result;
	}

	I32 AsI32()
	{
		I32 result;
		result = this->asI32;
		return result;
	}

	U32 AsU32()
	{
		U32 result;
		result = this->asU32;
		return result;
	}

	F32 AsF32()
	{
		F32 result;
		result = this->asF32;
		return result;
	}

	bool AsBool()
	{
		bool result;
		result = this->asBool;
		return result;
	}

	vec2 AsVec2()
	{
		vec2 result;
		result.x = this->asVec2[0];
		result.y = this->asVec2[1];
		return result;
	}

	SystemTime AsSystemTime()
	{
		SystemTime result;
		result = this->asSystemTime;
		return result;
	}
};

enum EventType
{
	ET_NULL,

	ET_OnCollisionEnter,
	ET_OnCollision,
	ET_OnCollisionExit,

	ET_Transition,

	ET_Freeze,
	ET_Unfreeze,

	ET_DummyEvent,

	ET_COUNT
};

struct Event
{
	EventType type;
	static const size_t MAXARGUMENTCOUNT = 12;
	EventArgument arguments[MAXARGUMENTCOUNT];

	Event()
	{
		this->type = ET_NULL;
	}

	Event(Event* e)
	{
		EventType et = e->GetType();
		this->SetType(et);
		for (size_t i = 0; i < this->MAXARGUMENTCOUNT; ++i)
		{
			this->arguments[i] = e->arguments[i];
		}
	}

	EventType GetType()
	{
		EventType result;
		result = this->type;
		return result;
	}

	void SetType(EventType type)
	{
		this->type = type;
	}
};
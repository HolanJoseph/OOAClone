#pragma once
#include "StringAPI.h"

inline void StringTestsAll()
{
	size_t c1 = Length("weed");
	char* w = Copy("Assets/x60/Objects/weed.bmp");
	size_t c2 = Length(w);

	bool b1 = Compare("weed", "weed");
	bool b2 = Compare("weed", "weeds");
	bool b3 = Compare("Assets/x60/Objects/weed.bmp", w);
	bool b4 = Compare(w, w);

	char* cp1 = Copy("weed");
	char* cp2 = Copy("weed", 5);
	char* cp3 = Copy(w);
	char* cp4 = Copy(w, c2);

	free(cp1);
	free(cp2);
	free(cp3);
	free(cp4);



	char* cc1 = Concat("weed", "weed");
	char* cc2 = Concat(w, "weed");

	free(cc1);
	free(cc2);



	char* i1 = Insert("weed", 3, "s");
	char* i2 = Insert(w, 22, "s");
	char* i3 = Insert(w, 22, "/1");
	char* i4 = Insert(i2, 23, "/1");

	free(i1);
	free(i2);
	free(i3);
	free(i4);



	char* e1 = Erase(w, 9, 8);
	char* e2 = Erase(w, 9, 6453654);

	free(e1);
	free(e2);



	I32 ti1 = ToI32("weed");
	I32 ti2 = ToI32("1");
	I32 ti3 = ToI32("-1");
	I32 ti4 = ToI32("1569");
	I32 ti5 = ToI32("-1569");
	I32 ti6 = ToI32("15x");

	U32 tu1 = ToU32("weed");
	U32 tu2 = ToU32("1");
	U32 tu3 = ToU32("-1");
	U32 tu4 = ToU32("1569");
	U32 tu5 = ToU32("-1569");
	U32 tu6 = ToU32("15x");

	bool tf1 = ToF32("weed") == 0.0f;
	bool tf2 = ToF32("1") == 1.0f;
	bool tf3 = ToF32("-1") == -1.0f;
	bool tf4 = ToF32("1569") == 1569.0f;
	bool tf5 = ToF32("-1569") == -1569.0f;
	bool tf6 = ToF32("15x") == 15.0f;
	bool tf7 = ToF32(".1") == .1f;
	bool tf8 = ToF32("1.1") == 1.1f;
	bool tf9 = ToF32("-1.1") == -1.1f;
	bool tf10 = ToF32("1.") == 1.0f;
	bool tf11 = ToF32("-1.") == -1.0f;
	bool tf12 = ToF32("-.1") == -.1f;
	bool tf13 = ToF32("-0.12") == -.12f;
	bool tf14 = ToF32("0.14") == .14f;
	F32 stof1 = StringToF32("-0.12");
	bool tf15 = ToF32("-0.12") == stof1;

	char* s1 = ToString(1);
	char* s2 = ToString(-57);
	char* s3 = ToString(346543);
	I32 min = I32_MIN;
	I32 max = I32_MAX;
	char* s4 = ToString(min);
	char* s5 = ToString(max);
	char* s6 = ToString(0);

	free(s1);
	free(s2);
	free(s3);
	free(s4);
	free(s5);
	free(s6);



	U32 umax = U32_MAX;
	char* u1 = ToString(U32_MAX);
	char* u2 = ToString(0u);
	char* u3 = ToString(576u);

	free(u1);
	free(u2);
	free(u3);



	char* f1 = ToString(123.456, 3);
	char* f2 = ToString(123.456, 2);
	char* f3 = ToString(123.456, 1);
	char* f4 = ToString(123.456, 10);
	char* f5 = ToString(233.007, 4);
	char* f6 = ToString(233.007, 6);
	char* f7 = ToString(233.007, 7);
	char* f8 = ToString(233.007, 10);

	free(f1);
	free(f2);
	free(f3);
	free(f4);
	free(f5);
	free(f6);
	free(f7);
	free(f8);
}
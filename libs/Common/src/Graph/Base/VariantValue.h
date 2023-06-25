/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <ctools/cTools.h>
#include <Common/Globals.h>
#include <string>

class COMMON_API VariantValue
{
private:
	int type = 0; // 0:float,2:bool,3:int;

private:
	ct::fvec4 float_value;
	float float_step = 0.1f;
	float float_step_fast = 1.0f;
	int countDecimals = 2;
	char countDecimalsChar[10] = "%.2f";

	ct::bvec4 bool_value;
	
	ct::ivec4 int_value;
	int int_step = 1;
	int int_step_fast = 10;
	
	int countChannels = 1;
	
public:
	VariantValue()
	= default;

	void Clear()
	{
		float_value = 0.0f;
		bool_value = false;
		int_value = 0;
		countChannels = 1;
		SetCountDecimals(2);
	}

	// constructor 

	explicit VariantValue(float vValue)		{ type = 0; countChannels = 1; float_value.Set(vValue,		0.0f,		0.0f,		0.0f);		}
	explicit VariantValue(ct::fvec2 vValue)	{ type = 0; countChannels = 2; float_value.Set(vValue.x,	vValue.y,	0.0f,		0.0f);		}
	explicit VariantValue(ct::fvec3 vValue)	{ type = 0; countChannels = 3; float_value.Set(vValue.x,	vValue.y,	vValue.z,	0.0f);		}
	explicit VariantValue(ct::fvec4 vValue)	{ type = 0; countChannels = 4; float_value.Set(vValue.x,	vValue.y,	vValue.z,	vValue.w);	}

	explicit VariantValue(bool vValue)		{ type = 1; countChannels = 1; bool_value.Set(vValue,		false,		false,		false);		}
	explicit VariantValue(ct::bvec2 vValue)	{ type = 1; countChannels = 2; bool_value.Set(vValue.x,		vValue.y,	false,		false);		}
	explicit VariantValue(ct::bvec3 vValue)	{ type = 1; countChannels = 3; bool_value.Set(vValue.x,		vValue.y,	vValue.z,	false);		}
	explicit VariantValue(ct::bvec4 vValue)	{ type = 1; countChannels = 4; bool_value.Set(vValue.x,		vValue.y,	vValue.z,	vValue.w);	}

	explicit VariantValue(int vValue)		{ type = 2; countChannels = 1; int_value.Set(vValue,		0,			0,			0);			}
	explicit VariantValue(ct::ivec2 vValue)	{ type = 2; countChannels = 2; int_value.Set(vValue.x,		vValue.y,	0,			0);			}
	explicit VariantValue(ct::ivec3 vValue)	{ type = 2; countChannels = 3; int_value.Set(vValue.x,		vValue.y,	vValue.z,	0);			}
	explicit VariantValue(ct::ivec4 vValue)	{ type = 2; countChannels = 4; int_value.Set(vValue.x,		vValue.y,	vValue.z,	vValue.w);	}

	// set

	void Set(float vValue)			{ type = 0; countChannels = 1; float_value.Set(vValue,		0.0f,		0.0f,		0.0f);		}
	void Set(ct::fvec2 vValue)		{ type = 0; countChannels = 2; float_value.Set(vValue.x,	vValue.y,	0.0f,		0.0f);		}
	void Set(ct::fvec3 vValue)		{ type = 0; countChannels = 3; float_value.Set(vValue.x,	vValue.y,	vValue.z,	0.0f);		}
	void Set(ct::fvec4 vValue)		{ type = 0; countChannels = 4; float_value.Set(vValue.x,	vValue.y,	vValue.z,	vValue.w);	}
	void SetStepValue(float vStepValue) { float_step = vStepValue; }
	void SetStepFastValue(float vStepValue) { float_step_fast = vStepValue; }
	void SetCountDecimalsChar(int vCountDecimals)
	{
		snprintf(countDecimalsChar, 9, "%%.%if", vCountDecimals);
	}
	void SetCountDecimals(int vCountDecimals)
	{ 
		countDecimals = vCountDecimals; 
		SetCountDecimalsChar(vCountDecimals);
	}

	void Set(bool vValue)			{ type = 1; countChannels = 1; bool_value.Set(vValue,		false,		false,		false);		}
	void Set(ct::bvec2 vValue)		{ type = 1; countChannels = 2; bool_value.Set(vValue.x,		vValue.y,	false,		false);		}
	void Set(ct::bvec3 vValue)		{ type = 1; countChannels = 3; bool_value.Set(vValue.x,		vValue.y,	vValue.z,	false);		}
	void Set(ct::bvec4 vValue)		{ type = 1; countChannels = 4; bool_value.Set(vValue.x,		vValue.y,	vValue.z,	vValue.w);	}

	void Set(int vValue)			{ type = 2; countChannels = 1; int_value.Set(vValue,		0,			0,			0);			}
	void Set(ct::ivec2 vValue)		{ type = 2; countChannels = 2; int_value.Set(vValue.x,		vValue.y,	0,			0);			}
	void Set(ct::ivec3 vValue)		{ type = 2; countChannels = 3; int_value.Set(vValue.x,		vValue.y,	vValue.z,	0);			}
	void Set(ct::ivec4 vValue)		{ type = 2; countChannels = 4; int_value.Set(vValue.x,		vValue.y,	vValue.z,	vValue.w);	}
	void SetStepValue(int vStepValue) { int_step = vStepValue; }
	void SetStepFastValue(int vStepValue) { int_step_fast = vStepValue; }
	
	// Get

	float* GetFloatPtr() { return &float_value.x; }
	float GetFloatStepValue() const { return float_step; }
	float GetFloatStepFastValue() const { return float_step_fast; }
	int GetCountDecimals() const { return countDecimals; }
	const char* GetCountDecimalsChar() const 
	{
		return countDecimalsChar;
	}

	bool* GetBoolPtr() { return &bool_value.x; }
	
	int* GetIntPtr() { return &int_value.x; }
	int GetIntStepValue() const { return int_step; }
	int GetIntStepFastValue() const { return int_step_fast; }

	int GetCountChannels() const { return countChannels; }

	// operator

	bool operator == (VariantValue v) const
	{
		if (type == v.type && 
			countChannels == v.countChannels)
		{
			if (type == 0 && countChannels == 1)
				return
				float_value.x == v.float_value.x;
			if (type == 0 && countChannels == 2)
				return
				float_value.x == v.float_value.x &&
				float_value.y == v.float_value.y;
			if (type == 0 && countChannels == 3)
				return
				float_value.x == v.float_value.x &&
				float_value.y == v.float_value.y &&
				float_value.z == v.float_value.z;
			if (type == 0 && countChannels == 4) 
				return 
				float_value.x == v.float_value.x && 
				float_value.y == v.float_value.y && 
				float_value.z == v.float_value.z && 
				float_value.w == v.float_value.w;

			if (type == 1 && countChannels == 1)
				return
				bool_value.x == v.bool_value.x;
			if (type == 1 && countChannels == 2)
				return
				bool_value.x == v.bool_value.x &&
				bool_value.y == v.bool_value.y;
			if (type == 1 && countChannels == 3)
				return
				bool_value.x == v.bool_value.x &&
				bool_value.y == v.bool_value.y &&
				bool_value.z == v.bool_value.z;
			if (type == 1 && countChannels == 4)
				return
				bool_value.x == v.bool_value.x &&
				bool_value.y == v.bool_value.y &&
				bool_value.z == v.bool_value.z &&
				bool_value.w == v.bool_value.w;

			if (type == 2 && countChannels == 1)
				return
				int_value.x == v.int_value.x;
			if (type == 2 && countChannels == 2)
				return
				int_value.x == v.int_value.x &&
				int_value.y == v.int_value.y;
			if (type == 2 && countChannels == 3)
				return
				int_value.x == v.int_value.x &&
				int_value.y == v.int_value.y &&
				int_value.z == v.int_value.z;
			if (type == 2 && countChannels == 4)
				return
				int_value.x == v.int_value.x &&
				int_value.y == v.int_value.y &&
				int_value.z == v.int_value.z &&
				int_value.w == v.int_value.w;
		}

		return false;
	}

	std::string GetString() const
	{
		if (type == 0 && countChannels == 1) return ct::toStr(float_value.x);
		if (type == 0 && countChannels == 2) return ct::toStr(float_value.x) + ", " + ct::toStr(float_value.y);
		if (type == 0 && countChannels == 3) return ct::toStr(float_value.x) + ", " + ct::toStr(float_value.y) + ", " + ct::toStr(float_value.z);
		if (type == 0 && countChannels == 4) return ct::toStr(float_value.x) + ", " + ct::toStr(float_value.y) + ", " + ct::toStr(float_value.z) + ", " + ct::toStr(float_value.w);

		if (type == 1 && countChannels == 1) return ct::toStr(bool_value.x);
		if (type == 1 && countChannels == 2) return ct::toStr(bool_value.x) + ", " + ct::toStr(bool_value.y);
		if (type == 1 && countChannels == 3) return ct::toStr(bool_value.x) + ", " + ct::toStr(bool_value.y) + ", " + ct::toStr(bool_value.z);
		if (type == 1 && countChannels == 4) return ct::toStr(bool_value.x) + ", " + ct::toStr(bool_value.y) + ", " + ct::toStr(bool_value.z) + ", " + ct::toStr(bool_value.w);

		if (type == 2 && countChannels == 1) return ct::toStr(int_value.x);
		if (type == 2 && countChannels == 2) return ct::toStr(int_value.x) + ", " + ct::toStr(int_value.y);
		if (type == 2 && countChannels == 3) return ct::toStr(int_value.x) + ", " + ct::toStr(int_value.y) + ", " + ct::toStr(int_value.z);
		if (type == 2 && countChannels == 4) return ct::toStr(int_value.x) + ", " + ct::toStr(int_value.y) + ", " + ct::toStr(int_value.z) + ", " + ct::toStr(int_value.w);

		return "";
	}
};
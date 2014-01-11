#pragma once

#include "stdafx.h"

template <typename T>
class Singleton
{
	static T
		*inst;

public:
	static T *GetInstance()
	{
		if(!inst)
		{
			inst = new T;
		}

		return inst;
	}

	static void FreeInstance()
	{
		if(inst)
		{
			delete inst;

			inst = nullptr;
		}
	}
};

template <typename T>
T
	*::Singleton<T>::inst = nullptr;
/*
 * Array.h
 *
 *  Created on: Jan 8, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_ARRAY_H_
#define INCLUDE_DRIVER_ARRAY_H_
extern "C"
{
#include <c_types.h>
}

template<class T>
class PtrArray
{
protected:
	T **arry;
	size_t len;
	size_t size;
	size_t growsize;

	size_t Grow()
	{
		T **p = new T *[size + growsize];
		if (p == NULL)
			return 0;

		if (arry != NULL)
		{
			for (size_t i = 0; i < len; i++)
				p[i] = arry[i];

			delete[] arry;
			arry = (T**)NULL;
		}

		arry = p;
		return growsize;
	}

protected:

public:
	PtrArray(size_t gz = 10)
	{
		growsize = gz;
		len = size = 0;
		arry = (T**)NULL;
		Grow();
	};

	size_t length()
	{
		return len;
	}
	;
	void Add(T *item)
	{
		arry[len++] = item;
		if (len >= size)
			Grow();
	};

	T * RemoveAt(size_t idx)
	{
		T * ret = (T*)NULL;
		if (idx < len)
		{
			ret = arry[idx];
			for (; idx < len - 1; idx++)
				arry[idx] = arry[idx + 1];
			len--;
		}

		return ret;
	};

};

template<class T>
class Queue: public PtrArray<T>
{
public:
	Queue(size_t gz = 10) :
			PtrArray<T>(gz)
	{

	};

	void Push(T *item)
	{
		PtrArray<T>::Add(item);
	};

	T *Pop()
	{
		return PtrArray<T>::RemoveAt(0);
	};

	T *Peek()
	{
		return PtrArray<T>::arry[0];
	}
};
#endif /* INCLUDE_DRIVER_ARRAY_H_ */

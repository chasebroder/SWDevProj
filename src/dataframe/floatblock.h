#pragma once

#include "block.h"


/**
* FloatBlock - to represent a block of float.
*
*/
class FloatBlock : public Block
{
public:
	float* vals_; //list of float, owned
	size_t size_;
	size_t capacity_;

	// constructor
	FloatBlock()
	{
		capacity_ = BLOCK_SIZE;
		size_ = 0;
		vals_ = new float[capacity_];
		memset(vals_, 0, capacity_ * sizeof(float));
	}

	// deconstructor
	~FloatBlock()
	{
		delete[] vals_;
	}

	// get the float with the index in the array
	float get(size_t index)
	{
		// check for out-of-bounds
		if (index >= size_)
		{
			fprintf(stderr, "Out-Of-Bounds Error: cannot get value from index %zu", index);
			exit(1);
		}
		return vals_[index];
	}

	// add float to end of this block. if can't fit, return -1
	int add(float s)
	{
		if (size_ >= capacity_)
		{
			return -1;
		}
        vals_[size_] = s;
		size_++;
	}

	// set the element in the given index to the given float
	void set(size_t index, float s)
	{
		// check for out-of-bounds
		if (index >= size_)
		{
			printf("Out-Of-Bounds Error: cannot set value at index %zu", index);
			exit(1);
		}
		
		vals_[index] = s;
	}

};
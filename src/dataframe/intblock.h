#pragma once

#include "block.h"


/**
* IntBlock - to represent a block of ints.
*
*/
class IntBlock : public Block
{
public:
	int* vals_; //list of ints, owned
	size_t size_;
	size_t capacity_;

	// constructor
	IntBlock()
	{
		capacity_ = BLOCK_SIZE;
		size_ = 0;
		vals_ = new int[capacity_];
		memset(vals_, 0, capacity_ * sizeof(int));
	}

	// deconstructor
	~IntBlock()
	{
		delete[] vals_;
	}

	// get the int with the index in the array
	int get(size_t index)
	{
		// check for out-of-bounds
		if (index >= size_)
		{
			fprintf(stderr, "Out-Of-Bounds Error: cannot get value from index %zu", index);
			exit(1);
		}
		return vals_[index];
	}

	// add int to end of this block. if can't fit, return -1
	int add(int n)
	{
		if (size_ >= capacity_)
		{
			return -1;
		}
        vals_[size_] = n;
		size_++;
	}

	// set the element in the given index to the given object
	void set(size_t index, int v)
	{
		// check for out-of-bounds
		if (index >= size_)
		{
			printf("Out-Of-Bounds Error: cannot set value at index %zu", index);
			exit(1);
		}
		
		vals_[index] = v;
	}

};
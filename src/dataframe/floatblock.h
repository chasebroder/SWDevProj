#pragma once

#include "block.h"
#include "../serial/serial.h"


/**
* FloatBlock - to represent a block of floats.
*
*/
class FloatBlock : public Block
{
public:
	float* vals_; //list of floats, owned

	FloatBlock()
	{
		capacity_ = BLOCK_SIZE;
		size_ = 0;
		vals_ = new float[capacity_];
		memset(vals_, 0, capacity_ * sizeof(float));
	}

	~FloatBlock()
	{
		delete[] vals_;
	}
	
	/** Serialize this block of floats into s */
	void serialize(Serializer* s) {
		s->write(capacity_);
		s->write(size_);
		for (size_t i = 0; i < size_; i++) {
			s->write(vals_[i]);
		}
	}
	
	/** Deserialize a block of floats into this block (mutate) */
	void deserialize(Serializer* s) {
		delete[] vals_;
		capacity_ = s->readSizeT();
		vals_ = new float[capacity_];
		memset(vals_, 0, capacity_ * sizeof(float));
		size_ = s->readSizeT();
		for (size_t i = 0; i < size_; i++) {
			vals_[i] = s->readFloat();
		}
	}

	/** Gets the float at the specified index of the array */
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

	/** Adds the float to end of this block. If can't fit, return -1. */
	int add(float s)
	{
		if (size_ >= capacity_)
		{
			return -1;
		}
        vals_[size_] = s;
		size_++;
	}

	/** Set the float at the given index */
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

	/** Check if two blocks equal */
	bool equals(Object* other)
	{
		if (this == other) {
			return true;
		}

		FloatBlock* b = dynamic_cast<FloatBlock*>(other);
		if (b == nullptr || size_ != b->size_ || capacity_ != b->capacity_)
		{
			return false;
		}

		for (size_t i = 0; i < size_; i++)
		{
			//set a tolerance
			float tolerance = 0.0001;
			if (abs(vals_[i] - b->vals_[i]) > tolerance)
			{
				return false;
			}
		}

		return true;
	}
	
	/** Compute hash code of this bool block */
	size_t hash_me_()
	{
		size_t hash_ = 0;
		hash_ += size_;
		hash_ += capacity_;

		for (size_t i = 0; i < size_; i++)
		{
			if (vals_[i])
			{
				hash_ += 1;
			}
		}

		return hash_;
	}
	
	/** Clears the memory in this FloatBlock */
	void clear() {
		memset(vals_, 0, capacity_ * sizeof(float));
		size_ = 0;
	}
};

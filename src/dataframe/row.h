#pragma once

#include "../utils/object.h"
#include "../utils/string.h"
#include "schema.h"
#include "fielder.h"
#include "rowelement.h"

/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 */
class Row : public Object
{
public:
	Schema *scm_;
	RowElement **entries_;
	size_t len_;
	size_t rowIdx_;
	/** Build a row following a schema. */
	Row(Schema &scm)
	{
		scm_ = new Schema(scm);
		len_ = scm.width();
		entries_ = new RowElement *[len_];
		for (size_t i = 0; i < len_; i++)
		{
			entries_[i] = new RowElement();
		}
		rowIdx_ = 0;
	}

	~Row()
	{
		delete scm_;
		for (size_t i = 0; i < len_; i++)
		{
			delete entries_[i];
		}

		delete[] entries_;
	}

	/** Setters: set the given column with the given value. Setting a column with
    * a value of the wrong type is undefined. */
	void set(size_t col, int val)
	{
		exitIfBadSet_(col, 'I');
		entries_[col]->set(val);
	}

	void set(size_t col, float val)
	{
		exitIfBadSet_(col, 'F');
		entries_[col]->set(val);
	}

	void set(size_t col, bool val)
	{
		exitIfBadSet_(col, 'B');
		entries_[col]->set(val);
	}

	/** Acquire ownership of the string. */
	void set(size_t col, String *val)
	{
		exitIfBadSet_(col, 'S');
		entries_[col]->set(val);
	}

	/** Set/get the index of this row (ie. its position in the dataframe. This is
   *  only used for informational purposes, unused otherwise */
	void set_idx(size_t idx)
	{
		rowIdx_ = idx;
	}

	size_t get_idx()
	{
		return rowIdx_;
	}

	/** Getters: get the value at the given column. If the column is not
    * of the requested type, the result is undefined. */
	int get_int(size_t col)
	{
		exitIfBadSet_(col, 'I');
		if (entries_[col]->isSet())
		{
			return entries_[col]->getInt();
		}
	}
	bool get_bool(size_t col)
	{
		exitIfBadSet_(col, 'B');
		if (entries_[col]->isSet())
		{
			return entries_[col]->getBool();
		}

		exitIfNotSet_(col);
	}
	float get_float(size_t col)
	{
		exitIfBadSet_(col, 'F');
		if (entries_[col]->isSet())
		{
			return entries_[col]->getFloat();
		}

		exitIfNotSet_(col);
	}
	String *get_string(size_t col)
	{
		exitIfBadSet_(col, 'S');
		if (entries_[col]->isSet())
		{
			return entries_[col]->getString();
		}

		exitIfNotSet_(col);
	}

	/** Number of fields in the row. */
	size_t width()
	{
		return len_;
	}

	/** Type of the field at the given position. An idx >= width is  undefined. */
	char col_type(size_t idx)
	{
		if (idx >= len_)
		{
			fprintf(stderr, "Column %zu out of bounds in row", idx);
			exit(1);
		}
		return scm_->col_type(idx);
	}

	/** Given a Fielder, visit every field of this row. The first argument is
    * index of the row in the dataframe.
    * Calling this method before the row's fields have been set is undefined. */
	void visit(size_t idx, Fielder &f)
	{
		// idx is ignored
		f.start(idx);
		char colType = 0;
		RowElement *curElem = NULL;
		for (size_t i = 0; i < len_; i++)
		{
			colType = scm_->col_type(i);
			curElem = entries_[i];
			switch (colType)
			{
			case 'I':
				f.accept(curElem->getInt());
				break;
			case 'B':
				f.accept(curElem->getBool());
				break;
			case 'F':
				f.accept(curElem->getFloat());
				break;
			case 'S':
				f.accept(curElem->getString());
				break;
			default:
				fprintf(stderr, "Unknown col type in row: %c", colType);
				exit(2);
			}
		}
		f.done();
	}

	void exitIfBadSet_(size_t idx, char expectedType)
	{
		if (idx >= len_)
		{
			fprintf(stderr, "Column %zu out of bounds in row", idx);
			exit(1);
		}
		char actualType = scm_->col_type(idx);
		if (actualType != expectedType)
		{
			fprintf(stderr, "Cannot set row value. Failed to add entry of type %c to Column %zu of type %c", expectedType, idx, actualType);
			exit(1);
		}
	}

	/** Exits program if try to access value that's not set */
	void exitIfNotSet_(size_t idx)
	{
		fprintf(stderr, "Row element at column %zu is not set", idx);
		exit(1);
	}
};
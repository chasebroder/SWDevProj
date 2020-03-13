#pragma once

#include "../utils/object.h"
#include "../utils/string.h"
#include "../utils/helper.h"

#include "intcolumn.h"
#include "stringcolumn.h"
#include "boolcolumn.h"
#include "floatcolumn.h"
#include "columnarray.h"

#include "schema.h"
#include "rower.h"
#include "row.h"
#include "fielder.h"
#include "thread.h"

/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 *
 * @author kierzenka.m@husky.neu.edu & broder.c@husky.neu.edu
 */
class DataFrame : public Object
{
public:
  ColumnArray *columns_; //keeps track of columns in data frame
  Schema schema_;        //schema of dataframe

  /** Create a data frame with the same columns as the give df but no rows */
  DataFrame(DataFrame &df) : DataFrame(df.get_schema())
  {
  }

  /** Create a data frame from a schema and columns. All columns are created
    * empty. */
  DataFrame(Schema &schema)
  {
    //don't copy rows
    schema_ = *(new Schema(schema, false));
    size_t numCols = schema_.width();
    columns_ = new ColumnArray();
    for (int i = 0; i < numCols; i++)
    {
      addNewColumn_(schema_.col_type(i));
    }
  }

  ~DataFrame()
  {
    delete columns_;
  }

  /** Returns the dataframe's schema. Modifying the schema after a dataframe
    * has been created in undefined. */
  Schema &get_schema()
  {
    return schema_;
  }

  /** Adds a column this dataframe, updates the schema, the new column
    * is external, and appears as the last column of the dataframe, the
    * name is optional and external. A nullptr colum is undefined. */
  void add_column(Column *col, String *name)
  {
    if (col == nullptr)
    {
      fprintf(stderr, "Cannot add null column");
      exit(1);
    }
    else if (col->size() != schema_.length())
    {
      fprintf(stderr, "Cannot add column whose length (%zu) != number of rows in dataframe (%zu)\n", col->size(), schema_.length());
      exit(1);
    }

    //get type of column
    char type = getColType_(col);

    schema_.add_column(type, name);
    columns_->add(col);
  }

  /** Return the value at the given column and row. Accessing rows or
   *  columns out of bounds, or request the wrong type is undefined.*/
  int get_int(size_t col, size_t row)
  {
    IntColumn *tmp = safeConvertIntCol_(col);
    return tmp->get(row);
  }

  bool get_bool(size_t col, size_t row)
  {
    BoolColumn *tmp = safeConvertBoolCol_(col);
    return tmp->get(row);
  }

  float get_float(size_t col, size_t row)
  {
    FloatColumn *tmp = safeConvertFloatCol_(col);
    return tmp->get(row);
  }

  String *get_string(size_t col, size_t row)
  {
    StringColumn *tmp = safeConvertStringCol_(col);
    return tmp->get(row);
  }

  /** Return the offset of the given column name or -1 if no such col. */
  int get_col(String &col)
  {
    return schema_.col_idx(col.c_str());
  }

  /** Return the offset of the given row name or -1 if no such row. */
  int get_row(String &col)
  {
    return schema_.row_idx(col.c_str());
  }

  /** Set the value at the given column and row to the given value.
    * If the column is not  of the right type or the indices are out of
    * bound, the result is undefined. */
  void set(size_t col, size_t row, int val)
  {
    IntColumn *tmp = safeConvertIntCol_(col);
    tmp->set(row, val);
  }

  void set(size_t col, size_t row, bool val)
  {
    BoolColumn *tmp = safeConvertBoolCol_(col);
    tmp->set(row, val);
  }

  void set(size_t col, size_t row, float val)
  {
    FloatColumn *tmp = safeConvertFloatCol_(col);
    tmp->set(row, val);
  }

  void set(size_t col, size_t row, String *val)
  {
    StringColumn *tmp = safeConvertStringCol_(col);
    tmp->set(row, val);
  }

  void push_back(size_t col, int val)
  {
    IntColumn *tmp = safeConvertIntCol_(col);
    tmp->push_back(val);
  }

  void push_back(size_t col, bool val)
  {
    BoolColumn *tmp = safeConvertBoolCol_(col);
    tmp->push_back(val);
  }

  void push_back(size_t col, float val)
  {
    FloatColumn *tmp = safeConvertFloatCol_(col);
    tmp->push_back(val);
  }

  void push_back(size_t col, String *val)
  {
    StringColumn *tmp = safeConvertStringCol_(col);
    tmp->push_back(val);
  }

  /** Set the fields of the given row object with values from the columns at
    * the given offset.  If the row is not form the same schema as the
    * dataframe, results are undefined.
    */
  void fill_row(size_t idx, Row &row)
  {
    if (idx >= schema_.length())
    {
      fprintf(stderr, "Row %zu does not exist in dataframe", idx);
      exit(1);
    }

    size_t rowWidth = row.width();
    checkRowEntries_(rowWidth);
    for (int i = 0; i < rowWidth; i++)
    {
      checkColTypes_(row.col_type(i), i);
      setColumnValByType_(i, idx, row);
    }
  }

  /** Add a row at the end of this dataframe. The row is expected to have
   *  the right schema and be filled with values, otherwise undedined.  */
  void add_row(Row &row)
  {
    size_t rowWidth = row.width();
    checkRowEntries_(rowWidth);
    for (int i = 0; i < rowWidth; i++)
    {
      checkColTypes_(row.col_type(i), i);
      addToEndOfColByType_(i, row);
    }

    //Row does not have a name
    schema_.add_row(nullptr);
  }

  /** The number of rows in the dataframe. */
  size_t nrows()
  {
    return schema_.length();
  }

  /** The number of columns in the dataframe.*/
  size_t ncols()
  {
    return schema_.width();
  }

  /** Visit rows in order */
  void map(Rower &r)
  {
    map(r, 0, schema_.length());
  }

  /** Visit subset of rows in order */
  void map(Rower &r, size_t startIdx, size_t endIdx)
  {
    Row *row = new Row(schema_);

    for (size_t rowIdx = startIdx; rowIdx < endIdx; rowIdx++)
    {
      row->set_idx(rowIdx);
      //iterate through each column to get value
      for (int colIdx = 0; colIdx < row->width(); colIdx++)
      {
        setRowValByColType_(*row, colIdx, rowIdx, schema_.col_type(colIdx));
      }

      r.accept(*row);
    }

    delete row;
  }

  /** An inner class of DataFrame representing a Thread for doing work on a contiguous
	 *    subset of rows of a DataFrame.
	 *  
	 *  author: kierzenka.m@husky.neu.edu */
  class DataFrameThread : public Thread
  {
  public:
    size_t startRowIdx_; // inclusive
    size_t endRowIdx_;   // exclusive
    DataFrame *df_;
    Rower &r_;

    DataFrameThread(DataFrame *df, Rower &r, size_t startRowIdx, size_t endRowIdx) : df_(df), r_(r), startRowIdx_(startRowIdx), endRowIdx_(endRowIdx) {}

    void run()
    {
      df_->map(r_, startRowIdx_, endRowIdx_);
    }
  };

  /** This method clones the Rower and executes the map in parallel over
	 *  the specified number of threads.
	 */
  void pmap(Rower &r, size_t numThreads)
  {
    if (numThreads == 0)
    {
      fprintf(stderr, "Number of threads to execute in parallel (%zu) must be > 0", numThreads);
      exit(1);
    }
    DataFrameThread **threads = new DataFrameThread *[numThreads];
    size_t startIdx = 0, endIdx = 0, step = schema_.length() / numThreads;
    int i;
    for (i = 0; i < numThreads - 1; i++)
    {
      startIdx = i * step;
      endIdx = (i + 1) * step;
      threads[i] = new DataFrameThread(this, r, startIdx, endIdx);
      threads[i]->start();
    }

    //handle remaining rows in last thread
    startIdx = i * step;
    endIdx = schema_.length();
    threads[i] = new DataFrameThread(this, r, startIdx, endIdx);
    threads[i]->start();

    for (int i = numThreads - 1; i >= 0; i--)
    {
      threads[i]->join();
    }
    delete threads;
  }

  /** This method clones the Rower and executes the map in parallel. Join is
	* used at the end to merge the results. Creates (number of threads / 2) + 1
	* parallel worker threads. This means that for Intel CPUs with hyperthreading,
	* this will run 1 thread per physical CPU core + 1 extra
	*/
  void pmap(Rower &r)
  {
    size_t numThreads = (std::thread::hardware_concurrency() / 2) + 1;
    //p("Number of threads in pmap: ").pln(numThreads);
    pmap(r, numThreads);
  }

  /** Create a new dataframe, constructed from rows for which the given Rower
    * returned true from its accept method. */
  DataFrame *filter(Rower &r)
  {
    DataFrame *newFrame = new DataFrame(schema_);
    for (int i = 0; i < schema_.length(); i++)
    {
      Row *row = createRow_(i);
      if (r.accept(*row))
      {
        newFrame->add_row(*row);
      }
      delete row;
    }

    return newFrame;
  }

  /** Print the dataframe in SoR format to standard output. */
  void print()
  {
    for (int i = 0; i < schema_.width(); i++)
    {
      for (int j = 0; j < schema_.length(); i++)
      {
        columns_->get(i)->printElement(j);
      }

      printf("\n");
    }
  }

  /** Return proper column for type */
  void addNewColumn_(char type)
  {
    switch (type)
    {
    case 'I':
      columns_->add(new IntColumn());
      break;
    case 'B':
      columns_->add(new BoolColumn());
      break;
    case 'F':
      columns_->add(new FloatColumn());
      break;
    case 'S':
      columns_->add(new StringColumn());
      break;
    default:
      fprintf(stderr, "Invalid column type %c\n", type);
      exit(2);
    }
    // We get the columns from the schema, so no need to add cols to it here
  }

  /** Set the field from the row given the type */
  void setColumnValByType_(size_t colIdx, size_t rowIdx, Row &row)
  {
    switch (row.col_type(colIdx))
    {
    case 'I':
      set(colIdx, rowIdx, row.get_int(colIdx));
      break;
    case 'B':
      set(colIdx, rowIdx, row.get_bool(colIdx));
      break;
    case 'F':
      set(colIdx, rowIdx, row.get_float(colIdx));
      break;
    case 'S':
      set(colIdx, rowIdx, row.get_string(colIdx));
      break;
    default:
      fprintf(stderr, "Invalid col type: %c", row.col_type(colIdx));
    }
  }

  /** Add to end of column depending on the type */
  void addToEndOfColByType_(size_t colIdx, Row &row)
  {
    switch (row.col_type(colIdx))
    {
    case 'I':
      safeConvertIntCol_(colIdx)->push_back(row.get_int(colIdx));
      break;
    case 'B':
      safeConvertBoolCol_(colIdx)->push_back(row.get_bool(colIdx));
      break;
    case 'F':
      safeConvertFloatCol_(colIdx)->push_back(row.get_float(colIdx));
      break;
    case 'S':
      safeConvertStringCol_(colIdx)->push_back(row.get_string(colIdx));
      break;
    default:
      fprintf(stderr, "Invalid col type: %c", row.col_type(colIdx));
    }
  }

  /** Return pointer to column at given index as IntColumn
	* Errors and exits if no column at index or of improper type*/
  IntColumn *safeConvertIntCol_(size_t colIdx)
  {
    errorIfOutOfBounds_(colIdx);
    IntColumn *ic = columns_->get(colIdx)->as_int();
    if (ic == nullptr)
    {
      fprintf(stderr, "Illegal Column Conversion: column %zu is not an int column", colIdx);
      exit(1);
    }
    return ic;
  }

  /** Return pointer to column at given index as BoolColumn
	* Errors and exits if no column at index or of improper type*/
  BoolColumn *safeConvertBoolCol_(size_t colIdx)
  {
    errorIfOutOfBounds_(colIdx);
    BoolColumn *ic = columns_->get(colIdx)->as_bool();
    if (ic == nullptr)
    {
      fprintf(stderr, "Illegal Column Conversion: column %zu is not an bool column", colIdx);
      exit(1);
    }
    return ic;
  }

  /** Return pointer to column at given index as FloatColumn
	* Errors and exits if no column at index or of improper type*/
  FloatColumn *safeConvertFloatCol_(size_t colIdx)
  {
    errorIfOutOfBounds_(colIdx);
    FloatColumn *ic = columns_->get(colIdx)->as_float();
    if (ic == nullptr)
    {
      fprintf(stderr, "Illegal Column Conversion: column %zu is not a float column", colIdx);
      exit(1);
    }
    return ic;
  }

  /** Return pointer to column at given index as StringColumn
	* Errors and exits if no column at index or of improper type*/
  StringColumn *safeConvertStringCol_(size_t colIdx)
  {
    errorIfOutOfBounds_(colIdx);
    StringColumn *ic = columns_->get(colIdx)->as_string();
    if (ic == nullptr)
    {
      fprintf(stderr, "Illegal Column Conversion: column %zu is not a string column", colIdx);
      exit(1);
    }
    return ic;
  }

  /** Helper that allows program to error and exit if index-out-of-bounds */
  void errorIfOutOfBounds_(size_t colIdx)
  {
    if (colIdx >= schema_.width())
    {
      fprintf(stderr, "Out-Of-Bounds Error: cannot get column from index %zu", colIdx);
      exit(1);
    }
  }

  /** Error and exit if number of entries in row does not equal number of columns
   * in schema */
  void checkRowEntries_(size_t rowWidth)
  {
    if (rowWidth != schema_.width())
    {
      fprintf(stderr, "Cannot have row of %zu entries in schema of %zu columns",
              rowWidth, schema_.width());
      exit(1);
    }
  }

  /** Checks for matching col types. Error and exit if not the same */
  void checkColTypes_(char colTypeFromRow, size_t schemaColIdx)
  {
    char colTypeFromSchema = schema_.col_type(schemaColIdx);
    if (colTypeFromRow != colTypeFromSchema)
    {
      fprintf(stderr, "Row's column type \"%c\" at index %zu does not match dataframe's column type \"%c\"", colTypeFromRow, schemaColIdx, colTypeFromSchema);
      exit(1);
    }
  }

  /** Create Row object from the DataFrame info */
  Row *createRow_(size_t rowIdx)
  {
    Row *r = new Row(schema_);
    r->set_idx(rowIdx);

    //iterate through each column to get value
    for (int i = 0; i < r->width(); i++)
    {
      setRowValByColType_(*r, i, rowIdx, schema_.col_type(i));
    }

    return r;
  }

  /** Set value in Row, given the column type */
  void setRowValByColType_(Row &r, size_t colIdx, size_t rowIdx, char colType)
  {
    switch (colType)
    {
    case 'I':
      r.set(colIdx, safeConvertIntCol_(colIdx)->get(rowIdx));
      break;
    case 'B':
      r.set(colIdx, safeConvertBoolCol_(colIdx)->get(rowIdx));
      break;
    case 'F':
      r.set(colIdx, safeConvertFloatCol_(colIdx)->get(rowIdx));
      break;
    case 'S':
      r.set(colIdx, safeConvertStringCol_(colIdx)->get(rowIdx)->clone());
      break;
    default:
      fprintf(stderr, "Invalid col type: %c", colType);
    }
  }

  char getColType_(Column *col)
  {
    IntColumn *ic = col->as_int();
    FloatColumn *fc = col->as_float();
    BoolColumn *bc = col->as_bool();
    StringColumn *sc = col->as_string();
    if (ic != nullptr)
    {
      return ic->get_type();
    }
    else if (fc != nullptr)
    {
      return fc->get_type();
    }
    else if (bc != nullptr)
    {
      return bc->get_type();
    }
    else if (sc != nullptr)
    {
      return sc->get_type();
    }

    //doesn't match any types, so delegate to Column
    return col->get_type();
  }
};
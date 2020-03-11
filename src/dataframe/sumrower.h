#pragma once

#include "rower.h"
#include "dataframe.h"
#include "row.h"

/*******************************************************************************
 *  SumRower::
 *  A Rower implementation that computes the total sum of entries in a data frame.
 */
class SumRower : public Rower {
 public:
	DataFrame* df_;
	int sum_;
	size_t colToReportIdx_ = 2;
	
	SumRower(DataFrame* df) {
		df_ = df;
		sum_ = 0;
	}
	
  /** This method is called once per row. The row object is on loan and
      should not be retained as it is likely going to be reused in the next
      call. The return value is used in filters to indicate that a row
      should be kept. */
  bool accept(Row& r) {
	  sum_ = 0;
	  for (size_t i = 0; i < colToReportIdx_; i++) {
			sum_ += r.get_int(i);
	  }
	  df_->set(colToReportIdx_, r.get_idx(), sum_);
	  return true;
  }
 
  /** Once traversal of the data frame is complete the rowers that were
      split off will be joined.  There will be one join per split. The
      original object will be the last to be called join on. The join method
      is reponsible for cleaning up memory. */
  void join_delete(Rower* other) {}
};
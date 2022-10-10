// Copyright 2021 Tier IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#ifndef SEGMENTATION_EVALUATOR__STAT_HPP_
#define SEGMENTATION_EVALUATOR__STAT_HPP_

namespace segmentation_diagnostics
{
/**
 * @brief class to incrementally build statistics
 * @typedef T type of the values (default to double)
 */
template <typename T = double>
class Stat
{
public:
  /**
   * @brief get cumulative true positives
   */
  int tp() const { return tp_accu_; }

  /**
   * @brief get cumulative false positives
   */
  int fp() const { return fp_accu_; }

  /**
   * @brief get cumulative false negatives
   */
  int fn() const { return fn_accu_; }

  /**
   * @brief get cumulative true negatives
   */
  int tn() const { return tn_accu_; }

  /**
   * @brief get cumulative accuracy
   */
  float accuracy() const
  {
    return float(tp_accu_ + tn_accu_) / float(tp_accu_ + tn_accu_ + fn_accu_ + fp_accu_);
  }

  /**
   * @brief get cumulative precision
   */
  float precision() const { return float(tp_accu_) / float(tp_accu_ + fp_accu_); }

  /**
   * @brief get cumulative recall
   */
  float recall() const { return float(tp_accu_) / float(tp_accu_ + fn_accu_); }

  /**
   * @brief Print confusion matrix
   */
  void printConfMat()
  {
    std::cout << std::left << std::fixed
              << "            ┌──────────────┬─────────────────────────────┐\n"
                 "            │              │       Predicted values      │\n"
                 "            │  Confusion   ├──────────────┬──────────────┤\n"
                 "            │   matrix     │    object    │    ground    │\n"
                 "┌───────────┼──────────────┼──────────────┼──────────────┤\n"
                 "│  Actual   │    object    │"
              << std::setw(14) << tp_ << "│" << std::setw(13) << fn_
              << " │\n"
                 "│  values   ├──────────────┼──────────────┼──────────────┤\n"
                 "│           │    ground    │"
              << std::setw(14) << fp_ << "│" << std::setw(13) << tn_
              << " │\n"
                 "└───────────┴──────────────┴──────────────┴──────────────┘\n";
  }

  /**
   * @brief Print accumulated confusion matrix
   */
  void printConfMatAccu()
  {
    std::cout << std::left << std::fixed
              << "            ┌──────────────┬─────────────────────────────┐\n"
                 "            │              │       Predicted values      │\n"
                 "            │  Confusion   ├──────────────┬──────────────┤\n"
                 "            │   matrix     │    object    │    ground    │\n"
                 "┌───────────┼──────────────┼──────────────┼──────────────┤\n"
                 "│  Actual   │    object    │"
              << std::setw(14) << tp_accu_ << "│" << std::setw(13) << fn_accu_
              << " │\n"
                 "│  values   ├──────────────┼──────────────┼──────────────┤\n"
                 "│           │    ground    │"
              << std::setw(14) << fp_accu_ << "│" << std::setw(13) << tn_accu_
              << " │\n"
                 "└───────────┴──────────────┴──────────────┴──────────────┘\n";
  }

  /**
   * @brief set confusion matrix
   */
  void setConfustionMatPerCloud(const std::vector<int> & conf_mat)
  {
    tp_ = conf_mat[0];
    fn_ = conf_mat[1];
    fp_ = conf_mat[2];
    tn_ = conf_mat[3];
    accumulateConfustionMat(conf_mat);
  }

  /**
   * @brief accumulate confusion matrix
   */
  void accumulateConfustionMat(const std::vector<int> & conf_mat)
  {
    tp_accu_ += conf_mat[0];
    fn_accu_ += conf_mat[1];
    fp_accu_ += conf_mat[2];
    tn_accu_ += conf_mat[3];
  }

  /**
   * @brief get the number of values used to build this statistic
   */
  unsigned int count() const { return count_; }

  template <typename U>
  friend std::ostream & operator<<(std::ostream & os, const Stat<U> & stat);

private:
  int tp_ = 0, fn_ = 0, fp_ = 0, tn_ = 0;
  int tp_accu_ = 0, fn_accu_ = 0, fp_accu_ = 0, tn_accu_ = 0;

  unsigned int count_ = 0;
};

/**
 * @brief overload << operator for easy print to output stream
 */
template <typename T>
std::ostream & operator<<(std::ostream & os, const Stat<T> & stat)
{
  if (stat.count() == 0) {
    os << "None None None";
  } else {
    os << stat.min() << " " << stat.max() << " " << stat.mean();
  }
  return os;
}

}  // namespace segmentation_diagnostics

#endif  // SEGMENTATION_EVALUATOR__STAT_HPP_
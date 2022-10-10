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

#include "segmentation_evaluator/segmentation_evaluator_node.hpp"

#include "boost/lexical_cast.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace segmentation_diagnostics
{
SegmentationEvaluatorNode::SegmentationEvaluatorNode(const rclcpp::NodeOptions & node_options)
: Node("segmentation_evaluator", node_options),
  pcl_sub_(this, "~/input/pcl/to/eval", rclcpp::QoS{1}.best_effort().get_rmw_qos_profile()),
  pcl_gt_negative_cls_sub_(
    this, "~/input/pcl/gt/negative_cls", rclcpp::QoS{1}.best_effort().get_rmw_qos_profile()),
  pcl_gt_positive_cls_sub_(
    this, "~/input/pcl/gt/positive_cls", rclcpp::QoS{1}.best_effort().get_rmw_qos_profile()),
  sync_(SyncPolicy(10), pcl_sub_, pcl_gt_negative_cls_sub_, pcl_gt_positive_cls_sub_)
{
  tf_buffer_ptr_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
  tf_listener_ptr_ = std::make_unique<tf2_ros::TransformListener>(*tf_buffer_ptr_);

  sync_.registerCallback(std::bind(
    &SegmentationEvaluatorNode::syncCallback, this, std::placeholders::_1, std::placeholders::_2,
    std::placeholders::_3));

  output_file_str_ = declare_parameter<std::string>("output_file");
  if (output_file_str_.empty()) {
    RCLCPP_INFO(
      get_logger(),
      "Output file not specified, the results will NOT be saved!"
      "Provide output_file parameter to store the results.");
  }
  // List of metrics to calculate
  metrics_pub_ = create_publisher<DiagnosticArray>("~/metrics", 1);
  for (const std::string & selected_metric :
       declare_parameter<std::vector<std::string>>("selected_metrics")) {
    Metric metric = str_to_metric.at(selected_metric);
    metrics_dict_[metric] = Stat<double>();
    metrics_.push_back(metric);
  }
}

SegmentationEvaluatorNode::~SegmentationEvaluatorNode() {}

DiagnosticStatus SegmentationEvaluatorNode::generateDiagnosticStatus(
  const Metric & metric, const Stat<double> & metric_stat) const
{
  DiagnosticStatus status;
  status.level = status.OK;
  status.name = metric_to_str.at(metric);
  diagnostic_msgs::msg::KeyValue key_value;
  key_value.key = "accuracy";
  key_value.value = boost::lexical_cast<decltype(key_value.value)>(metric_stat.accuracy());
  status.values.push_back(key_value);
  key_value.key = "precision";
  key_value.value = boost::lexical_cast<decltype(key_value.value)>(metric_stat.precision());
  status.values.push_back(key_value);
  key_value.key = "recall";
  key_value.value = boost::lexical_cast<decltype(key_value.value)>(metric_stat.recall());
  status.values.push_back(key_value);
  return status;
}

void SegmentationEvaluatorNode::syncCallback(
  const PointCloud2::ConstSharedPtr & msg, const PointCloud2::ConstSharedPtr & msg_gt_negative_cls,
  const PointCloud2::ConstSharedPtr & msg_gt_positive_cls)
{
  DiagnosticArray metrics_msg;
  metrics_msg.header.stamp = now();

  for (Metric metric : metrics_) {
    metrics_dict_[metric] = metrics_calculator_.updateStat(
      metrics_dict_[metric], metric, *msg, *msg_gt_negative_cls, *msg_gt_positive_cls);

    if (metrics_dict_[metric].count() > 0) {
      metrics_msg.status.push_back(generateDiagnosticStatus(metric, metrics_dict_[metric]));
    }
  }

  if (!metrics_msg.status.empty()) {
    metrics_pub_->publish(metrics_msg);
  }
}
}  // namespace segmentation_diagnostics

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(segmentation_diagnostics::SegmentationEvaluatorNode)
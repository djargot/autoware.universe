cmake_minimum_required(VERSION 3.14)
project(obstacle_stop_planner)

find_package(autoware_cmake REQUIRED)
autoware_package()

find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PCL REQUIRED)

ament_auto_add_library(obstacle_stop_planner SHARED
  src/debug_marker.cpp
  src/node.cpp
  src/adaptive_cruise_control.cpp
)

target_include_directories(obstacle_stop_planner
  SYSTEM PUBLIC
    ${OpenCV_INCLUDE_DIRS}
    ${PCL_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(obstacle_stop_planner
    ${OpenCV_LIBRARIES}
    ${PCL_LIBRARIES}
)

rclcpp_components_register_node(obstacle_stop_planner
  PLUGIN "motion_planning::ObstacleStopPlannerNode"
  EXECUTABLE obstacle_stop_planner_node
)

ament_auto_package(
  INSTALL_TO_SHARE
  config
  launch
)

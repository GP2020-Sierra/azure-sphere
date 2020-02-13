CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Sierra2020 C)

add_library (parson parson.c)
add_library (epoll_timerfd_utilities epoll_timerfd_utilities.c)

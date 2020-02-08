CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Sierra2020 C)

add_library (avnet_onboard onboard.c lps22hh_reg.c lsm6dso_reg.c)

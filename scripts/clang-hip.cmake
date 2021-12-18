#[[

Default CMake cache for building in CI on AMDGPU platforms

#]]
set(HIOP_BUILD_SHARED OFF CACHE BOOL "")
set(HIOP_BUILD_STATIC ON CACHE BOOL "")
set(ENABLE_TESTS ON CACHE BOOL "")
set(HIOP_USE_MPI ON CACHE BOOL "")
set(HIOP_USE_RAJA ON CACHE BOOL "")
set(HIOP_USE_UMPIRE ON CACHE BOOL "")
set(HIOP_USE_GPU ON CACHE BOOL "")
set(HIOP_USE_CUDA OFF CACHE BOOL "")
set(HIOP_USE_HIP ON CACHE BOOL "")
set(HIOP_SPARSE OFF CACHE BOOL "")
set(HIOP_DEEPCHECKS ON CACHE BOOL "")
set(AMDGPU_TARGETS gfx908 CACHE STRING "")
set(GPU_TARGETS gfx908 CACHE STRING "")

set(CMAKE_C_COMPILER clang CACHE STRING "")
set(CMAKE_CXX_COMPILER hipcc CACHE STRING "")
set(CMAKE_Fortran_COMPILER gfortran CACHE STRING "")

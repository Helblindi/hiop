#pragma once
#include <cstddef>
#include <cstdio>
#include <string>

#include "hiopMatrixDenseRaja.hpp"

namespace hiop
{
//todo: remove this file, update LA factory
#if defined(HIOP_USE_CUDA)
  using hiopMatrixRajaDense = hiopMatrixDenseRaja<MemBackendUmpire, ExecPolicyRajaCuda>;
#elif defined(HIOP_USE_HIP)
  using hiopMatrixRajaDense = hiopMatrixDenseRaja<MemBackendUmpire, ExecPolicyRajaHip>;
#else
  
  using hiopMatrixRajaDense = hiopMatrixDenseRaja<MemBackendUmpire, ExecPolicyRajaOmp>;
#endif


} // namespace hiop

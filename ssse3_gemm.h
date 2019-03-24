#pragma once
#include "types.h"
#include <cstdint>
#include <stdint.h>

// 16-bit is in sse2_gemm.h

namespace intgemm {

// pmaddubsw (the 8-bit multiply) is SSSE3, so pedantically that's the version we need.
struct SSSE3_8bit {
  typedef int8_t Integer;

  // Currently A is prepared by quantization but this could theoretically change.
  static inline void PrepareA(const float *input, int8_t *output, float quant_mult, Index rows, Index cols) {
    Quantize(input, output, quant_mult, rows * cols);
  }

  static void Quantize(const float *input, int8_t *output, float quant_mult, Index size);

  // Tile size for B; B must be a multiple of this block size.
  static const Index kBTileRow = 16;
  static const Index kBTileCol = 8;

  static void PrepareB(const float *input, int8_t *output, float quant_mult, Index rows, Index cols);

  static void SelectColumnsB(const int8_t *input, int8_t *output, Index rows, const Index *cols_begin, const Index *cols_end);

  static void Multiply(const int8_t *A, const int8_t *B, float *C, float unquant_mult, Index A_rows, Index width, Index B_cols);
  
  static const char *const kName;

  static const CPUType kUses = CPU_SSSE3;
};

} // namespace intgemm

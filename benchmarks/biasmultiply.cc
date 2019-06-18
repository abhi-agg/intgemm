#include "intgemm.h"
#include "aligned.h"
#include <chrono>
#include <random>
#include <iostream>

using namespace intgemm;

template <class Routine>
void testOld(Index rows, Index cols) {

}

template <class Routine>
std::chrono::duration<double> testNew(Index A_rows, Index width, Index B_cols) {
  AlignedVector<float> A(A_rows * width);
  AlignedVector<float> B(width * B_cols);
  AlignedVector<float> bias(B_cols);
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  for (auto& it : A) {
    it = dist(gen);
  }
  for (auto& it : B) {
    it = dist(gen);
  }
  for (auto& it : bias) {
    it = dist(gen);
  }
  
  float alpha = 2.0f;
  float quant_mult = 127/alpha;
  float unquant_mult = 1.0/(quant_mult*quant_mult);

  AlignedVector<uint8_t> A_prep(A.size());
  AlignedVector<int8_t> B_prep(B.size());
  Routine::PrepareA(A.begin(), A_prep.begin(), quant_mult, A_rows, width);
  Routine::PrepareB(B.begin(), B_prep.begin(), quant_mult, width, B_cols);

  AlignedVector<float> test_C(A_rows * B_cols);

  Routine::PrepareBiasFor8(B.begin(), bias.begin(), alpha, width, B_cols);
  auto start = std::chrono::system_clock::now();
  Routine::Multiply8new(A_prep.begin(), B_prep.begin(), BiasAddUnquantizeC(test_C.begin(), bias.begin(), unquant_mult), A_rows, width, B_cols);
  auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;
  return elapsed_seconds;

}

template <class Routine>
std::chrono::duration<double> testOld(Index A_rows, Index width, Index B_cols) {
  AlignedVector<float> A(A_rows * width);
  AlignedVector<float> B(width * B_cols);
  AlignedVector<float> bias(B_cols);
  std::mt19937 gen;
  std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
  for (auto& it : A) {
    it = dist(gen);
  }
  for (auto& it : B) {
    it = dist(gen);
  }
  for (auto& it : bias) {
    it = dist(gen);
  }
  
  float alpha = 2.0f;
  float quant_mult = 127/alpha;
  float unquant_mult = 1.0/(quant_mult*quant_mult);

  AlignedVector<int8_t> A_prep(A.size());
  AlignedVector<int8_t> B_prep(B.size());
  Routine::PrepareA(A.begin(), A_prep.begin(), quant_mult, A_rows, width);
  Routine::PrepareB(B.begin(), B_prep.begin(), quant_mult, width, B_cols);

  AlignedVector<float> test_C(A_rows * B_cols);

  auto start = std::chrono::system_clock::now();
  Routine::Multiply(A_prep.begin(), B_prep.begin(), BiasAddUnquantizeC(test_C.begin(), bias.begin(), unquant_mult), A_rows, width, B_cols);
  auto end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;
  return elapsed_seconds;

}

int main(int argc, char ** argv) {
	int repeat = 1000;
	if (argc > 1) {
		repeat = atoi(argv[1]);
	}
	

	std::chrono::duration<double> oldSSSE3 = testOld<SSSE3_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		oldSSSE3 += testOld<SSSE3_8bit>(8, 256, 256);
		oldSSSE3 += testOld<SSSE3_8bit>(8, 2048, 256);
		oldSSSE3 += testOld<SSSE3_8bit>(320, 256, 256);
		oldSSSE3 += testOld<SSSE3_8bit>(472, 256, 256);
		oldSSSE3 += testOld<SSSE3_8bit>(248, 256, 256);
		oldSSSE3 += testOld<SSSE3_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of Old SSSE3 took: " << oldSSSE3.count() << " seconds." << std::endl;

	std::chrono::duration<double> newTimeSSSE3 = testOld<SSSE3_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		newTimeSSSE3 += testNew<SSSE3_8bit>(8, 256, 256);
		newTimeSSSE3 += testNew<SSSE3_8bit>(8, 2048, 256);
		newTimeSSSE3 += testNew<SSSE3_8bit>(320, 256, 256);
		newTimeSSSE3 += testNew<SSSE3_8bit>(472, 256, 256);
		newTimeSSSE3 += testNew<SSSE3_8bit>(248, 256, 256);
		newTimeSSSE3 += testNew<SSSE3_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of New SSSE3 took: " << newTimeSSSE3.count() << " seconds." << std::endl;

	std::chrono::duration<double> oldAVX2 = testOld<AVX2_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		oldAVX2 += testOld<AVX2_8bit>(8, 256, 256);
		oldAVX2 += testOld<AVX2_8bit>(8, 2048, 256);
		oldAVX2 += testOld<AVX2_8bit>(320, 256, 256);
		oldAVX2 += testOld<AVX2_8bit>(472, 256, 256);
		oldAVX2 += testOld<AVX2_8bit>(248, 256, 256);
		oldAVX2 += testOld<AVX2_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of Old AVX2 took: " << oldAVX2.count() << " seconds." << std::endl;

	std::chrono::duration<double> newTimeAVX2 = testOld<AVX2_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		newTimeAVX2 += testNew<AVX2_8bit>(8, 256, 256);
		newTimeAVX2 += testNew<AVX2_8bit>(8, 2048, 256);
		newTimeAVX2 += testNew<AVX2_8bit>(320, 256, 256);
		newTimeAVX2 += testNew<AVX2_8bit>(472, 256, 256);
		newTimeAVX2 += testNew<AVX2_8bit>(248, 256, 256);
		newTimeAVX2 += testNew<AVX2_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of New AVX2 took: " << newTimeAVX2.count() << " seconds." << std::endl;

	if (kCPU < CPU_AVX512BW) return 0;
	std::chrono::duration<double> oldAVX512 = testOld<AVX512_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		oldAVX512 += testOld<AVX512_8bit>(8, 256, 256);
		oldAVX512 += testOld<AVX512_8bit>(8, 2048, 256);
		oldAVX512 += testOld<AVX512_8bit>(320, 256, 256);
		oldAVX512 += testOld<AVX512_8bit>(472, 256, 256);
		oldAVX512 += testOld<AVX512_8bit>(248, 256, 256);
		oldAVX512 += testOld<AVX512_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of Old AVX512 took: " << oldAVX512.count() << " seconds." << std::endl;

	std::chrono::duration<double> newTimeAVX512 = testOld<AVX512_8bit>(1, 64, 8);
	for (int i = 0; i<repeat; i++) {
		newTimeAVX512 += testNew<AVX512_8bit>(8, 256, 256);
		newTimeAVX512 += testNew<AVX512_8bit>(8, 2048, 256);
		newTimeAVX512 += testNew<AVX512_8bit>(320, 256, 256);
		newTimeAVX512 += testNew<AVX512_8bit>(472, 256, 256);
		newTimeAVX512 += testNew<AVX512_8bit>(248, 256, 256);
		newTimeAVX512 += testNew<AVX512_8bit>(200, 256, 256);
	}

	std::cout << repeat << " iterations of New AVX512 took: " << newTimeAVX512.count() << " seconds." << std::endl;


}

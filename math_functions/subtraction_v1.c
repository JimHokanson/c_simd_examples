//
//  subtraction_v1.c
//
//  Setup
//  -----
//  AVX  _mm256_set_ps - set 256 bit vector with 8 floats
//
//  Instructions
//  ------------
//  AVX  _mm256_sub_ps - subtract 8 floats in one array from another array
//  

//  cd G:\repos\c_git\c_simd_examples\math_functions
//  gcc -mavx -o subtraction_v1 subtraction_v1.c

#include <immintrin.h>
#include <stdio.h>

int main() {

  //    Initial example from: http://www.codeproject.com/Articles/874396/Crunching-Numbers-with-AVX-and-AVX
  
  //  Initialization of vectors with 8 floats each
  __m256 evens = _mm256_set_ps(2.0, 5.0, 8.0, 11.0, 14.0, 17.0, 19.0, 22.0);
  __m256 odds  = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0,  11.0, 13.0, 15.0);

  /* Compute the difference between the two vectors */
  //result = evens - odds
  __m256 result = _mm256_sub_ps(evens, odds);

  //Display results, note these are perhaps reverse of what might be expected
  //largest difference is first
  float* f = (float*)&result;
  printf("%f %f %f %f %f %f %f %f\n",
            f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);
  //Result: 7.0 (f[0]), 6.0, 5.0 etc ...
  
  return 0;
}
//
//  direct_initialization.c
//
//  Instructions
//  ------------
//  AVX  _mm256_set_ps - set 256 bit vector with 8 floats

#include <immintrin.h>
#include <stdio.h>

int main() {

    
  //mask_set1
   
  //maskz_set1
    
  //setzero
    
  //setr
    
    
  //setr4
    
  //set4  
  
  //set1  
    
    
  //set - with broadcast  
  //e.g. _mm_set_pi8
    
  //set
  //  Initialization of vectors with 8 floats each
  __m256 data  = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0,  11.0, 13.0, 15.0);
  float* f = (float*)&data;  
  printf("%f %f %f %f %f %f %f %f\n",
            f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);

  
  
  
  
  return 0;
}
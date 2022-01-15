//
//  cmp_pd.c
//
//  Right now shows how to use _mm256_cmp_pd only for basic example
//
//
//  gcc -std=c11 -mavx -O3 -o cmp_pd.app cmp_pd.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define N_BYTES_SIMD 4

int main() {
        
    clock_t clock_begin;
    clock_t clock_end;
    
    __m256d a = _mm256_set1_pd(1);
    
    //Set to NaN
    __m256d b = _mm256_set1_pd(0.0/0.0);
    
    //https://stackoverflow.com/questions/8627331/what-does-ordered-unordered-comparison-mean
    //https://www.felixcloutier.com/x86/cmppd
    
    //EQ_UQ => equal, unordered (NaNs are true, non-signaling)
    //----------------------------------------------------------
    __m256d r1 = _mm256_cmp_pd(a, b, _CMP_EQ_UQ);  
    
    printf("_CMP_EQ_UQ ----------\n");
    printf("1,NaN,_CMP_EQ_UQ int: %d\n",_mm256_movemask_pd(r1)); //15 1,1,1,1
    
    a = _mm256_set1_pd(1);
    b = _mm256_set1_pd(1);
    r1 = _mm256_cmp_pd(a, b, _CMP_EQ_UQ); 
    printf("1,1,_CMP_EQ_UQ int: %d\n",_mm256_movemask_pd(r1));
    
    
    
    //---------------------------------------
    
    printf("_CMP_UNORD_Q ----------\n");
    a = _mm256_set1_pd(1);
    b = _mm256_set1_pd(0);
    r1 = _mm256_cmp_pd(a, b, _CMP_UNORD_Q); 
    printf("1,0,_CMP_UNORD_Q int: %d\n",_mm256_movemask_pd(r1)); //0
    r1 = _mm256_cmp_pd(b, a, _CMP_UNORD_Q); //flip order b,a
    printf("0,1,_CMP_UNORD_Q int: %d\n",_mm256_movemask_pd(r1)); //0
            
      
    a = _mm256_set1_pd(1);
    b = _mm256_set1_pd(0.0/0.0);
    
    r1 = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    printf("1,NaN,_CMP_UNORD_Q int: %d\n",_mm256_movemask_pd(r1));
    
    

}
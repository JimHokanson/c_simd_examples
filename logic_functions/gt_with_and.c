//
//  max_01.c
//
//  Horizontal max of double using _mm256_max_pd

//  gcc -std=c11 -mavx -O3 -o gt_with_and.app gt_with_and.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define N_BYTES_SIMD 4

int main() {
    
    //data[i] < threshold && data[i+1] >= threshold

    //Double - about the same time for sparse data
    //Single - noticeably faster ...
    
    clock_t clock_begin;
    clock_t clock_end;
    float *data = calloc(1e7+16,sizeof(float));
    
    double time_spent_std = 0;
    double time_spent_simd= 0;
    
    int count = 0;
    int count2 = 0;
    
    for (int i = 0; i <1e7; i+=500){
        //Joined together ...
        *(data+i+1) = 1;
        *(data+i+2) = 2;
    }
    
    for (int k=0; k <100; k++){
    
    //float threshold[8] = {1,1,1,1,1,1,1,1};
    float threshold2 = 1;
    
    
    
    //__m256d m0 = _mm256_set1_pd(threshold2);
    __m256 m0 = _mm256_loadu_ps(threshold);
    
    //__m256d for double ...
    __m256 m1;
    __m256 m2;
    __m256 r1;
    __m256 r2;
    __m256 r3;
    
    int r4;
    clock_begin = clock();
    for (int i = 0; i <1e7; i+=8){
        m1 = _mm256_loadu_ps(data+i);
        m2 = _mm256_loadu_ps(data+i+1);
        
        r1 = _mm256_cmp_ps(m1, m0, 17);
        r2 = _mm256_cmp_ps(m2, m0, 29);
        r3 = _mm256_and_ps(r1,r2);
        r4 = _mm256_movemask_ps(r3);
        count += _mm_popcnt_u32(r4);
    }
    clock_end = clock();
    time_spent_simd = time_spent_simd + (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

    clock_begin = clock();
    
    for (int i = 0; i <1e7; i++){
        if (data[i+1] >= threshold2 && data[i] < threshold2){
            count2 = count2 + 1;
        }
    }
    clock_end = clock();
    
    
    time_spent_std = time_spent_std + (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    }
    
    printf("count: %d\n",count);
    printf("count2: %d\n",count2);
    printf("t1: %g\n",time_spent_simd);
    printf("t2: %g\n",time_spent_std);
    
    
    //0 1 2 0 
    //
    
    free(data);
}
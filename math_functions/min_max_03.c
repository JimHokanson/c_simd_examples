//
//  max_04.c
//
//  Right now shows how to use _mm256_cmp_pd only for basic example
//
//  Results
//  -------
//  NaN for the 2nd input always shows up at the output
//
//
//  gcc -std=c11 -mavx -O3 -o min_max_03.app min_max_03.c

#include <immintrin.h>
#include <stdio.h> //printing
#include <math.h>
//#include <time.h>
//#include <stdint.h>

#define N_BYTES_SIMD 4

int main() {
        
    double na =  NAN;
    
    
    double d1[4] = {na,2,na,0.4};
    double d2[4] = {-INFINITY,-INFINITY,na,-INFINITY};
    
    double d3[4] = {na,2,na,0.4};
    double d4[4] = {INFINITY,INFINITY,na,INFINITY};
    
    double dr[4];
    __m256d result;
    __m256d a;
    __m256d b;
    
    a = _mm256_loadu_pd(d1);
    b = _mm256_loadu_pd(d2);

    result = _mm256_max_pd(a,b);
    
    _mm256_storeu_pd(dr,result); 
            
    //Not good, the 0.4 is overloaded with na
    //TODO: break out
    printf("na  v -Inf: %g\n",dr[0]);
    printf(" 2  v -Inf: %g\n",dr[1]);
    printf("na  v  na: %g\n",dr[2]);
    printf("0.4 v -Inf: %g\n",dr[3]);
    
    
      
    a = _mm256_loadu_pd(d3);
    b = _mm256_loadu_pd(d4);

    result = _mm256_min_pd(a,b);
    
    _mm256_storeu_pd(dr,result); 
    
    
    printf("na  v Inf: %g\n",dr[0]);
    printf(" 2  v Inf: %g\n",dr[1]);
    printf("na  v  na: %g\n",dr[2]);
    printf("0.4 v Inf: %g\n",dr[3]);
    
}
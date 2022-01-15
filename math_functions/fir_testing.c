//
//  max_01.c
//
//  

//  gcc -std=c11 -mavx -mfma  -O3 -ffast-math -fopenmp -o fir_testing.app fir_testing.c
//  gcc -std=c11 -O3 -o fir_testing.app fir_testing.c
/*
  x = rand(1,1e7);
  [b,a] = cheby1(7,3,2000/100000);
 tic
 for i = 1:10
    y = filter(b,1,x);
 end
 toc/10
 
 %How much slower when 'a' is used?
 %about 2x as long
 tic
 for i = 1:10
    y = filter(b,a,x);
 end
 toc/10
 
 tic
 for i = 1:10
    y = sl.array.mex_filter(b,a,x);
 end
 toc/10
 */

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define N_BYTES_SIMD 4

int main() {
    
    //data[i] < threshold && data[i+1] >= threshold

    //Double - about the same time for sparse data
    //Single - noticeably faster ...
    
    int n_samples = 1e8;
    clock_t clock_begin;
    clock_t clock_end;
    double *x = calloc(n_samples,sizeof(double));
    double *y = calloc(n_samples,sizeof(double));
    int filter_length = 8;
    //double c[8] = {1,2,3,4,5,6,7,8};
    double c[8] = {0.123,0.234,0.345,0.456,0.567,0.678,0.789,0.890};
    
    double time_spent_std = 0;
    double time_spent_std2 = 0;
    double time_spent_simd = 0;
    double time_spent_simd2 = 0;
    
//     int count = 0;
//     int count2 = 0;
//     
    for (int i = 0; i <n_samples; i++){
        x[i] = i;
    }
//     
//     for (int k=0; k <100; k++){
//     
//     //float threshold[8] = {1,1,1,1,1,1,1,1};
//     float threshold2 = 1;
//     
//     
//     
//     __m256d m0 = _mm256_set1_pd(threshold2);
//     //__m256d m0 = _mm256_loadu_ps(threshold);
//     
//     __m256d m1;
//     __m256d m2;
//     __m256d r1;
//     __m256d r2;
//     __m256d r3;
//     
//     int r4;
//     clock_begin = clock();
//     for (int i = 0; i <1e7; i+=8){
//         m1 = _mm256_loadu_ps(data+i);
//         m2 = _mm256_loadu_ps(data+i+1);
//         
//         r1 = _mm256_cmp_ps(m1, m0, 17);
//         r2 = _mm256_cmp_ps(m2, m0, 29);
//         r3 = _mm256_and_ps(r1,r2);
//         r4 = _mm256_movemask_ps(r3);
//         count += _mm_popcnt_u32(r4);
//     }
//     clock_end = clock();
//     time_spent_simd = time_spent_simd + (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    
    
    __m256 x0,x1,x2,x3,x4,x5,x6,x7;
    
    
    __m256 c0,c1,c2,c3,c4,c5,c6,c7;
    
    int k;    
    for (k=0; k <10; k++){
        
    __m256 y0;
    __m256 c0 = _mm256_set1_pd(c[0]);
    __m256 c1 = _mm256_set1_pd(c[1]);
    __m256 c2 = _mm256_set1_pd(c[2]);
    __m256 c3 = _mm256_set1_pd(c[3]);
    __m256 c4 = _mm256_set1_pd(c[4]);
    __m256 c5 = _mm256_set1_pd(c[5]);
    __m256 c6 = _mm256_set1_pd(c[6]);
    
    __m256 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
    __m256 f0, f1, f2, f3, f4, f5, f6, f7;
    __m256 r0, r1;
    
    __m256 x03, x47, x12, x36, x21, x25, x30, x14, x41, x52, x63, x74;
    
    __m256 x0,x1,x2,x3,x4,x5,x6,x7;
        
    //Before doing SIMD, this was just seeing if we could iterate over 
    //the filter length and update each y
    clock_begin = clock();
    for (int i = 0; i < filter_length; i++){
        for (int j = filter_length; j < n_samples; j+=4){
            y[j] += c[i]*x[j-i];
        }
    }
    clock_end = clock();
    //time_spent_simd += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

    
    //_mm256_mul_pd   3-5,0.5
    //_mm256_fmadd_pd 4-5,0.5
    //_mm256_add_pd     3,1
    clock_begin = clock();
    for (int j = filter_length; j < n_samples; j+=4) {
        
        //7 6 5 4 3 2 1 0 -1 -2 -3 -4 -5 -6 -7
        //              0  0  0  0. 0  0  0  0  x03 x47
        //            1 1  1  1. 1  1  1  1     x12 x36
        //          2 2 2  2. 2  2  2  2        x21 x25
        //        3 3 3 3. 3  3  3  3           x30 x14
        //      4 4 4 4.4  4  4  4              x41 x03 <= overlap with first entry   
        //    5 5 5 5.5 5  5  5                 x52 x12
        //  6 6 6 6.6 6 6  6                    x63 x21
        //7 7 7 7.7 7 7 7                       x74 x30
        
        
        //To collapse we need
        //     7  6  5  4   3  2  1  0 <= index of final assignment
        //c0 * 7  6  5  4,  3  2  1  0 x74,x30
        //c1 * 6  5  4  3,  2  1  0 -1 x63,x21   t2,t3
        //c2 * 5  4  3  2,  1  0 -1 -2 x52 x12
        //c3 * 4  3  2  1,  0 -1 -2 -3 x41 x03   t6,t7
        //c4 * 3  2  1  0, -1 -2 -3 -4 x30,x14
        //c5 * 2  1  0 -1, -2 -3 -4 -5 x21,x25   t10,t11
        //c6 * 1  0 -1 -2, -3 -4 -5 -6 x12,x36
        //c7 * 0 -1 -2 -3, -4 -5 -6 -7 x03,x47   t14,t15
        
        r1 = _mm256_add_pd(f2,f3);
        
        
       	_mm256_storeu_pd(&y[j+3], r1);
        
        y0 = _mm256_set1_pd(0);
        x0 = _mm256_loadu_pd(&x[j+0]);
        y0 = _mm256_fmadd_pd(c0,x0,y0); 
        x1 = _mm256_loadu_pd(&x[j-1]);
        y0 = _mm256_fmadd_pd(c1,x1,y0);
        x2 = _mm256_loadu_pd(&x[j-2]);
        y0 = _mm256_fmadd_pd(c2,x2,y0);
        x3 = _mm256_loadu_pd(&x[j-3]);
        y0 = _mm256_fmadd_pd(c3,x3,y0);
        x4 = _mm256_loadu_pd(&x[j-4]);
        y0 = _mm256_fmadd_pd(c4,x4,y0);
        x5 = _mm256_loadu_pd(&x[j-5]);
        y0 = _mm256_fmadd_pd(c5,x5,y0);
        x6 = _mm256_loadu_pd(&x[j-6]);
        y0 = _mm256_fmadd_pd(c6,x6,y0);
        x7 = _mm256_loadu_pd(&x[j-7]); 
        y0 = _mm256_fmadd_pd(c7,x7,y0);
        
       	_mm256_storeu_pd(&y[j], y0);
    }
    
    clock_end = clock();
    time_spent_simd += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
        
    clock_begin = clock();
    for (int j = filter_length; j < n_samples; j+=8) {
        
        //7 6 5 4 3 2 1 0 -1 -2 -3 -4 -5 -6 -7
        //              0  0  0  0. 0  0  0  0  x03 x47
        //            1 1  1  1. 1  1  1  1     x12 x36
        //          2 2 2  2. 2  2  2  2        x21 x25
        //        3 3 3 3. 3  3  3  3           x30 x14
        //      4 4 4 4.4  4  4  4              x41 x03 <= overlap with first entry   
        //    5 5 5 5.5 5  5  5                 x52 x12
        //  6 6 6 6.6 6 6  6                    x63 x21
        //7 7 7 7.7 7 7 7                       x74 x30
        
        
        //To collapse we need
        //     7  6  5  4   3  2  1  0 <= index of final assignment
        //c0 * 7  6  5  4,  3  2  1  0 x74,x30
        //c1 * 6  5  4  3,  2  1  0 -1 x63,x21   t2,t3
        //c2 * 5  4  3  2,  1  0 -1 -2 x52 x12
        //c3 * 4  3  2  1,  0 -1 -2 -3 x41 x03   t6,t7
        //c4 * 3  2  1  0, -1 -2 -3 -4 x30,x14
        //c5 * 2  1  0 -1, -2 -3 -4 -5 x21,x25   t10,t11
        //c6 * 1  0 -1 -2, -3 -4 -5 -6 x12,x36
        //c7 * 0 -1 -2 -3, -4 -5 -6 -7 x03,x47   t14,t15
        
        x74 = _mm256_loadu_pd(&x[j+4]);
        x63 = _mm256_loadu_pd(&x[j+3]);
        x52 = _mm256_loadu_pd(&x[j+2]);
        x41 = _mm256_loadu_pd(&x[j+1]);
        x30 = _mm256_loadu_pd(&x[j+0]);
        x21 = _mm256_loadu_pd(&x[j-1]);
        x12 = _mm256_loadu_pd(&x[j-2]);
        x03 = _mm256_loadu_pd(&x[j-3]);
        x14 = _mm256_loadu_pd(&x[j-4]);
        x25 = _mm256_loadu_pd(&x[j-5]);
        x36 = _mm256_loadu_pd(&x[j-6]);
        x47 = _mm256_loadu_pd(&x[j-7]);
        
        t0 = _mm256_mul_pd(c0,x74);
        t1 = _mm256_mul_pd(c0,x30);
        t2 = _mm256_fmadd_pd(c1,x63,t0);
        t3 = _mm256_fmadd_pd(c1,x21,t1);
        
        t4 = _mm256_mul_pd(c2,x52);
        t5 = _mm256_mul_pd(c2,x12);
        t6 = _mm256_fmadd_pd(c3,x41,t4);
        t7 = _mm256_fmadd_pd(c3,x03,t5);
        
        t8 = _mm256_mul_pd(c4,x30);
        t9 = _mm256_mul_pd(c4,x14);
        t10 = _mm256_fmadd_pd(c5,x21,t8);
        t11 = _mm256_fmadd_pd(c5,x25,t9);
        
        t12 = _mm256_mul_pd(c6,x12);
        t13 = _mm256_mul_pd(c6,x36);
        t14 = _mm256_fmadd_pd(c7,x03,t12);
        t15 = _mm256_fmadd_pd(c7,x47,t13);
        
        f0 = _mm256_add_pd(t3,t7);
        f1 = _mm256_add_pd(t11,t15);
        
        r0 = _mm256_add_pd(f0,f1);
                
       	_mm256_storeu_pd(&y[j], r0);
        
        f2 = _mm256_add_pd(t2,t6);
        f3 = _mm256_add_pd(t10,t14);
        
        r1 = _mm256_add_pd(f2,f3);
                
       	_mm256_storeu_pd(&y[j+3], r1);
    }
    
    clock_end = clock();
    time_spent_simd2 += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    
    
// //         m1 = _mm256_loadu_ps(data+i);
// //         
// //         //0:-7
// //         //1:-6
// //         //2:-5
// //         //3:-4
// //         //4:-3
// //         //5:-2
// //         //6:-1
// //         //7:0
// //         m1 = _mm256_loadu_pd(&x[j);
// //         //Need 4 values shifted by various amounts ...
// //         y[j] = c[0]*x[j] + c[1]*x[j-1] + c[2]*x[j-2] + c[3]*x[j-3] +
// //                 + c[4]*x[j-4] + c[5]*x[j-5] + c[6]*x[j-6] + c[7]*x[j-7];
// //     }    
        
        
    
    clock_begin = clock();
    //This is the worst case approach - 66 ms   
    //---------------------------------------------
    for (int j = filter_length; j < n_samples; j++) {
        int s = 0; // s = accumulator
        for (int i =0; i < filter_length; i++){
            s += c[i] * x[j-i]; // x[] = input values
        }
     	y[j] = s; // y[] = output values
    }
    clock_end = clock();
    time_spent_std += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    
    
    
    //Way faster ... 18 ms
    //-----------------------------------------------
    clock_begin = clock();
    for (int j = filter_length; j < n_samples; j++) {
        y[j] = c[0]*x[j] + c[1]*x[j-1] + c[2]*x[j-2] + c[3]*x[j-3] +
                + c[4]*x[j-4] + c[5]*x[j-5] + c[6]*x[j-6] + c[7]*x[j-7];
    }
    clock_end = clock();
    time_spent_std2 += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

    printf("t1: %g\n",time_spent_simd);
    printf("t2: %g\n",time_spent_simd2);
    printf("t3: %g\n",time_spent_std);
    printf("t4: %g\n",time_spent_std2);
    
    
    }
    
//     printf("count: %d\n",count);
//     printf("count2: %d\n",count2);
//     printf("t1: %g\n",time_spent_simd);
    //printf("t2: %g\n",time_spent_std);
    //This seems to be necessary to keep the loop
    printf("t1: %g\n",time_spent_simd/k);
    printf("t2: %g\n",time_spent_simd2/k);
    printf("t3: %g\n",time_spent_std/k);
    printf("t4: %g\n",time_spent_std2/k);
    printf("y: %g\n",y[50]);
    printf("y: %g\n",y[3000000]);
    
    
    //0 1 2 0 
    //
    
    free(x);
    free(y);
}
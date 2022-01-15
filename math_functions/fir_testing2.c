//
//  max_01.c
//
//  

//  /usr/local/bin/gcc-8 -fopenmp -std=c11 -march=native  -O3 -ffast-math -o fir_testing2.app fir_testing2.c
//  ./fir_testing2.app
//  gcc -std=c11 -mavx -mfma  -O3 -ffast-math -o fir_testing2.app fir_testing2.c
//  gcc -std=c11 -march=native  -O3 -ffast-math -o fir_testing2.app fir_testing2.c
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

#include <omp.h>
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
    double c[8] = {0.123,0.234,0.345,0.456,0.567,0.678,0.789,0.890};
    
    double time_spent_std = 0;
    
    for (int i = 0; i <n_samples; i++){
        x[i] = i;
    }
    
    clock_begin = clock();
    
//664 - just as slow as simple loop    
if (0){   
    __m256d y0;
    __m256d c0 = _mm256_set1_pd(c[0]);
    __m256d c1 = _mm256_set1_pd(c[1]);
    __m256d c2 = _mm256_set1_pd(c[2]);
    __m256d c3 = _mm256_set1_pd(c[3]);
    __m256d c4 = _mm256_set1_pd(c[4]);
    __m256d c5 = _mm256_set1_pd(c[5]);
    __m256d c6 = _mm256_set1_pd(c[6]);
    __m256d c7 = _mm256_set1_pd(c[7]);
    
    __m256d t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
    __m256d f0, f1, f2, f3, f4, f5, f6, f7;
    __m256d r0, r1;
    
    __m256d x03, x47, x12, x36, x21, x25, x30, x14, x41, x52, x63, x74;
    
    __m256d x0,x1,x2,x3,x4,x5,x6,x7; 
    
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
}
    
    
//573    
if (0){
    __m256d x0,x1,x2,x3,x4,x5,x6,x7; //0 - 0 back
    __m256d c0 = _mm256_set1_pd(c[0]);
    __m256d c1 = _mm256_set1_pd(c[1]);
    __m256d c2 = _mm256_set1_pd(c[2]);
    __m256d c3 = _mm256_set1_pd(c[3]);
    __m256d c4 = _mm256_set1_pd(c[4]);
    __m256d c5 = _mm256_set1_pd(c[5]);
    __m256d c6 = _mm256_set1_pd(c[6]);
    __m256d c7 = _mm256_set1_pd(c[6]);
    
    __m256d y0, y1, y2, y3, y4, y5, y6, y7;

                                          
    // 17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0  (round 1)
    //                                                   3  2  1  0 <= 0 back
    //                                       6  5  4  3             <= 1 back
    //                         9   8   7  6                         <= 2 back
    // 
    //
    
    // 17  16  15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0  (round 2)                                     
    //                                       7  6  5  4  <= 0 back
    //                         10  9   8  7
    

    //sprintf("stop: %d\n",stops[99]);
    
    //omp_set_num_threads(1);
    
    //#pragma omp parallel for simd
    
        int cur_start = 0;
        y1 = _mm256_loadu_pd(x);     //0:3 => becomes y0 on first run
        y2 = _mm256_loadu_pd(x+4);   //4:7 => becomes y1 on first run
        y3 = _mm256_loadu_pd(x+8);
        y4 = _mm256_loadu_pd(x+12);
        y5 = _mm256_loadu_pd(x+16);
        y6 = _mm256_loadu_pd(x+20);
        y7 = _mm256_loadu_pd(x+24);
    
    for (int j = 28; j < n_samples; j+=4){
    

            //TODO: Can we avoid the awkward loads????
            //Perhaps store temporary variables ...

            //Can we do the shift in the fma stage????

            y0 = y1;
            y1 = y2;
            y2 = y3;
            y3 = y4;
            y4 = y5;
            y5 = y6;
            y6 = y7;

            x0 = _mm256_loadu_pd(&x[j-28-0]);
            x1 = _mm256_loadu_pd(&x[j-24-1]);
            x2 = _mm256_loadu_pd(&x[j-20-2]);
            x3 = _mm256_loadu_pd(&x[j-16-3]);
            x4 = _mm256_loadu_pd(&x[j-12-4]);
            x5 = _mm256_loadu_pd(&x[j-8-5]);
            x6 = _mm256_loadu_pd(&x[j-4-6]);
            x7 = _mm256_loadu_pd(&x[j-0-7]);

            y0 = _mm256_add_pd(c0,x0);
            _mm256_storeu_pd(&y[j-28], y0);
            y1 = _mm256_fmadd_pd(c1,x1,y1);
            y2 = _mm256_fmadd_pd(c2,x2,y2);
            y3 = _mm256_fmadd_pd(c3,x3,y3);
            y4 = _mm256_fmadd_pd(c4,x4,y4);
            y5 = _mm256_fmadd_pd(c5,x5,y5);
            y6 = _mm256_fmadd_pd(c6,x6,y6);
            y7 = _mm256_fmadd_pd(c7,x7,y7);
        }
    
    //TODO: We still have a little remaining at the end to handle ...
}

//653    
if (1){
    //Standard loop approach ----------------------------------------------
    #pragma omp parallel for simd
    for (int j = filter_length; j < n_samples; j++) {
        y[j] = c[0]*x[j] + c[1]*x[j-1] + c[2]*x[j-2] + c[3]*x[j-3] +
                + c[4]*x[j-4] + c[5]*x[j-5] + c[6]*x[j-6] + c[7]*x[j-7];
    }
}
    
    
    
    clock_end = clock();
    time_spent_std += (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

    printf("time (ms): %g\n",1000*time_spent_std);
    printf("y_last: %g\n",y[n_samples-1]);
}
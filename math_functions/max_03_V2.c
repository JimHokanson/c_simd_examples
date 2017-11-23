//
//  max_03_V2.c
//
//  This is the same as max_03.c but using 3 comparisons per loop instead
//  of just 2.
//
//  Further speedup is observed:
//  max_03_V2: 14.5x
//  max_03:    13.5x
//
//  Improvements
//  ------------
//  1) Use SIMD for extras. Since this processes 48 values in a loop, we
//  might have a size of the data that doesn't fit in the loop but that is
//  amenable to SIMD. For example, if we have 47 extra values, we should 
//  collapse the first 36 using SIMD, and only handle the last 15 manually.
//  2) The same comment as #1 goes for if we have small sample sizes (less
//  than 48 values total)
//  

//  Mac:
//  gcc -std=c11 -mavx2 -o max_03_V2.app max_03_V2.c
//  gcc -std=c11 -mavx2 -O3 -o max_03_V2.app max_03_V2.c
//  ./max_03_V2.app

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define N_VALUES_PER_LOOP 48
#define N_VALUES_PER_SIMD 16

int main() {
    
    
    //Inputs
    //---------------------------
    size_t n_extra_in = 13;
    size_t N = 1e8 + n_extra_in;
    int n_loops = 10;
    
    //Intermediate variables
    //----------------------
    //
    //This is redundant but normally we would only have n, not n_extra_in
    size_t n_extra = N - (N/N_VALUES_PER_LOOP)*N_VALUES_PER_LOOP;
    uint16_t output[N_VALUES_PER_SIMD];
    uint16_t *data;
    uint16_t *data_move;
    uint16_t *extra_data;
    uint16_t max_value = 0;
    clock_t clock_begin;
    clock_t clock_end;
    clock_t clock_end2;
    uint16_t temp_value;
    __m256i next1;
    __m256i result1;
    __m256i next2;
    __m256i result2;
    __m256i next3;
    __m256i result3;
    double time_spent_std;
    double time_spent_simd;
    double speed_up = 0;
    
    
    //Data initialization
    //--------------------------
    data = (uint16_t *)calloc(N,sizeof(uint16_t));
    memset(data,0,N*sizeof(uint16_t));
    if (!data){
        printf("Memory Allocation Failure\n");
        return 0;
    }

    //This is somewhat arbitrary
    for (int i = 1000; i < 1000+N_VALUES_PER_LOOP; i++){
        data[i] = 30000+i;
    }

    //Trying to do more comparisons
    temp_value = 0;
    for (int i = 0; i < 25000*10; i+=10){
        temp_value += 1;
        data[i] = temp_value;
    }
    
    
    for (int k = 0; k < n_loops; k++){
        
        //Standard approach
        //=========================================================
        clock_begin = clock();
        max_value = data[0];
        data_move = data;
        ++data_move;
        for (size_t j = 1; j < N; j++){
            if (*data_move > max_value){
                max_value = *data_move;
            }
            ++data_move;
        }
        clock_end = clock();
        time_spent_std = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
        printf(" STD: time spent: %0.3f, max value = %d\n",time_spent_std,max_value);
        
    
        //Two at a time
        //==================================================
        clock_begin = clock();

        extra_data = data + (N - n_extra);

        if (N > N_VALUES_PER_LOOP){

            //Start with first 16 values
            result1  = _mm256_lddqu_si256((__m256i *)data);
            result2  = _mm256_lddqu_si256((__m256i *)data+N_VALUES_PER_SIMD);
            result3  = _mm256_lddqu_si256((__m256i *)data+2*N_VALUES_PER_SIMD);

            //a   b  c  d
            //  x   
            //     x
            //        x

            //Might be better to have:
            //a  b  c  d  e  f
            //  x  x    x  x 
            //    x   x

            //Compare 1st 16 to next 16, compare result to next 16, etc.
            for (int j = N_VALUES_PER_LOOP; j < (N-N_VALUES_PER_LOOP); j+=N_VALUES_PER_LOOP){
                next1 = _mm256_lddqu_si256((__m256i *)(data+j));
                result1 = _mm256_max_epu16(result1, next1);
             	next2 = _mm256_lddqu_si256((__m256i *)(data+j+N_VALUES_PER_SIMD));
                result2 = _mm256_max_epu16(result2, next2);
                next3 = _mm256_lddqu_si256((__m256i *)(data+j+ 2*N_VALUES_PER_SIMD));
                result3 = _mm256_max_epu16(result3, next3);
            }

            
            result1 = _mm256_max_epu16(result1, result2);
            result1 = _mm256_max_epu16(result1, result3);
            
            //Extract max values and reduce ...
            _mm256_storeu_si256((__m256i*)output, result1);
            //_mm256_storeu_si256((__m256i*)output2, result2);

            //Debug
            //------------
            //for (int i = 0; i < 16; i++){
              //printf("max value: %d\n",output[i]);  
            //}

            //Note we've reduced extras to just 16 by using am extra
            //16 v 16 call
            max_value = output[0];
            for (int i = 1; i < N_VALUES_PER_SIMD; i++){
                if (output[i] > max_value){
                    max_value = output[i];
                } 
            } 
        }else if (N == 0){
            max_value = 0;
        }

        //At this point we have:
        //- max_value
        //- extra_data
        //- n_extra

        clock_end = clock();

        for (size_t i = 0; i < n_extra; i++){
            if (*extra_data > max_value){
                max_value = *extra_data;
            }
            ++extra_data;
        }

        time_spent_simd = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
        printf("SIMD: time spent: %0.3f, max value = %d\n",time_spent_simd,max_value);

        speed_up = (time_spent_std/time_spent_simd) + speed_up;
        
    }
    
    speed_up = speed_up/n_loops;
    
    printf("Speedup %0.6f\n",speed_up);
    
    return 0;
}
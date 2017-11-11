//
//  max_v1.c
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
//  cd /Users/jameshokanson/repos/c_simd_examples/math_functions
//  gcc -std=c11 -mavx2 -o max_v2 max_v2.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

int main() {
    
    
    //Inputs
    //------
    size_t n_extra_in = 13;
    size_t N = 1e8 + n_extra_in;
    
    //Intermediate variables
    //----------------------
    //
    //This is redundant but normally we would only have n, not n_extra_in
    size_t n_extra = N - (N/16)*16;
    uint16_t output[16];
    uint16_t *data;
    uint16_t *data_move;
    uint16_t *extra_data;
    uint16_t max_value = 0;
    clock_t clock_begin;
    clock_t clock_end;
    __m256i next;
    __m256i result;
    
    data = (uint16_t *)calloc(N,sizeof(uint16_t));
    if (!data){
        printf("Memory Allocation Failure\n");
        return 0;
    }
    
    //This is somewhat arbitrary
    for (int i = 1000; i < 1000+16; i++){
        data[i] = 30000+i;
    }
    
    //Trying to do more comparisons
    uint16_t temp_value;
    for (int i = 0; i < 25000*10; i+=10){
        temp_value += 1;
        data[i] = temp_value;
    }
    
    
    for (int k = 0; k < 10; k++){
    
    //The main algorithm
    //------------------------------
    clock_begin = clock();
    
        
    extra_data = data + (N - n_extra);
        
    if (N == 0){
        return 0;
    }else if (N > 16){
    
        //Start with first 16 values
        result  = _mm256_lddqu_si256((__m256i *)data);
        
        //a   b  c  d
        //  x   
        //     x
        //        x
        
        //Might be better to have:
        //a  b  c  d  e  f
        //  x  x    x  x 
        //    x   x
        
        //Compare 1st 16 to next 16, compare result to next 16, etc.
        for (int j = 16; j < (N-16); j+=16){
            next = _mm256_lddqu_si256((__m256i *)(data+j));
            //Requires AVX2
            result = _mm256_max_epu16(result, next);
        }

        //Extract max values and reduce ...
        _mm256_storeu_si256((__m256i*)output, result);

        //Debug
        //------------
        //for (int i = 0; i < 16; i++){
          //printf("max value: %d\n",output[i]);  
        //}
        
        max_value = output[0];
        for (int i = 1; i < 16; i++){
            if (output[i] > max_value){
                max_value = output[i];
            } 
        } 
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

    
    
    printf("max value = %d\n",max_value);
    
    double time_spent = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    printf("SIMD: time spent: %0.6f\n",time_spent);
    
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
    printf("max value = %d\n",max_value);
    time_spent = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
    printf("Standard: time spent: %0.6f\n",time_spent);
    }
    
    free(data);
    
    return 0;
}
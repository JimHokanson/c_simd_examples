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
//  gcc -mavx2 -o max_v2 max_v2.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

int main() {
    
    
    //Inputs
    //------
    size_t n_extra_in = 13;
    size_t N = 1e7 + n_extra_in;
    
    //Intermediate variables
    //----------------------
    //
    //This is redundant but normally we would only have n, not n_extra_in
     n_extra;
    uint16_t output[16];
    uint16_t *data;
    uint16_t max_value = 0;
    data = (uint16_t *)calloc(N,sizeof(uint16_t));
    if (!data){
        printf("Failure\n");
        return 0;
    }
    
    //This is somewhat arbitrary
    for (int i = 1000; i < 1000+16; i++){
        data[i] = 30000+i
    }
    
    //The main algorithm
    //------------------------------
    clock_t begin = clock();
    __m256i next;
    
    
    size_t n_extra = N - N/16;
    uint16_t *extra_data = data + (N - n_extra);
    
    if (N == 0){
        return 0;
    }else if (N > 16)
    
        //Start with first 16 values
        __m256i result = _mm256_lddqu_si256((__m256i *)data);

        //Compare 1st 16 to next 16, compare result to next 16, etc.
        for (int j = 16; j < (N-16); j+=16){
            next = _mm256_lddqu_si256((__m256i *)(data+j));
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
    
    //TODO: Any leftovers????
    
    
    clock_t end = clock();
    
    for (int i = 0; i < 16; i++){
        printf("max value: %d\n",output[i]);
    }
    
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spent: %0.6f\n",time_spent);
    
    
    
    
    
    begin = clock();
    uint16_t max_value = data[0];
    ++d;
    for (int j = 1; j < N; j++){
        if (*data > max_value){
            max_value = *data;
        }
        ++d;
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spent: %0.6f\n",time_spent);
    
    free(data);
    
    return 0;
}
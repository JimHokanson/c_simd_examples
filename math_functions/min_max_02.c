//
//  min_max_02.c
//
//  Results
//  --------
//
//  Horizontal min and max of uint16 using:
//  - _mm256_max_epu16
//  - _mm256_min_epu16

//  gcc -std=c11 -mavx2 -o min_max_02.app min_max_02.c
//  gcc -std=c11 -mavx2 -O3 -o min_max_02.app min_max_02.c
//  ./min_max_02.app

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define N_BYTES_SIMD 16

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
    
    size_t n_extra = N - (N/N_BYTES_SIMD)*N_BYTES_SIMD;
    uint16_t max_output[N_BYTES_SIMD];
    uint16_t min_output[N_BYTES_SIMD];
    uint16_t *data;
    uint16_t *data_move;
    uint16_t *extra_data;
    uint16_t max_value = 0;
    uint16_t min_value = 0;
    clock_t clock_begin;
    clock_t clock_end;
    clock_t clock_end2;
    uint16_t temp_value;
    
    
    __m256i next;
    __m256i max_result;
    __m256i min_result;
    
    double time_spent_std;
    double time_spent_simd;
    double speed_up = 0;
    
    
    
    
    for (int k = 0; k < n_loops; k++){
        
        //Data initialization    
        //==================================================    
        data = (uint16_t *)calloc(N,sizeof(uint16_t));

        if (!data){
            printf("Memory Allocation Failure\n");
            return 0;
        }

        for (size_t i = 0; i < N; i++){
            data[i] = 0;
        }

        //This is somewhat arbitrary
        for (int i = 1000; i < 1000+16; i++){
            data[i] = 30000+i;
        }
        
        //This is somewhat arbitrary
        for (int i = 2000; i < 2000+16; i++){
            data[i] = -30000+i;
        }

        //Trying to do more comparisons
        temp_value = 0;
        for (int i = 0; i < 25000*10; i+=10){
            temp_value += 1;
            data[i] = temp_value;
        }
        //==================================================
    
    
        //Standard approach
        //=========================================================
        clock_begin = clock();
        max_value = data[0];
        min_value = data[0];
        data_move = data;
        ++data_move;
        for (size_t j = 1; j < N; j++){
            if (*data_move > max_value){
                max_value = *data_move;
            }else if (*data_move < min_value){
                min_value = *data_move;
            }
            ++data_move;
        }
        clock_end = clock();
        time_spent_std = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
        printf("Standard: time spent: %0.6f, max value = %d, min_value = %d\n",
                time_spent_std,max_value,min_value);
        
        free(data);
        //=========================================================
    
        //Data reinitialization
        //==================================================    
        data = (uint16_t *)calloc(N,sizeof(uint16_t));
        if (!data){
            printf("Memory Allocation Failure\n");
            return 0;
        }

        for (size_t i = 0; i < N; i++){
            data[i] = 0;
        }

         //This is somewhat arbitrary
        for (int i = 1000; i < 1000+16; i++){
            data[i] = 30000+i;
        }
        
        //This is somewhat arbitrary
        for (int i = 2000; i < 2000+16; i++){
            data[i] = -30000+i;
        }

        //Trying to do more comparisons
        temp_value = 0;
        for (int i = 0; i < 25000*10; i+=10){
            temp_value += 1;
            data[i] = temp_value;
        }
        //==================================================
    
    
        
        //The main algorithm
        //==================================================
        min_value = data[0];
        max_value = data[0];
        clock_begin = clock();

        extra_data = data + (N - n_extra);

        if (N > N_BYTES_SIMD){

            //Start with first 16 values
            max_result = _mm256_lddqu_si256((__m256i *)data);
            min_result = max_result;

            //a   b  c  d
            //  x   
            //     x
            //        x

            //Might be better to have:
            //a  b  c  d  e  f
            //  x  x    x  x 
            //    x   x

            //Compare 1st 16 to next 16, compare result to next 16, etc.
            //
            //TODO: The < N - N_BYTES_SIMD is not correct
            //  
            for (int j = N_BYTES_SIMD; j < (N-N_BYTES_SIMD); j+=N_BYTES_SIMD){
                next = _mm256_lddqu_si256((__m256i *)(data+j));
                //Requires AVX
                max_result = _mm256_max_epu16(max_result, next);
                min_result = _mm256_min_epu16(min_result, next);
            }

            //Extract max values and reduce ...
            _mm256_storeu_si256((__m256i*)max_output, max_result);
            _mm256_storeu_si256((__m256i*)min_output, min_result);

            //Debug
            //------------
            //for (int i = 0; i < 16; i++){
              //printf("max value: %d\n",output[i]);  
            //}

            //TODO: Still need to process any extra samples ...
            
            max_value = max_output[0];
            for (int i = 1; i < N_BYTES_SIMD; i++){
                if (max_output[i] > max_value){
                    max_value = max_output[i];
                } 
            } 
            min_value = min_output[0];
            for (int i = 1; i < N_BYTES_SIMD; i++){
                if (min_output[i] < min_value){
                    min_value = min_output[i];
                } 
            } 
        }else if (N == 0){
            //Technically these should be something else ...
            max_value = 0;
            min_value = 0;
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
        printf("SIMD: time spent: %0.6f, max value = %d, min_value = %d\n",
                time_spent_simd,max_value,min_value);

        speed_up = (time_spent_std/time_spent_simd) + speed_up;
        
        free(data);
    }
    
    speed_up = speed_up/n_loops;
    
    printf("Speedup %0.6f\n",speed_up);
    
    return 0;
}
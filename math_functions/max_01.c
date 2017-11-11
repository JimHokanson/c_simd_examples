//
//  max_01.c
//
//  Horizontal max of double using _mm256_max_pd

//  gcc -std=c11 -mavx -o max_01.app max_01.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define N_BYTES_SIMD 4

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
    double output[N_BYTES_SIMD];
    double *data;
    double *data_move;
    double *extra_data;
    double max_value = 0;
    clock_t clock_begin;
    clock_t clock_end;
    clock_t clock_end2;
    double temp_value;
    
    
    __m256d next;
    __m256d result;
    
    double time_spent_std;
    double time_spent_simd;
    double speed_up = 0;
    
    
    
    
    for (int k = 0; k < n_loops; k++){
        
        //Data initialization    
        //==================================================    
        data = (double *)calloc(N,sizeof(double));

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
        printf("Standard: time spent: %0.6f, max value = %g\n",time_spent_std,max_value);
        
        free(data);
        //=========================================================
    
        //Data reinitialization
        //==================================================    
        data = (double *)calloc(N,sizeof(double));
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

        //Trying to do more comparisons
        temp_value = 0;
        for (int i = 0; i < 25000*10; i+=10){
            temp_value += 1;
            data[i] = temp_value;
        }
        //==================================================
    
    
        
        //The main algorithm
        //==================================================
        clock_begin = clock();

        extra_data = data + (N - n_extra);

        if (N > N_BYTES_SIMD){

            //Start with first 16 values
            result  = _mm256_loadu_pd(data);

            //a   b  c  d
            //  x   
            //     x
            //        x

            //Might be better to have:
            //a  b  c  d  e  f
            //  x  x    x  x 
            //    x   x

            //Compare 1st 16 to next 16, compare result to next 16, etc.
            for (int j = N_BYTES_SIMD; j < (N-N_BYTES_SIMD); j+=N_BYTES_SIMD){
                next = _mm256_loadu_pd((data+j));
                //Requires AVX
                result = _mm256_max_pd(result, next);
            }

            //Extract max values and reduce ...
            _mm256_storeu_pd(output, result);

            //Debug
            //------------
            //for (int i = 0; i < 16; i++){
              //printf("max value: %d\n",output[i]);  
            //}

            max_value = output[0];
            for (int i = 1; i < N_BYTES_SIMD; i++){
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
        printf("SIMD: time spent: %0.6f, max value = %g\n",time_spent_simd,max_value);

        speed_up = (time_spent_std/time_spent_simd) + speed_up;
        
        free(data);
    }
    
    speed_up = speed_up/n_loops;
    
    printf("Speedup %0.6f\n",speed_up);
    
    return 0;
}
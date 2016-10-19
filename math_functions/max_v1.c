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
//  gcc -mavx -o max_v1 max_v1.c

#include <immintrin.h>
#include <stdio.h>
#include <time.h>

int main() {
    
    //TODO: Not sure what the best or worst cases would be, depending on the data
    //double data[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    double data[16] = {1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0};
    
    //TODO: Move this example off of the stack into a large dynamically allocated array
    
    clock_t begin = clock();
    
    __m256d d1,d2,d3,d4,d5,d6,d7;
    
    double *ff;
    double max1;
    double max2;
    double max_value;
    //Jim's laptop 1e8 => 2.75 seconds (without final max)
    for (int i = 0; i < 1e8; i++){
        //Run time drops dramatically for this, not sure of what
        //optimization is being performed
        
        //The execution time drops dramatically without this line ...
        data[0] = i; //Hopefully this forces running everything
        d1 = _mm256_loadu_pd(&data[0]);
        d2 = _mm256_loadu_pd(&data[4]);
        d3 = _mm256_loadu_pd(&data[8]);
        d4 = _mm256_loadu_pd(&data[12]);

        // Different comparison stategies
        //  Approach 1
        //  ----------
        //  A   B   C   D
        //    1       2
        //        3
        //
        //  Approach 2 - this approach seems not to be as good ...
        //  ----------
        //  A   B   C   D  <= this seems to not be as good
        //    1   
        //        2
        //           3
        //
        //  Expanding to higher counts - # equals # of sets of 4 minus 1
        //  A B C D E  (5 sets, 4 comparisons)
        //  1(AB) 2(CD)
        //    3(12)
        //       4(3E)
        //
        //  ??? How to generalize for each level - hardcode for some level
        //  
        //  Yes, it seems like we hardcode out to some level and then reuse
        //  as necessary ...
        //  TODO: Also need to examine loading aligned vs unaligned memory
        // 

        //Approach 1
        //--------------
        d5 = _mm256_max_pd(d1,d2);
        d6 = _mm256_max_pd(d3,d4);
        d7 = _mm256_max_pd(d5,d6);
        
        //Just this adds on an extra 1.25 seconds?!?!?!?
        //3.52 seconds - so 0.75 seconds to collapse
        ff = (double *)&d7;
        max_value = *ff;
        if (*(++ff) > max_value){
            max_value = *ff;
        }else if (*(++ff) > max_value){
            max_value = *ff;
        }else if (*(++ff) > max_value){
            max_value = *ff;
        }
        
        //TODO: Try this ...
        //http://stackoverflow.com/questions/9795529/how-to-find-the-horizontal-maximum-in-a-256-bit-avx-vector        
    }
    
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spent: %0.2f\n",time_spent);
    printf("max value: %f\n",max_value);
    
    
    //6.73 seconds
    begin = clock();
    for (int i = 0; i < 1e8; i++){
        ff = &data[0];
        max_value = *ff;
        for (int j = 1; j < 16; j++){
            if (*(++ff) > max_value){
                max_value = *ff;
            }
        }
        
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
    printf("time spent: %0.2f\n",time_spent);
    printf("max value: %f\n",max_value);    

      
  return 0;
}
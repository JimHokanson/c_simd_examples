//
//  string_02.c
//
//  Task
//  ----
//  Count the # of occurences of a single character in a string ...
//
//  Simd calls
//  ----------
//
//  
//  Status
//  ------
//  Complete although code is a bit messy
//

//  gcc -std=c11 -mavx2 -mlzcnt -O3 -o string_02.app string_02.c
//  ./string_02.app



// alias gcc=/usr/local/Cellar/gcc/6.3.0_1/bin/gcc-6
//
// and back to apple gcc:
//
// alias gcc=/usr/bin/gcc


//Intersting (not for this file, just in general):
//https://www.strchr.com/strcmp_and_strlen_using_sse_4.2

#include <immintrin.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <x86intrin.h>



int main() {
    
    
    //_mm256_cmpeq_epi8
    
    //Strings:
    //- every nth char is set
    //
    
    
    
    size_t N = 2e8;
    bool found = false; //Not really used ...
    volatile size_t count_simd = 0;
    volatile size_t count_man1 = 0;
    volatile size_t count_man2 = 0;
    
    
    char * str = malloc(N);
    char * s;
    char * str2;
    memset(str,'a',N);
    clock_t clock_begin;
    clock_t clock_end;
    
    __m256i char_match;
    __m256i str_simd; 
    __m256i result;
    __m256i* pSrc1;
    
    int simd_mask;
    
    double time_spent_man1;
    double time_spent_man2;
    double time_spent_simd;
    double speed_up1 = 0;
    double speed_up2 = 0;
    
    //Ideally this would be named better
    int n_loops_outer = 1;
    int n_loops_inner = 1;
    size_t char_position;
    
    
    for (size_t ch = 2; ch < 70; ch+=6){
        memset(str,'a',N);
        size_t count1 = 0;
        for (size_t i = 0; i < N; i+=ch){
            count1++;
            str[i] = 'b';
        }
        
        speed_up2 = 0;
        
        for (int k = 0; k < n_loops_outer; k++){
            
            
            
            //=================================================================
            //SIMD
            clock_begin = clock();
            char_match = _mm256_set1_epi8('b');
            //result = _mm256_set1_epi32(0);
            for (int n2 = 0; n2 < n_loops_inner; n2++){
                pSrc1 = (__m256i *)str;
                count_simd = 0;
                for (size_t i = 0; i < N-32; i+=32){
                    //TODO: I think we can use a different load instruction
                    //- not sure what the difference is
                    str_simd = _mm256_lddqu_si256(pSrc1);
                    result = _mm256_cmpeq_epi8(str_simd, char_match);
                    simd_mask = _mm256_movemask_epi8(result);
                    count_simd += _mm_popcnt_u32(simd_mask);
                    pSrc1++;
                }
                //Remainder
                //------------------
                size_t start_man;
                if (N < 32){
                    start_man = 0;
                } else {
                    start_man = N - 32;
                }
                for (size_t i = start_man; i < N; i++){
                    count_simd += str[i] == 'b';
                }
            }
            clock_end = clock();
            time_spent_simd = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
            //=================================================================
            
            //=========================================
            //Manual search - always add 0 or 1
            clock_begin = clock();
            for (int n2 = 0; n2 < n_loops_inner; n2++){
                count_man2 = 0;
                for (size_t i = 0; i < N; i++){
                    count_man2 += str[i] == 'b';
                }
            }
            //=========================================
            clock_end = clock();
            time_spent_man2 = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;

            //=========================================
            //Manual search - if-statement on adding
            clock_begin = clock();
            for (int n2 = 0; n2 < n_loops_inner; n2++){
                count_man1 = 0;
                for (size_t i = 0; i < N; i++){
                    if (str[i] == 'b'){
                       count_man1++; 
                    }
                }
            }
            //=========================================
            clock_end = clock();
            time_spent_man1 = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
            
            printf("Match at every: %zu\n",ch);
            printf("STD1: %0.3f, count: %zu\n",time_spent_man1,count_man1);
            printf("STD2: %0.3f, count: %zu\n",time_spent_man2,count_man2);
            printf("SIMD: %0.3f, count: %zu\n",time_spent_simd,count_simd);
            
            speed_up2 += time_spent_man2/time_spent_simd;
            
        }
        
        speed_up2 = speed_up2/(double)n_loops_outer;
        printf("Avg speed: %0.2f\n",speed_up2); 
    }
    return 0;
}
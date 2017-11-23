//
//  string_01.c
//
//  Status: Incomplete, requires final processing if the character is at
//  the very end of the string.
//

//  gcc -std=c11 -mavx2 -mlzcnt -O3 -o string_01.app string_01.c
//  ./string_01.app



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
    
    //To test:
    //   - find first instance of character
    //   - vary length of the placement of the character
    //   - test known and unknown string lengths
    
    //_mm256_cmpeq_epi8
    
    size_t N = 1e9;
    bool found = false; //Not really used ...
    volatile size_t char_index1 = 0;
    volatile size_t char_index2 = 0;
    volatile size_t char_index3 = 0;
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
    
    double time_spent_std1;
    double time_spent_std2;
    double time_spent_simd;
    double speed_up = 0;
    double speed_up2 = 0;
    
    //Ideally this would be named better
    int n_loops_outer = 1;
    int n_loops_inner = 10000000;
    size_t char_position;
    
    //Old setup
    //-----------
    //str[(size_t)5e8+1] = 'b';
    //n_loops_inner = 10;
    
    for (size_t ch = 2; ch < 50; ch+=2){
        memset(str,'a',N);
        char_position = ch;
        speed_up2 = 0;
        
        for (int k = 0; k < n_loops_outer; k++){
            
            
            
            //=================================================================
            //SIMD
            clock_begin = clock();
            char_match = _mm256_set1_epi8('b');
            result = _mm256_set1_epi32(0);
            for (int n2 = 0; n2 < n_loops_inner; n2++){
                
                //------------------------
                //__m256i _mm256_cmpeq_epi8 (__m256i a, __m256i b)
                
                str[(size_t)char_position] = 'b';
                
                pSrc1 = (__m256i *)str;
                
                for (size_t i = 0; i < N-32; i+=32){
                    str_simd = _mm256_lddqu_si256(pSrc1);
                    result = _mm256_cmpeq_epi8(str_simd, char_match);
                    simd_mask = _mm256_movemask_epi8(result);
                    if (simd_mask != 0){
                        break;
                    }
                    pSrc1++;
                }
                if (simd_mask == 0){
                    //TODO: Handle remainder
                }
                
                
                //sign of lzcnt32?
                s = (char *)pSrc1 + (31 - __lzcnt32(simd_mask));
                char_index3  = s - str;
                
                
                
                //=================================================================
            }
            clock_end = clock();
            time_spent_simd = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
            
            
            //=========================================
            //strchr
            clock_begin = clock();
            for (size_t n2 = 0; n2 < n_loops_inner; n2++){
                str[(size_t)char_position] = 'b';
                str2 = memchr(str,'b',N);
                char_index2 = str2 - str;
            }
            clock_end = clock();
            time_spent_std2 = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
            //=========================================
            
            //=========================================
            //Manual search
            clock_begin = clock();
            for (int n2 = 0; n2 < n_loops_inner; n2++){
                str[(size_t)char_position] = 'b';
                
                s = str;
                while (*s != 'b'){
                    ++s;
                }
                char_index1 = s - str;
            }
            //=========================================
            clock_end = clock();
            time_spent_std1 = (double)(clock_end - clock_begin) / CLOCKS_PER_SEC;
            
            printf("At ch position: %zu\n",char_position);
            printf("STD1: %0.3f, I: %zu\n",time_spent_std1,char_index1);
            printf("STD2: %0.3f, I: %zu\n",time_spent_std2,char_index2);
            printf("SIMD: %0.3f, I: %zu\n",time_spent_simd,char_index3);
            printf("strchr vs naive: %0.2fv\n",time_spent_std1/time_spent_std2);
            printf(" SIMD vs memchr: %0.2fv\n",time_spent_std2/time_spent_simd);
            
            speed_up2 += time_spent_std2/time_spent_simd;
            
        }
        
        speed_up2 = speed_up2/(double)n_loops_outer;
        printf("Avg speed: %0.2f\n",speed_up2);
        
    }
    
    
    
    return 0;
}
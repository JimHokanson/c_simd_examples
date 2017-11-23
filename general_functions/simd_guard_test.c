
//  gcc -std=c11 -mavx2 -o simd_guard_test.app simd_guard_test.c
//  ./simd_guard_test.app

#include "simd_guard.h"
#include <stdio.h>

int main() {

    struct cpu_x86 s;
    
    cpu_x86__detect_host(&s);
    printf("Support for:\n");
    printf("AVX: %d\n",s.HW_AVX);
    printf("AVX2: %d\n",s.HW_AVX2);
    printf("FMA3: %d\n",s.HW_FMA3);
    printf("FMA4: %d\n",s.HW_FMA4);
    printf("OS_AVX: %d\n",s.OS_AVX);
    return 0;
}
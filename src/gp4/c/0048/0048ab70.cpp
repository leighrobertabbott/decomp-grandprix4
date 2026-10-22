#include <gp4/gp4.h>

// Looks up a float in the table at 0x00a4d5e8: the index selects a slot via 0x006444d0, the
// slot gives a base via 0x00644294 and the element is slot * 5 + base. Returned in st0.
// FUNCTION: GP4 0x0048ab70
double __stdcall FUN_0048ab70(int index) {
    const int* slotOf = (const int*)GP4_ARRAY(int, 0x006444d0);
    const int* base = (const int*)GP4_ARRAY(int, 0x00644294);
    const float* table = (const float*)GP4_ARRAY(float, 0x00a4d5e8);
    int slot = slotOf[index];
    return table[slot * 5 + base[slot]];
}
GP4_IMPL(0x0048ab70, FUN_0048ab70,
         "stdcall(i32[0..3]) -> f64; gfields={0x006444d0*4/4:u32[0..3], 0x00644294*4/4:u32[0..4], 0x00a4d5e8*20/4:f32}")

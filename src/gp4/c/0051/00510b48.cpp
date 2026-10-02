#include <gp4/gp4.h>

// FUNCTION: GP4 0x00510b48
// Sets flag 0x02 in byte +0x70 of entry `index` of the 0x160-byte record table at 0x8943a0.
void __stdcall Record8943a0_SetFlag02(int index) {
    uint8_t* rec = GP4_ARRAY(uint8_t, 0x008943a0) + index * 0x160;
    rec[0x70] |= 0x02;
}
GP4_IMPL(0x00510b48, Record8943a0_SetFlag02, "stdcall(i32[0..21]) -> void")
// shape fix retest

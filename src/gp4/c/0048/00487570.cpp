#include <gp4/gp4.h>

// Stores the float argument at +0x3f4 of record `index` (records are 0x2a0 bytes apart).
// FUNCTION: GP4 0x00487570
void __fastcall FUN_00487570(uint8_t* self, void* /*edx*/, int index, float value) {
    *(float*)(self + index * 0x2a0 + 0x3f4) = value;
}
GP4_IMPL(0x00487570, FUN_00487570, "thiscall(ptr[0x1000]:bytes, i32[0..3], f32) -> void")

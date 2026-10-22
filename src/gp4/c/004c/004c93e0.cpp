#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Triple_4c93e0 { uint32_t A; uint32_t B; uint32_t C; };   // 12-byte element
#pragma pack(pop)

// Stores the 12-byte element *src into each of count consecutive 12-byte slots starting at dst
// (a null slot is skipped while the pointer still advances).
// FUN_004c93e0
// FUNCTION: GP4 0x004c93e0
void __stdcall FUN_004c93e0(Triple_4c93e0* dst, uint32_t count, const Triple_4c93e0* src) {
    for (; count != 0; --count) {
        if (dst != nullptr) {
            dst->A = src->A;
            dst->B = src->B;
            dst->C = src->C;
        }
        ++dst;
    }
}
GP4_IMPL(0x004c93e0, FUN_004c93e0, "stdcall(ptr[0x100]:bytes, u32[0..12], ptr[12]:bytes) -> void")

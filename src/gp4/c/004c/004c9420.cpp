#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Triple_4c9420 { uint32_t A; uint32_t B; uint32_t C; };   // 12-byte element
#pragma pack(pop)

// Copy-constructs the 12-byte element at dst from *src (a null dst is skipped) and returns dst.
// FUNCTION: GP4 0x004c9420
Triple_4c9420* __cdecl FUN_004c9420(Triple_4c9420* dst, const Triple_4c9420* src) {
    if (dst != nullptr) {
        dst->A = src->A;
        dst->B = src->B;
        dst->C = src->C;
    }
    return dst;
}
GP4_IMPL(0x004c9420, FUN_004c9420, "cdecl(nptr[12]:bytes, ptr[12]:bytes) -> u32")

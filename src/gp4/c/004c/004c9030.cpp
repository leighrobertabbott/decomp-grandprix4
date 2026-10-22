#include <gp4/gp4.h>

#pragma pack(push, 1)
// Vector of 12-byte elements: begin pointer at +4, end pointer at +8.
struct Unknown_004c9030 { uint8_t Unknown00[4]; uint32_t Begin04; uint32_t End08; };
#pragma pack(pop)

// Element count of a vector of 12-byte items: (end - begin) / 12, or 0 when begin is null.
// FUNCTION: GP4 0x004c9030
int32_t __fastcall FUN_004c9030(Unknown_004c9030* self, void* /*edx*/) {
    if (self->Begin04 == 0) {
        return 0;
    }
    return (int32_t)(self->End08 - self->Begin04) / 12;
}
GP4_IMPL(0x004c9030, FUN_004c9030, "thiscall(ptr[0x10]:{4:u32{0,0x1000,0x2000,0x7fff0000}, 8:u32}) -> i32")

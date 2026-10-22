#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb830 {
    uint8_t Unknown00[0x1c];
    int32_t Limit1c;
    int32_t Position20;
};
#pragma pack(pop)

// Moves the position at +0x20 by `delta` and clamps it to 0 .. Limit1c (minus 4 when `reserve`
// is set).
// FUNCTION: GP4 0x005cb830
void __fastcall FUN_005cb830(Unknown_005cb830* self, void* /*edx*/, int delta, char reserve) {
    int margin = 0;
    if (reserve != 0) {
        margin = 4;
    }
    int position = self->Position20 + delta;
    self->Position20 = position;
    if (position < 0) {
        self->Position20 = 0;
        return;
    }
    int limit = self->Limit1c - margin;
    if (position > limit) {
        self->Position20 = limit;
    }
}
GP4_IMPL(0x005cb830, FUN_005cb830, "thiscall(ptr[0x30]:{0x1c:i32[-10..1000], 0x20:i32[-100..1100]}, i32[-300..300], u8{0,1,0x80}) -> void")

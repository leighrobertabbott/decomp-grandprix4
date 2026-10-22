#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a9d50 { uint8_t Unknown00[0x14]; int32_t* Table14; };
#pragma pack(pop)

// Clears entry `index` (a sign-extended 16-bit value) of the dword table at +0x14; returns 0.
// FUNCTION: GP4 0x005a9d50
int32_t __fastcall FUN_005a9d50(Unknown_005a9d50* self, void* /*edx*/, int16_t index) {
    self->Table14[index] = 0;
    return 0;
}
GP4_IMPL(0x005a9d50, FUN_005a9d50, "thiscall(ptr[0x40]:{0x14:ptr[0x100]:bytes}, i16[0..63]) -> i32")

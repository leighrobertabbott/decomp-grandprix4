#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bcd00 { uint8_t Unknown00[0x10]; int32_t Slots10[1]; };
#pragma pack(pop)

// Returns slot `index` of the dword table at +0x10 of the object (`ret 4`); index 0 yields 0.
// FUNCTION: GP4 0x005bcd00
int32_t __fastcall FUN_005bcd00(const Unknown_005bcd00* self, void* /*edx*/, int32_t index) {
    if (index == 0) {
        return 0;
    }
    return self->Slots10[index];
}
GP4_IMPL(0x005bcd00, FUN_005bcd00, "thiscall(ptr[0x100]:bytes, i32[0..0x38]) -> i32")

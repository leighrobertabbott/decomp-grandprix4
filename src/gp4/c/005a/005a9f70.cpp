#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Entry_005a9f70 { int32_t First; int32_t Second; float Value; };
struct Unknown_005a9f70 { uint8_t Unknown00[0x160]; Entry_005a9f70 Entries160[1]; };
#pragma pack(pop)

// Fills the 12-byte entry `index` (a signed byte; values >= 1 are refused with 1) of the table at +0x160:
// First = a, Second = b, Value = the float c copied through the x87 stack. Returns 0 on success.
// FUNCTION: GP4 0x005a9f70
int32_t __fastcall FUN_005a9f70(Unknown_005a9f70* self, void* /*edx*/, int32_t index, int32_t a, int32_t b, float c) {
    const int8_t slot = static_cast<int8_t>(index);
    if (slot >= 1) {
        return 1;
    }
    Entry_005a9f70* entry = self->Entries160 + slot;
    entry->Value = gp4::x87::narrow_f32(gp4::x87::ext_f32(c));
    entry->First = a;
    entry->Second = b;
    return 0;
}
GP4_IMPL(0x005a9f70, FUN_005a9f70, "thiscall(ptr[0x200]:bytes, u32{=0,1,2,3,0x7f,0xf0,0xec,0xff,0xffffffff,0x100,0x1ff}, i32, i32, i32) -> i32")

#include <gp4/gp4.h>
#include <gp4/x87.h>

#pragma pack(push, 1)
struct Unknown_005a9fb0_Entry { int32_t First00; int32_t Second04; float Third08; };
struct Unknown_005a9fb0 {
    uint8_t Unknown00[0x184];
    Unknown_005a9fb0_Entry Entries184[1];
};
#pragma pack(pop)

// Stores the three value arguments into the 12-byte entry selected by the (signed byte) index at +0x184
// of the object; the third one goes through the x87 stack (fld dword / fstp dword). An index of 1 or
// more is refused and returns 1, otherwise returns 0.
// FUNCTION: GP4 0x005a9fb0
int32_t __fastcall FUN_005a9fb0(Unknown_005a9fb0* self, void* /*edx*/, int8_t index, int32_t first, int32_t second, float third) {
    using namespace gp4::x87;
    if (index >= 1) {
        return 1;
    }
    Unknown_005a9fb0_Entry* entry = &self->Entries184[index];
    entry->Third08 = narrow_f32(ext_f32(third));
    entry->First00 = first;
    entry->Second04 = second;
    return 0;
}
GP4_IMPL(0x005a9fb0, FUN_005a9fb0, "thiscall(ptr[0x200]:bytes, i32[-3..3], i32, i32, u32{0x7fa00000,0x3f800000,0xff800001,0x80000000}) -> i32")

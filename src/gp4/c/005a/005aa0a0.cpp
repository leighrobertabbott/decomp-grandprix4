#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005aa0a0_Entry { int32_t First00; int32_t Second04; int32_t Third08; };
struct Unknown_005aa0a0 {
    uint8_t Unknown00[0x184];
    Unknown_005aa0a0_Entry Entries184[1];
};
#pragma pack(pop)

// Reads the 12-byte entry selected by the (signed byte) index from the table at +0x184 and copies its three
// dwords to the three output pointers; an index of 1 or more is refused and returns 1, otherwise returns 0.
// FUNCTION: GP4 0x005aa0a0
int32_t __fastcall FUN_005aa0a0(Unknown_005aa0a0* self, void* /*edx*/, int8_t index, int32_t* first, int32_t* second, int32_t* third) {
    if (index >= 1) {
        return 1;
    }
    const Unknown_005aa0a0_Entry* entry = &self->Entries184[index];
    *first = entry->First00;
    *second = entry->Second04;
    *third = entry->Third08;
    return 0;
}
GP4_IMPL(0x005aa0a0, FUN_005aa0a0, "thiscall(ptr[0x300]:bytes, i32[-3..3], ptr[4]:bytes, ptr[4]:bytes, ptr[4]:bytes) -> i32")

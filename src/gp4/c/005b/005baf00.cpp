#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Table_005baf00 { uint8_t Unknown00[8]; void* Slot08[4]; };
#pragma pack(pop)

// Looks up the pointer in slot `index` of the table at +8 and, when it is non-null, runs the
// list-clearing routine 0x005babd0 on it (`ret 4`).
// FUNCTION: GP4 0x005baf00
void __fastcall FUN_005baf00(Table_005baf00* self, void* /*edx*/, int32_t index) {
    void* entry = *(void**)((uint8_t*)self + 8 + index * 4);
    if (entry != 0) {
        gp4::thiscall_<0x005babd0, void>(entry);
    }
}
GP4_IMPL(0x005baf00, FUN_005baf00, "thiscall(ptr[0x18]:{8*4/4:nptr[0x20]:{0x10:nptr[0x10]:{0:nptr[0x10]:{0:nptr[0x10]:{0:u32{=0}}}}}}, i32[0..3]) -> void")

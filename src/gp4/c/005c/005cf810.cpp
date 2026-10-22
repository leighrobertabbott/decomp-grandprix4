#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cf810 {
    uint8_t Unknown00[0x10];
    void* Object10;
    void* Object14;
};
#pragma pack(pop)

// Releases the two owned sub-objects at +0x10 and +0x14: each non-null one is run through the
// destructor thunk 0x005d1dc0, freed with 0x00446a60 and its pointer cleared.
// FUNCTION: GP4 0x005cf810
void __fastcall FUN_005cf810(Unknown_005cf810* self, void* /*edx*/) {
    void* object = self->Object10;
    if (object != 0) {
        gp4::thiscall_<0x005d1dc0, void>(object);
        gp4::cdecl_<0x00446a60, int>(object);
    }
    object = self->Object14;
    self->Object10 = 0;
    if (object != 0) {
        gp4::thiscall_<0x005d1dc0, void>(object);
        gp4::cdecl_<0x00446a60, int>(object);
    }
    self->Object14 = 0;
}
GP4_IMPL(0x005cf810, FUN_005cf810, "thiscall(ptr[0x20]:{0x10:nptr[0x20]:{0xc:nptr[0x180]+4:{0:u32[0..3]}}, 0x14:nptr[0x20]:{0xc:nptr[0x180]+4:{0:u32[0..3]}}}) -> void")

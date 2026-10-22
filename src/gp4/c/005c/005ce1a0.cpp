#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ce1a0 {
    const uint32_t* Vptr00;
    uint8_t Unknown04[0xd0 - 4];
    void* Children0d0[4];
};
#pragma pack(pop)

// Destructor body: resets the vtable pointer to 0x005fa6b0, then for each of the four child objects at
// +0xd0..+0xdc releases its array (0x005d1dd0), runs the same release through the thunk 0x005d1dc0, frees
// the child with 0x00446a60 and clears the slot.
// FUNCTION: GP4 0x005ce1a0
void __fastcall FUN_005ce1a0(Unknown_005ce1a0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005fa6b0);
    void** slot = self->Children0d0;
    int32_t remaining = 4;
    do {
        if (*slot != 0) {
            gp4::thiscall_<0x005d1dd0, void>(*slot);
            void* child = *slot;
            if (child != 0) {
                gp4::thiscall_<0x005d1dc0, void>(child);
                gp4::cdecl_<0x00446a60, int>(child);
            }
            *slot = 0;
        }
        slot++;
        remaining--;
    } while (remaining != 0);
}
GP4_IMPL(0x005ce1a0, FUN_005ce1a0, "thiscall(ptr[0xf0]:{0xd0*4/4:nptr[0x20]:{0xc:nptr[0x180]+4:{0:u32[0..3]}}}) -> void")

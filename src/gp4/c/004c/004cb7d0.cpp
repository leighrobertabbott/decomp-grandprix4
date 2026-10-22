#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004cb7d0 {
    uint32_t Vptr00;
    uint8_t Unknown04[0x17d0];
    uint32_t Listener17d4;
};
#pragma pack(pop)

// Constructor: runs the base constructor 0x004cad80, installs the vtable 0x005f0d78 and stores the
// argument (the listener object) at +0x17d4; returns this.
// FUNCTION: GP4 0x004cb7d0
Unknown_004cb7d0* __fastcall FUN_004cb7d0(Unknown_004cb7d0* self, void* /*edx*/, uint32_t listener) {
    gp4::thiscall_<0x004cad80, int>(self);
    self->Vptr00 = 0x005f0d78;
    self->Listener17d4 = listener;
    return self;
}
GP4_IMPL(0x004cb7d0, FUN_004cb7d0, "thiscall(ptr[0x17e0]:bytes, u32) -> u32")

#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004caaf0 {
    uint32_t Vptr00;
    uint32_t Field04;
};
#pragma pack(pop)

// Constructor body: stores the vtable pointer 0x005f0cec, calls the empty routine 0x005ae990,
// clears the dword at +4 and returns this.
// FUNCTION: GP4 0x004caaf0
Unknown_004caaf0* __fastcall FUN_004caaf0(Unknown_004caaf0* self, void* /*edx*/) {
    self->Vptr00 = 0x005f0cec;
    gp4::cdecl_<0x005ae990, void>();
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x004caaf0, FUN_004caaf0, "thiscall(ptr[8]:bytes) -> u32")

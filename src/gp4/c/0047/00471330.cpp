#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00471330 {
    uint8_t Unknown00[8];
    void* File08;  // CRT FILE closed through 0x005dc52c
};
#pragma pack(pop)

// Closes the CRT FILE at +8 (0x005dc52c) when set and clears the pointer.
// FUNCTION: GP4 0x00471330
void __fastcall FUN_00471330(Unknown_00471330* self, void* /*edx*/) {
    if (self->File08 != nullptr) {
        gp4::cdecl_<0x005dc52c, int>(self->File08);
    }
    self->File08 = nullptr;
}
GP4_IMPL(0x00471330, FUN_00471330, "thiscall(ptr[0x20]:{8:nptr[0x20]:layout=iostrg_0046b580}) -> void")

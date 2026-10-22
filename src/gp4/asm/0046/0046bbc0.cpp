#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046bbc0 { uint32_t Unknown00; uint32_t Words04[10]; };
#pragma pack(pop)

// Copies ten dwords (+4 .. +0x2b, rep movsd) from the stack argument to the object and returns the
// object (eax = ecx); the first dword is left alone.
// FUNCTION: GP4 0x0046bbc0
Unknown_0046bbc0* __fastcall FUN_0046bbc0(Unknown_0046bbc0* self, void* /*edx*/, const Unknown_0046bbc0* src) {
    for (int i = 0; i < 10; i++) {
        self->Words04[i] = src->Words04[i];
    }
    return self;
}
GP4_IMPL(0x0046bbc0, FUN_0046bbc0, "thiscall(ptr[0x40]:bytes, ptr[0x40]:bytes) -> u32")

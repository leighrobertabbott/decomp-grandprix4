#include <gp4/gp4.h>

// Source record in esi: the three angle dwords at +0x0c/+0x10/+0x14.
struct Unknown_0051eea7_Esi {
    uint8_t Unknown00[0x0c];
    uint32_t Angle0C;
    uint32_t Angle10;
    uint32_t Angle14;
};

// Loads the angle dwords of the record in esi into the block at 0x00854194 (the +0x10 dword first,
// so +0/+4/+8 = record +0x10/+0x0c/+0x14), then runs the angle conversion (0x0051edd5), the sin/cos
// pairs (0x0051edf9) and the rotation matrix (0x0051ee1b) on that block. edx = block, eax = last
// dword. Same code as 0x00401f8e.
// FUNCTION: GP4 0x0051eea7
void __cdecl FUN_0051eea7(gp4::Regs* r) {
    const Unknown_0051eea7_Esi* src = reinterpret_cast<const Unknown_0051eea7_Esi*>(static_cast<uintptr_t>(r->esi));
    uint32_t* block = GP4_ARRAY(uint32_t, 0x00854194);

    block[0] = src->Angle10;
    block[1] = src->Angle0C;
    const uint32_t last = src->Angle14;
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = 0x00854194u;
    gp4::call_regs(0x0051edd5, state);
    gp4::call_regs(0x0051edf9, state);
    gp4::call_regs(0x0051ee1b, state);

    r->eax = last;
    r->edx = 0x00854194u;
}
GP4_IMPL(0x0051eea7, FUN_0051eea7, "regs(esi:ptr[0x18]:bytes) -> (eax, edx)")

#include <gp4/gp4.h>

// Source record in esi: the three angle dwords at +0x0c/+0x10/+0x14.
struct Unknown_00401f8e_Esi {
    uint8_t Unknown00[0x0c];
    uint32_t Angle0C;
    uint32_t Angle10;
    uint32_t Angle14;
};

// Loads the angle dwords of the record in esi into the block at 0x00622dc8 (the +0x10 dword first,
// so +0/+4/+8 = record +0x10/+0x0c/+0x14), then runs the angle conversion (0x00401d5c), the sin/cos
// pairs (0x00401d80) and the rotation matrix (0x00401da2) on that block. edx = block, eax = last dword.
// FUNCTION: GP4 0x00401f8e
void __cdecl FUN_00401f8e(gp4::Regs* r) {
    const Unknown_00401f8e_Esi* src = reinterpret_cast<const Unknown_00401f8e_Esi*>(static_cast<uintptr_t>(r->esi));
    uint32_t* block = GP4_ARRAY(uint32_t, 0x00622dc8);

    block[0] = src->Angle10;
    block[1] = src->Angle0C;
    const uint32_t last = src->Angle14;
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = 0x00622dc8u;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
    r->edx = 0x00622dc8u;
}
GP4_IMPL(0x00401f8e, FUN_00401f8e, "regs(esi:ptr[0x18]:bytes) -> (eax, edx)")

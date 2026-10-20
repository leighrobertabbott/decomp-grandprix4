#include <gp4/gp4.h>

// Source record in esi: the three dwords at +0x0c/+0x10/+0x14 (orientation angles).
struct Unknown_00402225_Esi {
    uint8_t Unknown00[0x0c];
    uint32_t Angle0C;
    uint32_t Angle10;
    uint32_t Angle14;
};

// Copies the three angle dwords of the record in esi to edx+0/+4/+8, then runs the angle
// conversion (0x00401d5c), the sin/cos pairs (0x00401d80) and the rotation matrix (0x00401da2) on
// the edx block. eax keeps the last dword copied.
// FUNCTION: GP4 0x00402225
void __cdecl FUN_00402225(gp4::Regs* r) {
    const Unknown_00402225_Esi* src = reinterpret_cast<const Unknown_00402225_Esi*>(static_cast<uintptr_t>(r->esi));
    uint32_t* block = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->edx));

    block[0] = src->Angle0C;
    block[1] = src->Angle10;
    const uint32_t last = src->Angle14;
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
}
GP4_IMPL(0x00402225, FUN_00402225, "regs(edx:ptr[0xa0]:f64, esi:ptr[0x18]:bytes) -> (eax)")

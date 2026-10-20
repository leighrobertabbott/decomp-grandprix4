#include <gp4/gp4.h>

// Source record behind the pointer global 0x0060982c: the three angle dwords at +0x0c/+0x10/+0x14.
struct Unknown_00402081_Source {
    uint8_t Unknown00[0x0c];
    uint32_t Angle0C;
    uint32_t Angle10;
    uint32_t Angle14;
};

// Copies the three angle dwords of the record at [0x0060982c] into the block at 0x006231d8, then
// runs the angle conversion (0x00401d5c), the sin/cos pairs (0x00401d80) and the rotation matrix
// (0x00401da2) on that block. edx = block, edi = the record, eax = the last dword copied.
// FUNCTION: GP4 0x00402081
void __cdecl FUN_00402081(gp4::Regs* r) {
    const uint32_t sourceAddress = GP4_GLOBAL(uint32_t, 0x0060982c);
    const Unknown_00402081_Source* src = reinterpret_cast<const Unknown_00402081_Source*>(static_cast<uintptr_t>(sourceAddress));
    uint32_t* block = GP4_ARRAY(uint32_t, 0x006231d8);

    block[0] = src->Angle0C;
    block[1] = src->Angle10;
    const uint32_t last = src->Angle14;
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = 0x006231d8u;
    state.edi = sourceAddress;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
    r->edx = 0x006231d8u;
    r->edi = sourceAddress;
}
GP4_IMPL(0x00402081, FUN_00402081, "regs() -> (eax, edx, edi); globals=layout=view_00609860")

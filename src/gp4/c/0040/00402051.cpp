#include <gp4/gp4.h>

// Source record behind the pointer global 0x0060982c: the three angle dwords at +0x0c/+0x10/+0x14.
struct Unknown_00402051_Source {
    uint8_t Unknown00[0x0c];
    uint32_t Angle0C;
    uint32_t Angle10;
    uint32_t Angle14;
};

// Record behind the pointer global 0x00609860: its pointer at +0x40 addresses the destination block.
struct Unknown_00402051_Owner {
    uint8_t Unknown00[0x40];
    uint32_t Block40;
};

// Copies the three angle dwords of the record at [0x0060982c] into the block pointed to by
// [[0x00609860] + 0x40], then runs the angle conversion (0x00401d5c), the sin/cos pairs
// (0x00401d80) and the rotation matrix (0x00401da2) on that block. edx = block, edi = the record,
// eax = the last dword copied.
// FUNCTION: GP4 0x00402051
void __cdecl FUN_00402051(gp4::Regs* r) {
    const Unknown_00402051_Owner* owner = reinterpret_cast<const Unknown_00402051_Owner*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00609860)));
    const uint32_t blockAddress = owner->Block40;
    uint32_t* block = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(blockAddress));
    const uint32_t sourceAddress = GP4_GLOBAL(uint32_t, 0x0060982c);
    const Unknown_00402051_Source* src = reinterpret_cast<const Unknown_00402051_Source*>(static_cast<uintptr_t>(sourceAddress));

    block[0] = src->Angle0C;
    block[1] = src->Angle10;
    const uint32_t last = src->Angle14;
    block[2] = last;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.eax = last;
    state.edx = blockAddress;
    state.edi = sourceAddress;
    gp4::call_regs(0x00401d5c, state);
    gp4::call_regs(0x00401d80, state);
    gp4::call_regs(0x00401da2, state);

    r->eax = last;
    r->edx = blockAddress;
    r->edi = sourceAddress;
}
GP4_IMPL(0x00402051, FUN_00402051, "regs() -> (eax, edx, edi); globals=layout=view_00609860")

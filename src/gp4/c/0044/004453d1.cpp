#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a 32-bit `or reg, reg`: CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result >> 31) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x004453d1
// Copies the pending opcode stream [0x0062d19c, 0x0062d1a0) into the outgoing packet buffer and sends it:
// [0x007a97ac] = length + 1 and the bytes go to 0x007a97b1.., then 0x0051f0ae (service 0x4b) runs.
// While the link is down ([0x007aafd8] = 0xff) it only resets the cursors (0x004453bc). pushal / popal keep
// every register; the flags are those 0x0051f0ae leaves (or the equal `cmp byte` when the link is down).
void __cdecl FUN_004453d1(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x007aafd8) == 0xffu) {
        gp4::cdecl_<0x004453bc, void>();
        r->eflags = (r->eflags & ~kArithFlags) | 0x44u;
        return;
    }

    const uint32_t start = GP4_GLOBAL(uint32_t, 0x0062d19c);
    int32_t length = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0062d1a0) - start);
    if (length < 0) {
        length = 0;
    }
    const uint32_t count = static_cast<uint32_t>(length);
    GP4_GLOBAL(uint32_t, 0x007a97ac) = count + 1u;

    const uint8_t* from = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(start));
    uint8_t* to = GP4_ARRAY(uint8_t, 0x007a97b1);
    for (uint32_t i = 0; i != count; ++i) {
        to[i] = from[i];
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = count + 1u;
    c.ecx = 0u;
    c.edx = start;
    c.esi = start + count;
    c.edi = 0x007a97b1u + count;
    c.eflags = (r->eflags & ~kArithFlags) | logicFlags32(count);
    gp4::call_regs(0x0051f0ae, c);

    r->eflags = (r->eflags & ~kArithFlags) | (c.eflags & kArithFlags);
}
GP4_IMPL(0x004453d1, FUN_004453d1, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=send_004453d1")

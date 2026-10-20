#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a 32-bit `or reg, reg` (CF = OF = AF = 0).
static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result >> 31) != 0u ? 0x80u : 0u);
}

// Flags of an 8-bit `and al, imm` (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Flags of the 8-bit `cmp byte [0x007aafd8], 0xff`.
static inline uint32_t linkCmpFlags(uint32_t value) {
    const uint32_t result = (value - 0xffu) & 0xffu;
    return (value < 0xffu ? 1u : 0u)
        | parityFlag8(result)
        | (((value ^ 0xffu ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((value ^ 0xffu) & (value ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004452a0
// Receives and dispatches the incoming opcode stream. With the link up ([0x007aafd8] != 0xff) it fetches
// the packet with 0x00445423 (which copies it to 0x0062c19a and sets the length [0x0062d1a4]; on failure
// it marks the link down), then walks the stream with the read cursor [0x0062d1a8] up to the end
// [0x0062d1ac], handing the low nibble of each opcode byte to the dispatcher 0x004452f1 (the handlers
// advance the cursor). Returns with CF clear after a full walk, CF set when the link is down or no
// bytes arrived (stc); the other flags are the last compare's.
void __cdecl FUN_004452a0(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    uint32_t flags;
    uint32_t carry;

    if (GP4_GLOBAL(uint8_t, 0x007aafd8) == 0xffu) {
        flags = linkCmpFlags(0xffu);
        carry = 1u;
    } else {
        gp4::call_regs(0x00445423, s);
        const uint32_t link = GP4_GLOBAL(uint8_t, 0x007aafd8);
        if (link == 0xffu) {
            flags = linkCmpFlags(link);
            carry = 1u;
        } else {
            s.ecx = GP4_GLOBAL(uint32_t, 0x0062d1a4);
            if (s.ecx == 0u) {
                flags = logicFlags32(0u);
                carry = 1u;
            } else {
                s.ebx = 0x0062c19au;
                GP4_GLOBAL(uint32_t, 0x0062d1a8) = s.ebx;
                s.ebx += s.ecx;
                GP4_GLOBAL(uint32_t, 0x0062d1ac) = s.ebx;
                for (;;) {
                    s.ebx = GP4_GLOBAL(uint32_t, 0x0062d1a8);
                    const uint32_t end = GP4_GLOBAL(uint32_t, 0x0062d1ac);
                    if (s.ebx >= end) {
                        flags = cmpFlags32(s.ebx, end);
                        carry = 0u;
                        break;
                    }
                    const uint32_t opcode = GP4_FIELD(uint8_t, s.ebx, 0) & 0xfu;
                    s.eax = (s.eax & 0xffffff00u) | opcode;
                    s.eflags = (s.eflags & ~kArithFlags) | logicFlags8(opcode);
                    gp4::call_regs(0x004452f1, s);
                }
            }
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | (flags & kArithFlags & ~1u) | carry;
}
GP4_IMPL(0x004452a0, FUN_004452a0, "regs(eax:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recv_00445423")
// v2: diagnostics

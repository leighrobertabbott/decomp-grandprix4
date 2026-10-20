#include <gp4/gp4.h>

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kAF = 0x10u, kZF = 0x40u, kSF = 0x80u, kOF = 0x800u
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? kPF : 0u;
}

// Flags left by a byte-wide OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? kZF : 0u) | ((result & 0x80u) != 0u ? kSF : 0u);
}

// Flags of a 32-bit cmp a, b.
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (a < b ? 1u : 0u) | ((a ^ b ^ result) & kAF) | ((parity & 1u) == 0u ? kPF : 0u) |
           (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x0040f3f6
// Marks the record in esi (flag byte +0x82 bit 0). While the dword at +0x28 is below
// 0x10000000 it also updates the flag bytes at +0x82 / +0x88 and, when the byte global
// 0x007c2ea9 is 0x80, flags a record through 0x0040eb25.
void __cdecl FUN_0040f3f6(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x01u);
    const uint32_t value = GP4_FIELD(uint32_t, rec, 0x28);
    r->eax = value;

    uint32_t flags;
    if (static_cast<int32_t>(value) >= 0x10000000) {
        flags = cmpFlags(value, 0x10000000u);
    } else {
        if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
            rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x04u);
            gp4::Regs callee = *r;
            gp4::call_regs(0x0040eb25, callee);
        }
        rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x08u);
        rec[0x82] = static_cast<uint8_t>(rec[0x82] & 0xefu);
        rec[0x88] = static_cast<uint8_t>(rec[0x88] | 0x10u);
        rec[0x82] = static_cast<uint8_t>(rec[0x82] | 0x01u);
        flags = logicFlags8(rec[0x82]);
    }
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x0040f3f6, FUN_0040f3f6, "regs(esi:ptr[0x41d]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=carno_0066ff6c")

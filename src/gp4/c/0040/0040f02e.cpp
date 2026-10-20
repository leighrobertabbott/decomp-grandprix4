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

// Flags left by a logic operation (TEST / AND) on an 8-bit-wide result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? kZF : 0u) | ((result & 0x80u) != 0u ? kSF : 0u);
}

// Flags of a 32-bit cmp a, b.
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u) | ((a ^ b ^ result) & kAF) | parityFlag8(result) |
           (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x0040f02e
// Car block in esi, partner block pointer in edi (passed through to 0x0040e9ac). Unless the
// mode byte 0x007c2ea9 has bit 7 set it marks the block (0x0040f439) and, when the dword
// +0xc0 is not the 0xc0000000 sentinel, stores (clock - [+0xc0]) - ratio into the dword table
// at +0x20 indexed by byte +0x74. The pushal/popal pairs keep every register; the flags are
// those of the last flag writer (test / cmp / and).
void __cdecl FUN_0040f02e(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = logicFlags8(mode);
    if ((mode & 0x80u) == 0u) {
        gp4::Regs mark = *r;
        gp4::call_regs(0x0040f439, mark);

        const uint32_t sentinel = GP4_FIELD(uint32_t, rec, 0xc0);
        flags = cmpFlags(sentinel, 0xc0000000u);
        if (sentinel != 0xc0000000u) {
            gp4::Regs ratio = *r;
            ratio.eax = GP4_GLOBAL(uint32_t, 0x0066d814) - sentinel;
            gp4::call_regs(0x0040e9ac, ratio);

            const uint32_t value = ratio.eax - ratio.ebx;
            const uint32_t slot = GP4_FIELD(uint8_t, rec, 0x74);
            GP4_FIELD(uint32_t, rec + slot * 4u, 0x20) = value;
            flags = logicFlags8(slot);
        }
    }
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x0040f02e, FUN_0040f02e, "regs(esi:ptr[0xc8]:layout=pair_0040ea7d, edi:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040e9ac")

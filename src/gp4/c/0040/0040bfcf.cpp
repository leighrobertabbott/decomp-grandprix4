#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// sub / cmp left, right (32-bit) arithmetic flags.
static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The two per-segment updates; out of line so each keeps its own call (the compiler would
// otherwise select the callee address and merge the two calls).
static __declspec(noinline) void updateWithoutStep(gp4::Regs& c) {
    gp4::call_regs(0x0040bec7, c);
}

static __declspec(noinline) void updateWithStep(gp4::Regs& c) {
    gp4::call_regs(0x0040bd5f, c);
}

// FUNCTION: GP4 0x0040bfcf
// Advances the track-surface warm-up by ([0x0063b818] + [0x0062282c]) >> 16 segments: the 16-bit
// remainder stays in [0x0063b818]; the segment offset [0x0063b814] walks the 0x160-byte records at
// 0x008943a0 (lap ring [0x00891500]..[0x00866ecc], then the pit ring up to [0x00866ef8], wrapping
// to the first record). Per segment: the surface kind from 0x40c43b (record words +4 / +8 >> 3) is
// stored at [0x0062258c] and selects 0x40bd5f (kind with a nonzero step in 0x006225c0) or 0x40bec7;
// 0x51f2e9 refreshes the record when the update changed it (byte [0x0062ac98] == 0xff) and
// [0x0062ac99] != 0xff; every segment ends with 0x40c0ae. Registers are threaded through the
// callees exactly like the original (ebx/ebp/edx/eax leak out); the flags that escape are those of
// the final "sub edi, 0x8943a0" (or of "shr ecx, 0x10" when no segment is due).
void __cdecl FUN_0040bfcf(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t sum = GP4_GLOBAL(uint32_t, 0x0063b818) + GP4_GLOBAL(uint32_t, 0x0062282c);
    c.eax = sum & 0xffffu;
    GP4_GLOBAL(uint32_t, 0x0063b818) = c.eax;
    c.ecx = sum >> 16;
    if (c.ecx == 0) {
        // shr ecx, 0x10 with a zero result: CF = bit 15 of the sum, PF = ZF = 1.
        r->eax = c.eax;
        r->ecx = 0;
        r->eflags = (r->eflags & ~0x8d5u) | ((sum >> 15) & 1u) | 0x44u;
        return;
    }

    c.edi = GP4_GLOBAL(uint32_t, 0x0063b814) + 0x008943a0u;
    do {
        GP4_GLOBAL(uint8_t, 0x0062ac98) = 0;
        c.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, c.edi, 4)) >> 3);
        c.edx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, c.edi, 8)) >> 3);
        gp4::call_regs(0x0040c43b, c);
        GP4_GLOBAL(uint32_t, 0x0062258c) = c.eax;

        if (GP4_ARRAY(uint32_t, 0x006225c0)[c.eax] == 0u) {
            updateWithoutStep(c);
        } else {
            updateWithStep(c);
        }

        if (GP4_GLOBAL(uint8_t, 0x0062ac98) == 0xff && GP4_GLOBAL(uint8_t, 0x0062ac99) != 0xff) {
            gp4::call_regs(0x0051f2e9, c);
        }

        if (c.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
            c.edi += 0x160u;
            if (c.edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                GP4_GLOBAL(uint32_t, 0x0063b81c) += 1u;
                c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        } else {
            c.edi += 0x160u;
            if (c.edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                c.edi = 0x008943a0u;
            }
        }

        gp4::call_regs(0x0040c0ae, c);
        c.ecx -= 1u;
    } while (c.ecx != 0u);

    const uint32_t flags = subtractionFlags32(c.edi, 0x008943a0u);
    c.edi -= 0x008943a0u;
    GP4_GLOBAL(uint32_t, 0x0063b814) = c.edi;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040bfcf, FUN_0040bfcf, "regs(ebx:u32, ebp:u32) -> (eax, ebx, ecx, edx, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+map_0040c43b+ring_0040c0ae+state_0040bec7+state_0040bd5f+state_0040bce6+step_0040bfcf")

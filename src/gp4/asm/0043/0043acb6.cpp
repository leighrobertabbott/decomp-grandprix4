#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// 0x78-byte object records at 0x0063bd7c.
static inline uint32_t objectRecord(uint32_t index) {
    return index * 0x78u + 0x0063bd7cu;
}

// pushal ... popal routine. ebx = first object index. Walks the object indices after it, up to the
// count [0x006097ec] (or, when the byte [0x006097fd] is set, through at least the 22 slots 0..21 of
// the second flag table at 0x00607b58 and then the count again), and for every index whose flag byte
// (0x00607b18 + index, or 0x00607b58 + index in the second mode) is nonzero stores it at [0x006097f4]
// and hands 0x0043ac7f the starting record in ebx and the selected record in edi. Each call sees the
// registers the previous one left (0x0043ac7f clobbers ebx and ecx). All registers are restored; the
// flags left are those of the final `cmp ecx, [0x006097ec]`.
// FUNCTION: GP4 0x0043acb6
void __cdecl FUN_0043acb6(gp4::Regs* r) {
    const uint32_t first = r->ebx;
    GP4_GLOBAL(uint32_t, 0x006097f0) = first;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.ecx = first;
    s.ebx = objectRecord(first);

    uint32_t flags;
    if (GP4_GLOBAL(uint8_t, 0x006097fd) == 0u) {
        for (;;) {
            s.ecx += 1u;
            const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006097ec);
            flags = cmpFlags32(s.ecx, limit);
            if (static_cast<int32_t>(s.ecx) > static_cast<int32_t>(limit)) {
                break;
            }
            if (GP4_ARRAY(uint8_t, 0x00607b18)[s.ecx] != 0u) {
                GP4_GLOBAL(uint32_t, 0x006097f4) = s.ecx;
                s.edi = objectRecord(s.ecx);
                gp4::call_regs(0x0043ac7f, s);
            }
        }
    } else {
        for (;;) {
            s.ecx += 1u;
            bool run = static_cast<int32_t>(s.ecx) < 0x16;
            if (!run) {
                const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006097ec);
                flags = cmpFlags32(s.ecx, limit);
                run = static_cast<int32_t>(s.ecx) <= static_cast<int32_t>(limit);
            }
            if (!run) {
                break;
            }
            if (GP4_ARRAY(uint8_t, 0x00607b58)[s.ecx] != 0u) {
                GP4_GLOBAL(uint32_t, 0x006097f4) = s.ecx;
                s.edi = objectRecord(s.ecx);
                gp4::call_regs(0x0043ac7f, s);
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043acb6, FUN_0043acb6, "regs(ebx:u32[0..63]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043acb6")

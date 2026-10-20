#include <gp4/gp4.h>
#include <gp4/flags.h>

// Piecewise-linear interpolation over a key table of 0x20-byte records { key, v[7] } (two tables:
// 0x00934f7c, or 0x009350bc when the byte 0x00934f64 is set). The parameter 0x00934f04 is clamped
// to [0, 0xf00], the record pair bracketing it is found, and the Q14 fraction t is used to
// interpolate six values into the 0x009351fc block (+0x20..+0x28, +0x30..+0x38); the block's
// first part then receives a negated/copied arrangement of them. Register routine: ebx = block
// base, esi = the bracketing record, ebp = t, edx = high half of the last product; flags from the
// last neg.
// FUNCTION: GP4 0x0054ffcf
void __cdecl FUN_0054ffcf(gp4::Regs* r) {
    int32_t* const out = &GP4_GLOBAL(int32_t, 0x009351fc);
    const int32_t* rec = &GP4_GLOBAL(int32_t, 0x00934f7c);
    if (GP4_GLOBAL(uint8_t, 0x00934f64) != 0) rec = &GP4_GLOBAL(int32_t, 0x009350bc);

    int32_t x = GP4_GLOBAL(int32_t, 0x00934f04);
    if (x < 0) x = 0;
    if (x > 0xf00) x = 0xf00;
    while (x >= rec[8]) rec += 8;

    const uint32_t t = (static_cast<uint32_t>(x - rec[0]) << 14) / static_cast<uint32_t>(rec[8] - rec[0]);

    // record field -> block slot, in the original's order
    static const int kField[6] = {1, 4, 2, 5, 3, 6};
    static const int kSlot[6] = {8, 12, 9, 13, 10, 14};
    uint32_t hi = 0;
    for (int k = 0; k < 6; ++k) {
        const int f = kField[k];
        const int64_t p = static_cast<int64_t>(rec[f + 8] - rec[f]) * static_cast<int64_t>(static_cast<int32_t>(t));
        hi = static_cast<uint32_t>(static_cast<uint64_t>(p) >> 32);
        out[kSlot[k]] = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(p) >> 14)) + rec[f];
    }

    const uint32_t a = static_cast<uint32_t>(out[8]);
    out[0] = -out[12];
    out[4] = -out[8];
    out[1] = out[13];
    out[5] = out[9];
    out[2] = out[14];
    out[6] = out[10];

    r->eax = static_cast<uint32_t>(out[10]);
    r->ebx = 0x009351fcu;
    r->edx = hi;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(rec));
    r->ebp = t;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::neg32(a));
}
GP4_IMPL(0x0054ffcf, FUN_0054ffcf, "regs() -> (eax, ebx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=param_0054ffcf+vals_0054ffcf")

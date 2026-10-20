#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal/popal walk of the 0x160-byte ring records from the lap start [0x891504] to the lap end [0x866ecc].
// A record with flag +0x2c bit 6 whose byte +0x1d is outside 0x7c..0x83 starts a run: every record of the run
// gets byte +0x1d and the high nibble of +0x25 set, and the run goes on while the previously written record had
// flag bit 6. With word [0x66d8b4] bit 8 clear the run values come from the 0x4015dc generator (high nibble =
// (first & 1) << 4, +0x1d = the second value, redrawn until it lies in -61..61); with it set +0x1d is
// clamp(((+0x16 - [0x6f2f60]) << 6) / max(width, 0x100), 0, 0xff), width = +0x60 or +0x62 by the sign of +0x16,
// and the nibble is 0xf0. Registers are restored; the flags are those of the last 'cmp edi,[0x866ecc]'.
// FUNCTION: GP4 0x005201b9
void __cdecl FUN_005201b9(gp4::Regs* r) {
    gp4::cdecl_<0x00401530, void>();
    uint8_t* p = (uint8_t*)GP4_GLOBAL(uint32_t, 0x00891504);
    uint32_t flags = 0;
    uint8_t value = 0;
    uint8_t nibble = 0;
    gp4::Regs c = *r;

    for (;;) {
        if ((p[0x2c] & 0x40) != 0 && (uint8_t)(p[0x1d] - 0x7c) >= 8) {
            if ((GP4_GLOBAL(uint16_t, 0x0066d8b4) & 0x100) == 0) {
                for (;;) {
                    gp4::call_regs(0x004015dc, c);
                    nibble = (uint8_t)((c.eax & 1u) << 4);
                    gp4::call_regs(0x004015dc, c);
                    value = (uint8_t)c.eax;
                    if ((int8_t)value < 0x3e && (int8_t)value > -0x3e) break;
                }
            }
            for (;;) {
                if ((GP4_GLOBAL(uint16_t, 0x0066d8b4) & 0x100) == 0) {
                    p[0x1d] = value;
                    p[0x25] = (uint8_t)((p[0x25] & 0x0f) | nibble);
                } else {
                    const int16_t pos = *(const int16_t*)(p + 0x16);
                    int32_t width = pos < 0 ? *(const uint16_t*)(p + 0x60) : *(const uint16_t*)(p + 0x62);
                    if (width < 0x100) width = 0x100;
                    const int16_t rel = (int16_t)(pos - GP4_GLOBAL(int16_t, 0x006f2f60));
                    int32_t q = ((int32_t)rel << 6) / width;
                    if (q > 0xff) q = 0xff;
                    if (q < 0) q = 0;
                    p[0x1d] = (uint8_t)q;
                    p[0x25] = (uint8_t)((p[0x25] & 0x0f) | 0xf0);
                }
                p += 0x160;
                const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ecc);
                flags = gp4::flags::sub32((uint32_t)p, end);
                if ((uint32_t)p >= end) goto done;
                if ((p[-0x134] & 0x40) == 0) break;
            }
            continue;
        }
        p += 0x160;
        {
            const uint32_t end = GP4_GLOBAL(uint32_t, 0x00866ecc);
            flags = gp4::flags::sub32((uint32_t)p, end);
            if ((uint32_t)p >= end) break;
        }
    }
done:
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005201b9, FUN_005201b9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=seg_00441af6+loops_0041df3f+ring_008943a0+records_008943a0+words_008943a0")

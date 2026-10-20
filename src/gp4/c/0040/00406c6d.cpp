#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a dword-wide `test mem, -1` (CF = OF = AF = 0).
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

static inline void publish(gp4::Regs* r, const gp4::Regs& c, uint32_t flags) {
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (flags & 0x8d5u);
}

// Backward ring search: from the record in edi it walks [0x00868558] + 2 records back (the pit
// records clamp at the pit start [0x00891500], the lap records wrap by [0x00866ec4] below the lap
// start [0x00891504]), asking 0x004067ea for each one. A record where it succeeds with a zero
// result is returned at once (CF = 1). A non-zero result is refined with 0x004067b7 (slot 0) unless
// the previous record also succeeded (byte 0x00627949 is -1 after a success, 0 after a failure);
// the closest refined candidate (smallest 0x00629edc) is remembered in 0x006278dc/e0/e4. When the
// walk ends, CF = 1 with edi/eax = the remembered candidate, or CF = 0 when there was none. The
// result of every success is also published in 0x00629f04 / 0x00629f0c.
// FUNCTION: GP4 0x00406c6d
void __cdecl FUN_00406c6d(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x006278dc) = 0x7fffffffu;
    GP4_GLOBAL(uint32_t, 0x006278e4) = 0u;
    uint32_t counter = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558)) + 2u;
    GP4_GLOBAL(uint8_t, 0x00627949) = 0xffu;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    for (;;) {
        gp4::call_regs(0x004067ea, c);
        c.ecx = counter;  // push ecx / pop ecx around the call
        if ((c.eflags & 1u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x00627949) = 0u;
        } else {
            GP4_GLOBAL(uint32_t, 0x00629f04) = c.eax;
            GP4_GLOBAL(uint32_t, 0x00629f0c) = c.edi;
            if (c.eax == 0u) {
                // cmp eax, 0 (equal) then stc
                publish(r, c, 0x45u);
                return;
            }
            if (GP4_GLOBAL(uint8_t, 0x00627949) == 0u) {
                c.ebx = 0u;
                gp4::call_regs(0x004067b7, c);
                if ((c.eflags & 1u) == 0u) {
                    // jae: stc keeps the other flags of 0x004067b7
                    publish(r, c, (c.eflags & 0x8d5u) | 1u);
                    return;
                }
                c.edx = GP4_GLOBAL(uint32_t, 0x00629edc);
                if (static_cast<int32_t>(c.edx) < GP4_GLOBAL(int32_t, 0x006278dc)) {
                    GP4_GLOBAL(uint32_t, 0x006278dc) = c.edx;
                    GP4_GLOBAL(uint32_t, 0x006278e0) = c.eax;
                    GP4_GLOBAL(uint32_t, 0x006278e4) = c.edi;
                }
                GP4_GLOBAL(uint8_t, 0x00627949) = 0xffu;
            }
        }

        // step the record back one 0x160 slot
        if (c.edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
            c.edi -= 0x160u;
            if (c.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                c.edi = GP4_GLOBAL(uint32_t, 0x00891500);
            }
        } else {
            c.edi -= 0x160u;
            if (c.edi < GP4_GLOBAL(uint32_t, 0x00891504)) {
                c.edi += GP4_GLOBAL(uint32_t, 0x00866ec4);
            }
        }
        --counter;
        c.ecx = counter;
        if (counter == 0u) {
            break;
        }
    }

    const uint32_t best = GP4_GLOBAL(uint32_t, 0x006278e4);
    uint32_t flags = testFlags32(best);
    if (best != 0u) {
        c.edi = best;
        c.eax = GP4_GLOBAL(uint32_t, 0x006278e0);
        flags |= 1u;  // stc
    }
    // best == 0: clc leaves the flags of the test with CF clear
    publish(r, c, flags);
}
GP4_IMPL(0x00406c6d, FUN_00406c6d, "regs(edi:rec[0x8943a0,0x160,0..14]) -> (eax, ebx, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00406ac7+ring_0040aec5+state_004067ea+scan_00406d4b+state_0040bce6")

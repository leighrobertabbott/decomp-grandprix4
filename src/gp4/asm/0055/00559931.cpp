#include <gp4/gp4.h>
#include <gp4/flags.h>

// Links two cursors over the ring of 0x160-byte records (0x008943a0 .. [0x00866ecc], wrapping at the
// end) pairwise, `count` (eax) times: every step stores edi into dword +0x44 of the ebx record and
// ebx into dword +0x44 of the edi record, then moves edi on one record; ebx moves on one record
// whenever the Bresenham-style accumulator (starts at count/2, gains edx per step, compared signed
// with count) overflows count. Wrapped in pushal/popal, so every register is preserved; popal does
// not restore the flags, which stay those of the last `cmp edi, [0x00866ecc]` (or of `or ecx,ecx`
// when count is zero).
// FUNCTION: GP4 0x00559931
void __cdecl FUN_00559931(gp4::Regs* r) {
    const uint32_t count = r->eax;
    const uint32_t step = r->edx;
    uint32_t ebx = r->ebx;
    uint32_t edi = r->edi;

    if (count == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(count));
        return;
    }

    uint32_t acc = count >> 1;
    uint32_t flags = 0u;
    bool first = true;
    for (uint32_t left = count; left != 0u; --left) {
        if (!first) {
            acc += step;
            if (static_cast<int32_t>(acc) >= static_cast<int32_t>(count)) {
                acc -= count;
                ebx += 0x160u;
                if (ebx == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                    ebx = 0x008943a0u;
                }
                GP4_FIELD(uint32_t, ebx, 0x44) = edi;
            }
        } else {
            GP4_FIELD(uint32_t, ebx, 0x44) = edi;
            first = false;
        }
        GP4_FIELD(uint32_t, edi, 0x44) = ebx;
        edi += 0x160u;
        flags = gp4::flags::sub32(edi, GP4_GLOBAL(uint32_t, 0x00866ecc));
        if (edi == GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            edi = 0x008943a0u;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00559931, FUN_00559931, "regs(eax:u32[0..40], edx:u32[0..40], ebx:rec[0x8943a0,0x160,0..3], edi:rec[0x8943a0,0x160,0..3]) -> (cf, pf, af, zf, sf, of); globals=layout=records_008943a0")

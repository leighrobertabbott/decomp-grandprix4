#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// cmp reg, 0: CF = AF = OF = 0.
static inline uint32_t cmpZeroFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value >> 24) & 0x80u);
}

// edi = a 0x160-byte track-segment record of the ring at 0x008943a0; edx = 3 selects the signed
// step-count byte [edi+0x136], any other value [edi+0x137]. Steps along the ring (forward while the
// 0x004074db ratio edx is >= 0x4000 and the count is positive, backward while it is negative and the
// count is negative; the count in DAT_00627828 moves towards zero) and returns the final record in
// edi, the last 0x004074db eax/ecx and the ratio edx clamped to 0..0x4000 with the flags of
// "cmp edx, 0".
// FUNCTION: GP4 0x0041dada
void __cdecl FUN_0041dada(gp4::Regs* r) {
    uint32_t record = r->edi;
    const uint32_t selector = r->edx;

    if (selector == 3u) {
        GP4_GLOBAL(int32_t, 0x00627828) = GP4_FIELD(int8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record)), 0x136);
    } else {
        GP4_GLOBAL(int32_t, 0x00627828) = GP4_FIELD(int8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record)), 0x137);
    }

    gp4::Regs c = *r;
    for (;;) {
        c = *r;
        c.edi = record;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x004074db, c);
        const int32_t ratio = static_cast<int32_t>(c.edx);

        if (ratio < 0) {
            if (GP4_GLOBAL(int32_t, 0x00627828) >= 0) {
                break;
            }
            if (record >= GP4_GLOBAL(uint32_t, 0x00891500)) {
                record -= 0x160u;
                if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    record = GP4_GLOBAL(uint32_t, 0x00891500);
                }
            } else {
                record -= 0x160u;
                if (record < GP4_GLOBAL(uint32_t, 0x00891504)) {
                    record += GP4_GLOBAL(uint32_t, 0x00866ec4);
                }
            }
            GP4_GLOBAL(int32_t, 0x00627828) = GP4_GLOBAL(int32_t, 0x00627828) + 1;
        } else {
            if (ratio < 0x4000) {
                break;
            }
            if (GP4_GLOBAL(int32_t, 0x00627828) <= 0) {
                break;
            }
            record += 0x160u;
            if (record >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                if (record < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    record -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                } else if (record >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    record = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                }
            }
            GP4_GLOBAL(int32_t, 0x00627828) = GP4_GLOBAL(int32_t, 0x00627828) - 1;
        }
    }

    int32_t ratio = static_cast<int32_t>(c.edx);
    if (ratio > 0x4000) {
        ratio = 0x4000;
    }
    // The flags are those of "cmp edx, 0" before the final clamp to zero (mov leaves them).
    const uint32_t flags = cmpZeroFlags32(static_cast<uint32_t>(ratio));
    if (ratio < 0) {
        ratio = 0;
    }

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = static_cast<uint32_t>(ratio);
    r->edi = record;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041dada, FUN_0041dada, "regs(edx:u32[2..3], edi:rec[0x8943a0,0x160,0..11]) -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+state_0040759e+steps_0041dada")

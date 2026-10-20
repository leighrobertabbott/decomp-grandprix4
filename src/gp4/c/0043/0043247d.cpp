#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `add left, right`.
static inline uint32_t addFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// The record in esi (only the fields this routine touches).
struct Unknown_0043247d_Esi {
    uint8_t Unknown00[0x7c];
    int8_t Unknown7C;
    uint8_t Unknown7D[0x8d - 0x7d];
    uint8_t Unknown8D;
    uint8_t Unknown8E[0xf0 - 0x8e];
    uint32_t UnknownF0;      // next event time
    uint8_t UnknownF4[0xff - 0xf4];
    uint8_t UnknownFF;
    uint8_t Unknown100[0x1d8 - 0x100];
    uint32_t Unknown1D8;
    uint32_t Unknown1DC;
};

// 0x00401462 (random byte generator): returns eax with the new byte in al.
static inline uint32_t drawByte(const gp4::Regs* r, uint32_t eaxIn) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eaxIn;
    gp4::call_regs(0x00401462, c);
    return c.eax;
}

// FUNCTION: GP4 0x0043247d
// Schedules the next time [esi+0xf0] = [0x0066d814] + ebp (ebp is the delay in ms):
//   [0x007c2ea9] >= 0: 1000 for mode 0 with a non-negative byte +0x7c; otherwise 10000 when +0x8d bit 6
//     is set (also +0xff |= 0x40, and the current-car record [0x00679d30] is told [0x00852d5e] = 0x80
//     / [0x00852d34] = esi) else 15000 + (random byte << 8);
//   [0x007c2ea9] < 0: random pair delay 7000 + 8 * (a + b - 0xff), raised to the 0x004323c5 value and
//     [0x00602b58]; +0x1dc / +0x1d8 receive clock + max(pair, [0x00602b58]) / clock + 0x004323c5 value.
// The two player car pointers [0x0066d828] (when [0x007aafd4] is set) and [0x0066d820], or +0x8d bit 2
// in the negative mode, force 86400000. eax is the last value computed, edx the first random byte
// (negative mode); flags are those of the final add.
void __cdecl FUN_0043247d(gp4::Regs* r) {
    Unknown_0043247d_Esi* car = reinterpret_cast<Unknown_0043247d_Esi*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t ebp;

    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    bool forced = false;
    if ((mode & 0x80u) == 0u) {
        if (mode == 0u && car->Unknown7C >= 0) {
            ebp = 0x3e8u;
        } else if ((car->Unknown8D & 0x40u) != 0u) {
            car->UnknownFF |= 0x40u;
            if (r->esi == GP4_GLOBAL(uint32_t, 0x00679d30)) {
                GP4_GLOBAL(uint8_t, 0x00852d5e) = 0x80u;
                GP4_GLOBAL(uint32_t, 0x00852d34) = r->esi;
            }
            ebp = 0x2710u;
        } else {
            eax = drawByte(r, eax);
            eax = ((eax & 0xffu) << 8) + 0x3a98u;
            ebp = eax;
            if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && r->esi == GP4_GLOBAL(uint32_t, 0x0066d828)) {
                forced = true;
            } else if (r->esi == GP4_GLOBAL(uint32_t, 0x0066d820)) {
                forced = true;
            }
        }
    } else if ((car->Unknown8D & 4u) != 0u) {
        forced = true;
    } else {
        eax = drawByte(r, eax) & 0xffu;
        edx = eax;
        eax = drawByte(r, eax) & 0xffu;
        eax = ((eax + edx - 0xffu) << 3) + 0x1b58u;
        GP4_GLOBAL(uint32_t, 0x00602b54) = eax;
        ebp = eax;

        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x004323c5, call);
        eax = call.eax;
        GP4_GLOBAL(uint32_t, 0x00602b5c) = eax;
        if (static_cast<int32_t>(ebp) < static_cast<int32_t>(eax)) {
            ebp = eax;
        }
        eax = GP4_GLOBAL(uint32_t, 0x00602b58);
        if (static_cast<int32_t>(ebp) < static_cast<int32_t>(eax)) {
            ebp = eax;
        }
        eax = GP4_GLOBAL(uint32_t, 0x00602b54);
        if (static_cast<int32_t>(eax) < static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00602b58))) {
            eax = GP4_GLOBAL(uint32_t, 0x00602b58);
        }
        eax += GP4_GLOBAL(uint32_t, 0x0066d814);
        car->Unknown1DC = eax;
        eax = GP4_GLOBAL(uint32_t, 0x00602b5c) + GP4_GLOBAL(uint32_t, 0x0066d814);
        car->Unknown1D8 = eax;
    }
    if (forced) {
        ebp = 0x5265c00u;
    }

    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    car->UnknownF0 = clock + ebp;

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags(clock, ebp);
}
GP4_IMPL(0x0043247d, FUN_0043247d, "regs(esi:ptr[0x200]:bytes, eax:u32, edx:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043260e+state_004327cd+state_0066d8c0+state_00432372+temp_00602b50+state_0043247d+div_0043247d")

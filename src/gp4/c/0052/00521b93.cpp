#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / popal wrapper around the per-index clamp 0x00521b37. When the due time [0x0066d840] is set
// and the clock [0x0066d814] has reached it, the hold is cleared (0x66d840 = 0, byte 0x66d8e5 = 0,
// byte 0x66d8e9 = 0xff). Byte 0x893cf8 is cleared, 0x0052197e looks up the record of index
// [0x0066d8d8], and the pair (eax = [0x66d8e0], edx = [0x66d8dc]) is stepped by the first set bit of
// byte [esi+0x6f] (1: eax+1, 2: eax-1, 4: edx-1, 8: edx+1, 0x10: no step) unless a hold is active
// (byte 0x66d8e5 == 0xff: then only byte 0x66d8e4 is cleared). A step arms the hold: the due time
// becomes clock + [0x893ce0] (or [0x893ce4] when byte 0x66d8e9 is 0xff), the bytes 0x66d8e5, 0x66d8e4
// and 0x6f3b7c become 0xff and bit 0x40 of [esi+0x8b] is set. With no bit set everything is cleared.
// The clamped pair is stored back (0x66d8dc, 0x66d8e0 and 0x8660a4), byte 0x893cf8 = 0xff when eax
// changed. Registers are restored; the flags are those of the final byte compare, CF set only when
// 0x66d8e4 is not 0xff and 0x66d8e5 is.

// FUNCTION: GP4 0x00521b93
void __cdecl FUN_00521b93(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs c = *r;

    const uint32_t due = GP4_GLOBAL(uint32_t, 0x0066d840);
    if (due != 0u) {
        if (static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814) - due) >= 0) {
            GP4_GLOBAL(uint32_t, 0x0066d840) = 0u;
            GP4_GLOBAL(uint8_t, 0x0066d8e5) = 0u;
            GP4_GLOBAL(uint8_t, 0x0066d8e9) = 0xffu;
        }
    }

    GP4_GLOBAL(uint8_t, 0x00893cf8) = 0u;
    c.ebx = GP4_GLOBAL(uint32_t, 0x0066d8d8);
    gp4::call_regs(0x0052197e, c);
    c.eax = GP4_GLOBAL(uint32_t, 0x0066d8e0);
    c.edx = GP4_GLOBAL(uint32_t, 0x0066d8dc);

    const uint8_t bits = rec[0x6f];
    if ((bits & 0x1fu) == 0u) {
        GP4_GLOBAL(uint8_t, 0x0066d8e4) = 0u;
        GP4_GLOBAL(uint8_t, 0x0066d8e5) = 0u;
        GP4_GLOBAL(uint8_t, 0x0066d8e9) = 0u;
        GP4_GLOBAL(uint32_t, 0x0066d840) = 0u;
    } else if (GP4_GLOBAL(uint8_t, 0x0066d8e5) == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x0066d8e4) = 0u;
    } else {
        if ((bits & 1u) != 0u) {
            c.eax += 1u;
        } else if ((bits & 2u) != 0u) {
            c.eax -= 1u;
        } else if ((bits & 4u) != 0u) {
            c.edx -= 1u;
        } else if ((bits & 8u) != 0u) {
            c.edx += 1u;
        }
        const uint32_t delay = GP4_GLOBAL(uint8_t, 0x0066d8e9) == 0xffu
                                   ? GP4_GLOBAL(uint32_t, 0x00893ce4)
                                   : GP4_GLOBAL(uint32_t, 0x00893ce0);
        GP4_GLOBAL(uint32_t, 0x0066d840) = GP4_GLOBAL(uint32_t, 0x0066d814) + delay;
        GP4_GLOBAL(uint8_t, 0x0066d8e5) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x0066d8e4) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x006f3b7c) = 0xffu;
        rec[0x8b] = static_cast<uint8_t>(rec[0x8b] | 0x40u);
    }

    gp4::call_regs(0x00521b37, c);
    GP4_GLOBAL(uint32_t, 0x0066d8dc) = c.edx;
    if (c.eax != GP4_GLOBAL(uint32_t, 0x0066d8e0)) {
        GP4_GLOBAL(uint8_t, 0x00893cf8) = 0xffu;
    }
    GP4_GLOBAL(uint32_t, 0x0066d8e0) = c.eax;
    GP4_GLOBAL(uint32_t, 0x008660a4) = c.eax;

    const uint8_t armed = GP4_GLOBAL(uint8_t, 0x0066d8e4);
    uint32_t flags = gp4::flags::sub8(armed, 0xffu);
    uint32_t carry = 0u;
    if (armed != 0xffu) {
        const uint8_t hold = GP4_GLOBAL(uint8_t, 0x0066d8e5);
        flags = gp4::flags::sub8(hold, 0xffu);
        if (hold == 0xffu) {
            carry = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, (flags & ~1u) | carry);
}
GP4_IMPL(0x00521b93, FUN_00521b93, "regs(esi:ptr[0x94]:u8{0,1,2,4,8,0x10,0x20,0x40,0x80,0xff}) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=clock_0066d814+carbg_0066fef0+state_004059ef+player_0040f8d9+car_0052197e+links_00521d6f")

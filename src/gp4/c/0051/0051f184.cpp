#include <gp4/gp4.h>
#include <gp4/flags.h>

// Toggle handshake on bit 0x80 of the byte 0x0066d8a2: when the control byte 0x00679d1b has bit
// 0x40 set the toggle is cleared (and the routine reports "no"); otherwise a clear toggle is set,
// 0x0066d8a4 bit 0x80 is flipped and 0x004e89b0 runs (reported with CF = 1); an already-set toggle
// reports "no" (CF = 0). All general registers are preserved (pushal/popal); the flags are those
// of the last flag writer (`test`, `or cl, 0x80`, `and cl, 0` or the callee) with CF replaced.
// FUNCTION: GP4 0x0051f184
void __cdecl FUN_0051f184(gp4::Regs* r) {
    const uint32_t cl = r->ecx & 0xffu;
    uint32_t arith;
    bool run = false;

    if ((GP4_GLOBAL(uint8_t, 0x00679d1b) & 0x40u) == 0u) {
        const uint8_t toggle = GP4_GLOBAL(uint8_t, 0x0066d8a2);
        if ((toggle & 0x80u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0066d8a4) ^= 0x80u;
            GP4_GLOBAL(uint8_t, 0x0066d8a2) |= 0x80u;
            arith = gp4::flags::logic8(0u);
            run = true;
        } else {
            arith = gp4::flags::logic8(toggle & 0x80u);
        }
    } else {
        GP4_GLOBAL(uint8_t, 0x0066d8a2) &= 0x7fu;
        arith = gp4::flags::logic8(cl | 0x80u);
    }

    uint32_t flags = gp4::flags::merge(r->eflags, arith);
    if (run) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        c.st_xin_mask = 0;
        c.st_xout_mask = 0;
        c.eflags = flags;
        gp4::call_regs(0x004e89b0, c);
        flags = (c.eflags & ~1u) | 1u;
    } else {
        flags &= ~1u;
    }
    r->eflags = flags;
}
GP4_IMPL(0x0051f184, FUN_0051f184, "regs(ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_00510309+carbg_0066fef0+slotcar_00510abf+view_00a4df50+player_004e89b0+flags_0062ffa8")

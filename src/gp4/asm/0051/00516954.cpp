#include <gp4/gp4.h>
#include <gp4/flags.h>

// Lap-time text writer selector (leading-zero-suppressing variant of 0x00516941): a time (milliseconds)
// below 600000 (10:00.000, signed `jl`) is formatted into the buffer at edi by 0x00516ad6, otherwise
// the dash placeholder text comes from 0x00516a3d. Registers and flags come from whichever routine
// runs (the flags start as `cmp eax, 0x927c0`).
// (v2: one noinline helper per callee so the compiler cannot merge the two call addresses)
static __declspec(noinline) void run_time_text(gp4::Regs* r) {
    gp4::call_regs(0x00516ad6, *r);
}

static __declspec(noinline) void run_dash_text(gp4::Regs* r) {
    gp4::call_regs(0x00516a3d, *r);
}

// FUNCTION: GP4 0x00516954
void __cdecl FUN_00516954(gp4::Regs* r) {
    const int32_t time = static_cast<int32_t>(r->eax);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(r->eax, 0x000927c0u));
    if (time < 0x000927c0) {
        run_time_text(r);
    } else {
        run_dash_text(r);
    }
}
GP4_IMPL(0x00516954, FUN_00516954, "regs(eax:i32[0..1200000], edi:ptr[0x40]:bytes) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

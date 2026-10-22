#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: with the byte 0x00875284 clear runs 0x004a60d0, otherwise 0x004a6100 (the two sides of
// the pause/resume toggle of the sound object [0x00a6c0fc]). Both callees end in `xor eax, eax` before their
// ret, and popal keeps flags, so the flags are those of that xor (ZF = PF = 1, the rest clear) on both arms.
// Kept out of line below: MSVC otherwise merges the two calls into one with a selected address.
// FUNCTION: GP4 0x00510c70
static __declspec(noinline) void run_004a60d0() { gp4::cdecl_<0x004a60d0, uint32_t>(); }
static __declspec(noinline) void run_004a6100() { gp4::cdecl_<0x004a6100, uint32_t>(); }

void __cdecl FUN_00510c70(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x00875284) == 0u) {
        run_004a60d0();
    } else {
        run_004a6100();
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x00510c70, FUN_00510c70, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

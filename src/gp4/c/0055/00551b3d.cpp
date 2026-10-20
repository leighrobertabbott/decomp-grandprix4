#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the 11-entry tables: byte 0x00891030[i] = 1 and byte 0x00891044[i] = 0 for i = 0..10, then
// the word 0x0095422c = 0xffff. pushal/popal keep every register; the flags are those of the last
// `inc eax` (eax = 10 before it), with CF kept from the entry flags.
// FUNCTION: GP4 0x00551b3d
void __cdecl FUN_00551b3d(gp4::Regs* r) {
    uint8_t* const enabled = GP4_ARRAY(uint8_t, 0x00891030);
    uint8_t* const cleared = GP4_ARRAY(uint8_t, 0x00891044);
    for (uint32_t i = 0u; i < 0xbu; ++i) {
        enabled[i] = 1u;
        cleared[i] = 0u;
    }
    GP4_GLOBAL(uint16_t, 0x0095422c) = 0xffffu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(10u, r->eflags));
}
GP4_IMPL(0x00551b3d, FUN_00551b3d, "regs(cf:u32[0..1]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

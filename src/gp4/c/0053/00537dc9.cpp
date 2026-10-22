#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the flag dword at [edx + 8] against a mask that depends on the level [0x81e5c4] (unsigned):
// <= 0x20 -> 0xf0, <= 0x30 -> 0xe0, <= 0x40 -> 0xc0, above -> 0x80. eax = -1 when any masked bit is
// set, else 0; the flags are those of the final `test`.
// FUNCTION: GP4 0x00537dc9
void __cdecl FUN_00537dc9(gp4::Regs* r) {
    const uint32_t level = GP4_GLOBAL(uint32_t, 0x0081e5c4);
    uint32_t mask = 0x80u;
    if (level <= 0x20u) {
        mask = 0xf0u;
    } else if (level <= 0x30u) {
        mask = 0xe0u;
    } else if (level <= 0x40u) {
        mask = 0xc0u;
    }
    const uint32_t value = GP4_FIELD(uint32_t, r->edx, 8) & mask;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
    r->eax = (value != 0u) ? 0xffffffffu : 0u;
}
GP4_IMPL(0x00537dc9, FUN_00537dc9, "regs(edx:ptr[0x10]:{8:u32{0,0x0f,0x1f,0x7f,0x80,0xc0,0xe0,0xf0,0xff}}) -> (eax, cf, pf, af, zf, sf, of); gfields={0x0081e5c4:u32{0,0x20,0x21,0x30,0x31,0x40,0x41,0x100,0xffffffff}}")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests the word at 0x009679a4 against 0x26. When equal it tests the byte at 0x00875e85 against 0xff;
// when that differs too the routine ends with `or eax, eax`. The flags are those of the last of these
// instructions executed.
// FUNCTION: GP4 0x00551787
void __cdecl FUN_00551787(gp4::Regs* r) {
    const uint16_t value = GP4_GLOBAL(uint16_t, 0x009679a4);
    uint32_t flags = gp4::flags::sub16(value, 0x26u);
    if (value == 0x26u) {
        const uint8_t byteValue = GP4_GLOBAL(uint8_t, 0x00875e85);
        flags = gp4::flags::sub8(byteValue, 0xffu);
        if (byteValue != 0xffu) {
            flags = gp4::flags::logic32(r->eax);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551787, FUN_00551787, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

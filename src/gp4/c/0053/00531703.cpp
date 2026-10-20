#include <gp4/gp4.h>
#include <gp4/flags.h>

// Clears the byte at ebx + 0x24 when the dword at 0x007ad87c is zero. The flags are those of
// `cmp dword [0x7ad87c], 0`; the byte store does not touch them.
// FUNCTION: GP4 0x00531703
void __cdecl FUN_00531703(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x007ad87c);
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    if (value == 0u) {
        rec[0x24] = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(value, 0u));
}
GP4_IMPL(0x00531703, FUN_00531703, "regs(ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

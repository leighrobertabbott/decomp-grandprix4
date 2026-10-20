#include <gp4/gp4.h>
#include <gp4/flags.h>

// Inverts two 8 KiB byte maps in place (b = 0x10 - b). Each map is chosen by a 16-bit slot word of the table at
// 0x009e1b20 (words 0x43d and 0x43f, i.e. 0x009e239a / 0x009e239e): 0xffff means none (and also skips the second
// map), otherwise it is a byte offset into the pointer table at 0x006975ec whose dword holds the map base.
// pushal/popal keep every register; the flags are those of the last flag writer: `cmp bx, -1` when a slot is
// empty, otherwise the final `add al, 0x10` of the last pass.
static __declspec(noinline) uint32_t Word9e1b20(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

static uint32_t invertMap(uint32_t slot) {
    uint8_t* map = *reinterpret_cast<uint8_t**>(static_cast<uintptr_t>(0x006975ecu + slot));
    uint32_t f = 0u;
    for (uint32_t i = 0; i < 0x2000u; ++i) {
        const uint8_t negated = static_cast<uint8_t>(0u - map[i]);
        f = gp4::flags::add8(negated, 0x10u);
        map[i] = static_cast<uint8_t>(negated + 0x10u);
    }
    return f;
}

// FUNCTION: GP4 0x0051c770
void __cdecl FUN_0051c770(gp4::Regs* r) {
    const uint32_t first = Word9e1b20(0x43du);
    uint32_t f = gp4::flags::sub16(first, 0xffffu);
    if (first != 0xffffu) {
        f = invertMap(first);
        const uint32_t second = Word9e1b20(0x43fu);
        f = gp4::flags::sub16(second, 0xffffu);
        if (second != 0xffffu) {
            f = invertMap(second);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c770, FUN_0051c770, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tex_0051c690; wall=1800")

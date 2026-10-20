#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051c690
// Rescales a 64 KiB byte table in place: b = (uint8_t)(b - 0x0f) >> 2 for each of the 0x10000
// bytes. The table is chosen by the 16-bit word at 0x9e1b20 + 0x34b * 2 (= 0x9e21b6): 0xffff means
// none, otherwise it is a byte offset into the pointer table at 0x6975ec holding the table's base.
// pushal/popal keep every register; the flags are those of the last flag writer, i.e. `cmp bx, -1`
// when skipped, otherwise the final `shr al, 2` (CF = bit 1 of the pre-shift value, OF = AF = 0).
static __declspec(noinline) uint32_t Word9e1b20(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

void __cdecl FUN_0051c690(gp4::Regs* r) {
    const uint32_t index = Word9e1b20(0x34bu);
    uint32_t f = gp4::flags::sub16(index, 0xffffu);
    if (index != 0xffffu) {
        uint8_t* table = *reinterpret_cast<uint8_t**>(static_cast<uintptr_t>(0x006975ecu + index));
        uint32_t last = 0u;
        for (uint32_t i = 0; i < 0x10000u; ++i) {
            last = static_cast<uint8_t>(table[i] - 0x0fu);
            table[i] = static_cast<uint8_t>(last >> 2);
        }
        f = gp4::flags::logic8(last >> 2) | ((last >> 1) & 1u);
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c690, FUN_0051c690, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=tex_0051c690; wall=1800")

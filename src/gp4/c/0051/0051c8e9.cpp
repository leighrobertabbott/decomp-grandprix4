#include <gp4/gp4.h>
#include <gp4/flags.h>

// Word at 0x9e1b20 + 0x351*2 (= 0x9e21c2): 0xffff (none) or a byte offset into the 0x20-byte texture
// entry table at 0x6975ec.
static __declspec(noinline) uint32_t Word9e1b20(uint32_t index) {
    return GP4_ARRAY(uint16_t, 0x009e1b20)[index];
}

// pushal/popal wrapper (every GPR restored; the flags are those of the last flag writer). When the word is not
// 0xffff, the 0x2000-byte map at the entry's first dword is rewritten in place: each byte b becomes
// ((0x10 - b) << 4) & 0xff (`neg al ; add al, 0x10 ; shl al, 4`). The flags are those of `cmp bx, -1`
// when nothing is done, else those of the last `shl al, 4`.
// FUNCTION: GP4 0x0051c8e9
void __cdecl FUN_0051c8e9(gp4::Regs* r) {
    const uint32_t index = Word9e1b20(0x351u);
    uint32_t f = gp4::flags::sub16(index, 0xffffu);
    if (index != 0xffffu) {
        uint8_t* map = *reinterpret_cast<uint8_t**>(static_cast<uintptr_t>(0x006975ecu + index));
        uint32_t shifted = 0u;
        uint32_t sum = 0u;
        for (uint32_t i = 0; i != 0x2000u; ++i) {
            const uint32_t value = map[i];
            sum = (0u - value + 0x10u) & 0xffu;
            shifted = (sum << 4) & 0xffu;
            map[i] = static_cast<uint8_t>(shifted);
        }
        // shl al, 4: CF = bit 4 of the sum, OF (pinned oracle model) = bit 7 of (sum << 3) ^ result,
        // AF = 0, SF/ZF/PF from the result.
        f = gp4::flags::logic8(shifted);
        f |= (sum >> 4) & 1u;
        f |= ((((sum << 3) ^ shifted) & 0x80u) != 0u) ? 0x800u : 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c8e9, FUN_0051c8e9, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=tex_0051c8e9")

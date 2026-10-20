#include <gp4/gp4.h>
#include <gp4/flags.h>

// Maps a key code in eax to a replacement word. Only when the byte at 0x0082fd47 is 0xff and
// ah == 4: ax & 0x21ff is searched (repne scasw, 6 words) in the table at 0x007ab55a and, when
// found at index k, eax becomes the word at 0x0082fd1c + 2k; otherwise eax is unchanged.
// ecx/edx/edi are pushed and restored. Flags are those of the last flag writer: the cmp of the
// byte / of ah, the last scasw compare, or the `shl eax, 1` of the found path.
// FUNCTION: GP4 0x0053c4b1
void __cdecl FUN_0053c4b1(gp4::Regs* r) {
    const uint32_t eax_in = r->eax;
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0082fd47);
    uint32_t flags = gp4::flags::sub8(gate, 0xffu);
    if (gate == 0xffu) {
        const uint32_t high = (eax_in >> 8) & 0xffu;
        flags = gp4::flags::sub8(high, 4u);
        if (high == 4u) {
            const uint32_t key = eax_in & 0x21ffu;
            flags = gp4::flags::logic32(key);
            const uint16_t* keys = GP4_ARRAY(uint16_t, 0x007ab55a);
            uint32_t index = 0u;
            bool found = false;
            for (; index < 6u; ++index) {
                flags = gp4::flags::sub16(key, keys[index]);
                if (keys[index] == (uint16_t)key) {
                    found = true;
                    break;
                }
            }
            if (found) {
                const uint16_t* replacements = GP4_ARRAY(uint16_t, 0x0082fd1c);
                r->eax = replacements[index];
                flags = gp4::flags::logic32(index * 2u);
            }
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053c4b1, FUN_0053c4b1, "regs(eax:u32{0x400,0x401,0x4ff,0x2400,0x2401,0x0,0x12345678,0x404,0x4a5}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

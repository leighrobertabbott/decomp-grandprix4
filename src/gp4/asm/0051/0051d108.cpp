#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the file extension of the zero-or-more character name at eax: scans at most 0x50
// bytes for the first '.', and (unless it is the very first byte, or none is found) compares the
// three bytes after it, packed big-endian, with the 4-byte entries {c0, c1, c2, code} of the table at
// 0x009e1ac4 (first entry "JAM"; a code byte of 0xff ends the table). On a match the entry's code
// byte is stored to 0x009e1b1c and CF is returned clear; otherwise CF is returned set. All GPRs are
// preserved (pushal/popal); the other flags are those of the last compare executed.
// FUNCTION: GP4 0x0051d108
void __cdecl FUN_0051d108(gp4::Regs* r) {
    const uint8_t* name = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->eax));
    uint32_t count = 0x50u;
    uint32_t arith;
    bool found = false;

    for (;;) {
        const uint8_t ch = *name++;
        arith = gp4::flags::sub8(ch, 0x2eu);
        if (ch == 0x2eu) {
            if (count == 0x50u) {
                arith = gp4::flags::sub32(count, 0x50u);
            } else {
                const uint32_t ext = (static_cast<uint32_t>(name[0]) << 16) |
                                     (static_cast<uint32_t>(name[1]) << 8) | name[2];
                const uint8_t* entry = GP4_ARRAY(uint8_t, 0x009e1ac4);
                for (;;) {
                    const uint32_t key = (static_cast<uint32_t>(entry[0]) << 16) |
                                         (static_cast<uint32_t>(entry[1]) << 8) | entry[2];
                    const uint8_t code = entry[3];
                    entry += 4;
                    arith = gp4::flags::sub8(code, 0xffu);
                    if (code == 0xffu) {
                        break;
                    }
                    arith = gp4::flags::sub32(ext, key);
                    if (ext == key) {
                        GP4_GLOBAL(uint8_t, 0x009e1b1c) = code;
                        found = true;
                        break;
                    }
                }
            }
            break;
        }
        count -= 1u;
        if (count == 0u) {
            break;
        }
    }

    const uint32_t carry = found ? 0u : gp4::flags::CF;
    r->eflags = gp4::flags::merge(r->eflags, (arith & ~gp4::flags::CF) | carry);
}
GP4_IMPL(0x0051d108, FUN_0051d108, "regs(eax:ptr[0x60]:u32{0x78787878,0x79797979,0x7a7a7a7a,0x61616161,0x62626262,0x63636363,0x64646464,0x65656565,0x4d414a2e,0x5a5a5a2e}) -> (cf, pf, af, zf, sf, of)")

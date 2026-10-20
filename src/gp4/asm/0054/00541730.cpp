#include <gp4/gp4.h>
#include <gp4/flags.h>

// Translates the 22 slot keys at 0x007c2a1c into the 22 bytes at 0x007c2a32: a key of 0xff stays 0xff,
// any other key k becomes byte +0x17 of the 0x1c-byte slot k of the table at 0x00b1b8d8. Exit: esi and
// edi past the two tables, ecx = 0, al = the last byte stored and the rest of eax as the last non-0xff
// key's slot address left it (eax's upper bits otherwise unchanged). The flags are those of the final
// `dec ecx` (1 -> 0) with CF = 0 (every earlier writer leaves it clear).
// FUNCTION: GP4 0x00541730
void __cdecl FUN_00541730(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x007c2a1c);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c2a32);
    uint32_t eax = r->eax;

    for (uint32_t left = 0x16u; left != 0u; --left) {
        const uint32_t key = *src++;
        eax = (eax & 0xffffff00u) | key;
        if (key == 0xffu) {
            *dst = 0xffu;
        } else {
            eax = key * 0x1cu + 0x00b1b8d8u;
            const uint32_t value = GP4_FIELD(uint8_t, eax, 0x17);
            eax = (eax & 0xffffff00u) | value;
            *dst = static_cast<uint8_t>(value);
        }
        ++dst;
    }

    r->eax = eax;
    r->ecx = 0u;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(src));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(dst));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x00541730, FUN_00541730, "regs(eax:u32) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=blocks_0054045b")

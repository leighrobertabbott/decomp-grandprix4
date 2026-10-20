#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0054853f
// Searches the 22 bytes at 0x007c2a48 for the first entry whose low six bits equal the low six
// bits of al. eax = -1 when that entry also has one of its top two bits (0xc0) set, else 0 (also
// 0 when nothing matches). ebx, ecx and edx are preserved. Flags: the `test byte, 0xc0` on a
// match, otherwise the last `cmp al, dl` (dl = last entry & 0x3f).
void __cdecl FUN_0054853f(gp4::Regs* r) {
    const uint8_t key = (uint8_t)((uint8_t)r->eax & 0x3fu);
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x007c2a48);
    uint8_t entry = 0;
    uint32_t index = 0;
    uint32_t remaining = 0x16;

    do {
        entry = (uint8_t)(table[index] & 0x3fu);
        index += 1u;
        remaining -= 1u;
        if (key == entry) {
            break;
        }
    } while (remaining != 0u);

    uint32_t flags;
    uint32_t result;
    if (key == entry) {
        const uint8_t tag = (uint8_t)(table[index - 1u] & 0xc0u);
        flags = gp4::flags::logic8(tag);
        result = (tag != 0u) ? 0xffffffffu : 0u;
    } else {
        flags = gp4::flags::sub8(key, entry);
        result = 0u;
    }

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054853f, FUN_0054853f, "regs(eax:u32[0..255]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

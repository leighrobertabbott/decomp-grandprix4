#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up the key in al in the table at 0x0081d891: a count byte followed by 4-byte entries
// { key, a, b, c }. Found: eax/ebx/ecx = sign-extended a/b/c of the entry, flags of the matching
// `cmp al, [key]` (equal). Not found: eax = -1, ebx = ecx = 0, flags of the final `dec ecx`
// (result 0, CF kept from the preceding `add edx, 4`). edx is preserved.
// FUNCTION: GP4 0x0052c689
void __cdecl FUN_0052c689(gp4::Regs* r) {
    const uint8_t key = (uint8_t)r->eax;
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x0081d891);
    uint32_t remaining = table[0];
    const uint8_t* entry = table + 1;

    for (;;) {
        if (key == *entry) {
            r->eax = (uint32_t)(int32_t)(int8_t)entry[1];
            r->ebx = (uint32_t)(int32_t)(int8_t)entry[2];
            r->ecx = (uint32_t)(int32_t)(int8_t)entry[3];
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(key, *entry));
            return;
        }
        const uint32_t step = gp4::flags::add32((uint32_t)(uintptr_t)entry, 4u);
        entry += 4;
        remaining -= 1u;
        if (remaining == 0u) {
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, step));
            break;
        }
    }

    r->eax = 0xffffffffu;
    r->ebx = 0u;
    r->ecx = 0u;
}
GP4_IMPL(0x0052c689, FUN_0052c689, "regs(eax:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

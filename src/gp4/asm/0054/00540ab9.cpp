#include <gp4/gp4.h>
#include <gp4/flags.h>

// Scans the 0x16-byte table at 0x7c2a1c for the byte in al. Leaves edx = the number of matching
// entries and edi = the index of the last match (0xffffffff when none); ebx ends one past the table
// and ecx = 0x16. eax is unchanged. The flags are those of the final `cmp ecx, 0x16` (equal
// operands).
// FUNCTION: GP4 0x00540ab9
void __cdecl FUN_00540ab9(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x007c2a1c);
    uint32_t matches = 0u;
    uint32_t last = 0xffffffffu;
    uint32_t index = 0u;
    do {
        if (key == table[index]) {
            last = index;
            ++matches;
        }
        ++index;
    } while (index < 0x16u);
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(table + index));
    r->ecx = index;
    r->edx = matches;
    r->edi = last;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, 0x16u));
}
GP4_IMPL(0x00540ab9, FUN_00540ab9, "regs(eax:u32) -> (ebx, ecx, edx, edi, cf, pf, af, zf, sf, of)")

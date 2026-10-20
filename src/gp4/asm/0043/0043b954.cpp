#include <gp4/gp4.h>

// Looks the byte at esi+0x73 up in the 11-byte table at 0x007c3bcd and stores its index (11 when
// absent) at esi+0xfd. pushal/popal restore every general register; the flags are those of the
// last compare, which ended equal in both exits (ZF and PF set, the rest clear).
// FUNCTION: GP4 0x0043b954
void __cdecl FUN_0043b954(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t key = GP4_FIELD(uint8_t, record, 0x73);
    uint32_t index = 0u;
    do {
        if (key == GP4_ARRAY(uint8_t, 0x007c3bcd)[index]) {
            break;
        }
        index++;
    } while (index != 0xbu);
    GP4_FIELD(uint8_t, record, 0xfd) = static_cast<uint8_t>(index);
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043b954, FUN_0043b954, "regs(esi:ptr[0x105]:bytes) -> (cf, pf, af, zf, sf, of)")

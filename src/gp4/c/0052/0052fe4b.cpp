#include <gp4/gp4.h>
#include <gp4/flags.h>

// Follows the pointer chain starting at the pointer cell 0x00843404 twelve times (esi = [esi]) to a
// record, stores the packet count [0x007a97a4] in the dword +0x2a, then walks the 0x3c-byte records
// from 0x00827160 ([0x007a97a4] + 1 of them, a do-while) and builds a bit mask of the records whose
// byte +4 is zero (bit n for record n; shl takes the count mod 32); the low 16 bits go to the word
// +0x30. pushal/popal keep every register; the flags are those of the final `cmp ecx, eax` (equal:
// ZF and PF).
// FUNCTION: GP4 0x0052fe4b
void __cdecl FUN_0052fe4b(gp4::Regs* r) {
    uint32_t link = 0x00843404u;
    for (int hop = 0; hop < 12; ++hop) {
        link = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(link));
    }
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(link));
    GP4_FIELD(uint32_t, rec, 0x2a) = GP4_GLOBAL(uint32_t, 0x007a97a4);

    const uint8_t* entry = GP4_ARRAY(const uint8_t, 0x00827160);
    uint32_t mask = 0u;
    uint32_t index = 0u;
    uint32_t limit;
    do {
        if (entry[4] == 0u) {
            mask |= 1u << (index & 31u);
        }
        entry += 0x3c;
        index += 1u;
        limit = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    } while (index != limit);
    GP4_FIELD(uint16_t, rec, 0x30) = static_cast<uint16_t>(mask);

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, limit));
}
GP4_IMPL(0x0052fe4b, FUN_0052fe4b, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4+hop_00843404+flag_00827164")

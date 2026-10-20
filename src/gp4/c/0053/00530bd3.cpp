#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the first [0x7a97a4] + 1 records (0x3c bytes each) of the table at 0x827160: byte +5 = 1,
// dwords +0xc and +0x10 = -1 and the seven dwords at +0x28..+0x43 cleared (the clear runs 8 bytes
// into the next record, whose +5 is then set again). pushal/popal restore every register; the
// flags are those of the final `dec ecx` (1 -> 0) with CF kept from the last `add ebx, 0x3c`.
// FUNCTION: GP4 0x00530bd3
void __cdecl FUN_00530bd3(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x007a97a4) + 1u;
    uint32_t record = 0x00827160u;
    uint32_t carry_flags = r->eflags;
    do {
        uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(record));
        GP4_FIELD(uint8_t, p, 5) = 1u;
        GP4_FIELD(uint32_t, p, 0xc) = 0xffffffffu;
        GP4_FIELD(uint32_t, p, 0x10) = 0xffffffffu;
        for (uint32_t i = 0; i != 7u; ++i) {
            GP4_FIELD(uint32_t, p, 0x28 + i * 4u) = 0u;
        }
        carry_flags = gp4::flags::add32(record, 0x3cu);
        record += 0x3cu;
        count -= 1u;
    } while (count != 0u);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, carry_flags));
}
GP4_IMPL(0x00530bd3, FUN_00530bd3, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=count_007a97a4")

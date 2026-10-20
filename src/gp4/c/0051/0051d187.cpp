#include <gp4/gp4.h>
#include <gp4/flags.h>

// Claims bit (eax & 0xf) of the bitmap dword 0x00852d64 unless the gate byte 0x0066d8a1 is set. CF = 1 when the
// bit was already set (nothing changes), CF = 0 otherwise (the bit is set, or the gate is on). eax becomes
// eax & 0xf when the gate is clear. The other flags are those of the `test`/`and` that ran last.
// FUNCTION: GP4 0x0051d187
void __cdecl FUN_0051d187(gp4::Regs* r) {
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0066d8a1);
    uint32_t flags = gp4::flags::logic8(gate);
    if (gate == 0u) {
        const uint32_t bit = r->eax & 0xfu;
        r->eax = bit;
        flags = gp4::flags::logic32(bit);
        uint32_t& map = GP4_GLOBAL(uint32_t, 0x00852d64);
        if (((map >> bit) & 1u) != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, flags | static_cast<uint32_t>(gp4::flags::CF));
            return;
        }
        map |= 1u << bit;
    }
    r->eflags = gp4::flags::merge(r->eflags, flags & ~static_cast<uint32_t>(gp4::flags::CF));
}
GP4_IMPL(0x0051d187, FUN_0051d187, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")

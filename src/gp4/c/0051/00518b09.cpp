#include <gp4/gp4.h>
#include <gp4/flags.h>

// Allocates a slot in the 16-byte in-use table at 0x0088f988: the first zero byte is set to 0x80 and
// its index returned in eax with CF clear (flags of the `test` that found it: ZF and PF). When all 16
// bytes are used eax = -1 and CF is set (the other flags are those of the final `cmp eax, 0x10`).
// FUNCTION: GP4 0x00518b09
void __cdecl FUN_00518b09(gp4::Regs* r) {
    uint8_t* table = GP4_ARRAY(uint8_t, 0x0088f988);
    uint32_t index = 0u;
    while (index != 0x10u) {
        if (table[index] == 0u) {
            table[index] = 0x80u;
            r->eax = index;
            r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u) & ~static_cast<uint32_t>(gp4::flags::CF));
            return;
        }
        index += 1u;
    }
    r->eax = 0xffffffffu;
    r->eflags = gp4::flags::merge(r->eflags, (gp4::flags::sub32(0x10u, 0x10u) & ~static_cast<uint32_t>(gp4::flags::CF)) | gp4::flags::CF);
}
GP4_IMPL(0x00518b09, FUN_00518b09, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=full_00518b09")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the low three bits of the byte 0x0082fd6b are clear and the byte 0x0082fd46 differs from the copy at
// 0x0082fd79, the copy is updated. eax is saved and restored; the flags are those of the last compare that ran
// (`test` of the gate, `cmp al, [copy]`, or `cmp al, 0xff` after the store).
// FUNCTION: GP4 0x0053c815
void __cdecl FUN_0053c815(gp4::Regs* r) {
    const uint8_t gate = GP4_GLOBAL(uint8_t, 0x0082fd6b) & 7u;
    uint32_t flags = gp4::flags::logic8(gate);
    if (gate == 0u) {
        const uint8_t current = GP4_GLOBAL(uint8_t, 0x0082fd46);
        const uint8_t copy = GP4_GLOBAL(uint8_t, 0x0082fd79);
        flags = gp4::flags::sub8(current, copy);
        if (current != copy) {
            GP4_GLOBAL(uint8_t, 0x0082fd79) = current;
            flags = gp4::flags::sub8(current, 0xffu);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0053c815, FUN_0053c815, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

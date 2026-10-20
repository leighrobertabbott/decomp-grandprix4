#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the four input axis words for the viewed car [0x00679d30]: when car byte +0x8d has bit 7
// clear the words 0x0066d884, 0x0066d882 and 0x0066d886 are set to 0x8000 (centred) and 0x0066d888 to
// the word 0x0088f6c8 (ax keeps that value); otherwise only 0x0066d888 is cleared. The flags are
// those of `test byte [esi+0x8d], 0x80`; esi is restored.
// FUNCTION: GP4 0x0051ff9e
void __cdecl FUN_0051ff9e(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    const uint32_t flags = car[0x8d];
    if ((flags & 0x80u) == 0u) {
        GP4_GLOBAL(uint16_t, 0x0066d884) = 0x8000u;
        GP4_GLOBAL(uint16_t, 0x0066d882) = 0x8000u;
        GP4_GLOBAL(uint16_t, 0x0066d886) = 0x8000u;
        const uint16_t value = GP4_GLOBAL(uint16_t, 0x0088f6c8);
        GP4_GLOBAL(uint16_t, 0x0066d888) = value;
        r->eax = (r->eax & 0xffff0000u) | value;
    } else {
        GP4_GLOBAL(uint16_t, 0x0066d888) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(flags & 0x80u));
}
GP4_IMPL(0x0051ff9e, FUN_0051ff9e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d+flags_00408f7b")

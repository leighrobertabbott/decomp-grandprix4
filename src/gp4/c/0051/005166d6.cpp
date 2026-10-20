#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: index = (carbyte[+0x7c] & 0x3f) - 1 of the selected car [0x679d30], clamped above
// to 0x15 (signed, may be -1); eax = -(byte at 0x66ec4a[index]); adds the word [0x7c2e96] to its low
// half when [0x7c2ea9] == 0x80, else the dword [0x7c2e98]; a negative sum is replaced by 0 and stored
// at 0x00a6987c. Every GPR is restored; the flags are those of `cmp eax, 0` on the unclamped sum.
// FUNCTION: GP4 0x005166d6
void __cdecl FUN_005166d6(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    int32_t index = static_cast<int32_t>(GP4_FIELD(uint8_t, car, 0x7c) & 0x3fu) - 1;
    if (index > 0x15) {
        index = 0x15;
    }
    const uint32_t negated = 0u - static_cast<uint32_t>(GP4_ARRAY(uint8_t, 0x0066ec4a)[index]);
    uint32_t sum;
    if (GP4_GLOBAL(uint8_t, 0x007c2ea9) == 0x80u) {
        const uint16_t low = static_cast<uint16_t>(static_cast<uint16_t>(negated) + GP4_GLOBAL(uint16_t, 0x007c2e96));
        sum = (negated & 0xffff0000u) | low;
    } else {
        sum = negated + GP4_GLOBAL(uint32_t, 0x007c2e98);
    }
    const uint32_t arith = gp4::flags::sub32(sum, 0u);
    if (static_cast<int32_t>(sum) < 0) {
        sum = 0u;
    }
    GP4_GLOBAL(uint32_t, 0x00a6987c) = sum;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005166d6, FUN_005166d6, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0044334d+carno_0066ff6c+counts_0066ec49")

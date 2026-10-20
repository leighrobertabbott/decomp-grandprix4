#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043227e
// When bit 1 of byte +0x8d of the esi car is set: if bytes +0x26b has both bit 5 and bit 4 set, bit 2
// of it is set too; then 0x00431fa3 queues the group reset (its registers and flags are the result).
// Without bit 1 the flags are those of the `test` (zero result: ZF|PF).
void __cdecl FUN_0043227e(gp4::Regs* r) {
    void* car = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));

    if ((GP4_FIELD(uint8_t, car, 0x8d) & 2u) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    const uint8_t state = GP4_FIELD(uint8_t, car, 0x26b);
    if ((state & 0x20u) != 0u && (state & 0x10u) != 0u) {
        GP4_FIELD(uint8_t, car, 0x26b) = static_cast<uint8_t>(state | 4u);
    }
    gp4::call_regs(0x00431fa3, *r);
}
GP4_IMPL(0x0043227e, FUN_0043227e, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+state_00431efb+state_00432f18+carx_0042fe2f+state_0043227e")

#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_0066fef0 {
    uint8_t Unknown00[0x84];
    uint8_t Unknown84;
    uint8_t Unknown85[0x8d - 0x85];
    uint8_t Flags8D;
};

// [0x00a6987c] = 1 when bit 4 of byte +0x8d of the selected car [0x00679d30] is set and its byte +0x84
// is below 9 (signed compare of the zero-extended byte), else 0. pushal/popal wrapper: every GPR is
// restored; the flags are those of "test byte [esi+0x8d],0x10" when that bit is clear, otherwise of
// "cmp eax,9".
// FUNCTION: GP4 0x0051661c
void __cdecl FUN_0051661c(gp4::Regs* r) {
    const Unknown_0066fef0* car = reinterpret_cast<const Unknown_0066fef0*>(GP4_GLOBAL(uint32_t, 0x00679d30));
    GP4_GLOBAL(uint32_t, 0x00a6987c) = 0u;
    uint32_t arith = gp4::flags::logic8(car->Flags8D & 0x10u);
    if ((car->Flags8D & 0x10u) != 0u) {
        const uint32_t level = car->Unknown84;
        arith = gp4::flags::sub32(level, 9u);
        if (static_cast<int32_t>(level) < 9) {
            GP4_GLOBAL(uint32_t, 0x00a6987c) = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0051661c, FUN_0051661c, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+order_00418f8f")

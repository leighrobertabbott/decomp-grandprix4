#include <gp4/gp4.h>
#include <gp4/flags.h>

// 22 slots of 0x1c bytes at 0x00b1b8d8; a slot is in use when its first dword is nonzero.
struct Slot_00b1b8d8 {
    uint32_t Used00;
    uint8_t Unknown04[0x18];
};
static_assert(sizeof(Slot_00b1b8d8) == 0x1c, "slot size");

// FUNCTION: GP4 0x00541d5a
// Finds the n-th (n = eax, 0-based) slot in use of the 22-slot table at 0x00b1b8d8.
// Found: eax = slot address, CF = 0 (other flags from `cmp ebx, 0` with ebx = 0).
// Not found (fewer than n+1 slots in use): eax = 0x00b1b8d8 (table base), CF = 1 (other flags
// from the final `dec ecx` reaching 0). ebx and ecx are preserved.
void __cdecl FUN_00541d5a(gp4::Regs* r) {
    Slot_00b1b8d8* slot = GP4_ARRAY(Slot_00b1b8d8, 0x00b1b8d8);
    uint32_t skip = r->eax;
    uint32_t remaining = 22;
    uint32_t result = 0x00b1b8d8u;
    uint32_t arith = (gp4::flags::dec32(1u, 0u) & ~gp4::flags::CF) | gp4::flags::CF;

    do {
        if (slot->Used00 != 0u) {
            if (skip == 0u) {
                result = (uint32_t)(uintptr_t)slot;
                arith = gp4::flags::sub32(0u, 0u) & ~gp4::flags::CF;
                break;
            }
            skip -= 1u;
        }
        slot += 1;
        remaining -= 1u;
    } while (remaining != 0u);

    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00541d5a, FUN_00541d5a, "regs(eax:u32[0..30]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a")

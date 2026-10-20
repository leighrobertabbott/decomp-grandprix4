#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first free slot of the block-pool table at 0x00829cac (0x400 slots of 0x18 bytes; a slot is free when
// its dword +0xc is zero). eax = the slot address, or 0 when all are in use. ecx is saved and restored. The flags
// are those of the `cmp dword [eax+0xc], 0` that found the slot, or (table full) those of the final `dec ecx`
// (1 -> 0) which keeps the CF of the last `add eax, 0x18`.
// FUNCTION: GP4 0x00538971
void __cdecl FUN_00538971(gp4::Regs* r) {
    uint32_t slot = 0x00829cacu;
    uint32_t flags = 0u;
    for (uint32_t remaining = 0x400u; ; ) {
        const uint32_t state = GP4_FIELD(uint32_t, slot, 0xc);
        if (state == 0u) {
            flags = gp4::flags::sub32(state, 0u);
            break;
        }
        flags = gp4::flags::add32(slot, 0x18u);
        slot += 0x18u;
        --remaining;
        if (remaining == 0u) {
            flags = gp4::flags::dec32(1u, flags);
            slot = 0u;
            break;
        }
    }
    r->eax = slot;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00538971, FUN_00538971, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=pool_005389b9+full_00538971")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the first free slot of the 22-slot table at 0x00b1b8d8 (0x1c bytes each; a slot is free when
// its first dword is zero). eax = the slot address, or 0 when every slot is used. ebx is saved and
// restored. The flags are those of the `cmp dword [slot], 0` that found the slot, or of the final
// `cmp ebx, 0x16`.
// FUNCTION: GP4 0x00541a18
void __cdecl FUN_00541a18(gp4::Regs* r) {
    uint8_t* slot = GP4_ARRAY(uint8_t, 0x00b1b8d8);
    uint32_t arith = 0u;
    for (uint32_t i = 0; i != 0x16u; ++i) {
        const uint32_t first = GP4_FIELD(uint32_t, slot, 0);
        arith = gp4::flags::sub32(first, 0u);
        if (first == 0u) {
            r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
            r->eflags = gp4::flags::merge(r->eflags, arith);
            return;
        }
        slot += 0x1c;
    }
    r->eax = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x16u, 0x16u));
}
GP4_IMPL(0x00541a18, FUN_00541a18, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_00541d5a+full_00541a18")

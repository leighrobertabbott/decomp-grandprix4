#include <gp4/gp4.h>

// Projects the car record in esi onto its current track segment: 0x004049bf stores the cos/sin-scaled pair
// [0x00623e30] / [0x00623e38] from the car's words +0x18 and +0x9a, then 0x00402fcd (edi = segment pointer [esi+0x1c])
// interpolates along the segment. Returns eax = [0x00623e30], edx = the 0x00402fcd result and ecx = [0x00623e38]; the
// flags are those of the last callee. ebp, ebx and edi are saved and restored.
// FUNCTION: GP4 0x00443db0
void __cdecl FUN_00443db0(gp4::Regs* r) {
    const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    state.edi = *reinterpret_cast<const uint32_t*>(car + 0x1c);
    gp4::call_regs(0x004049bf, state);
    gp4::call_regs(0x00402fcd, state);

    r->edx = state.eax;
    r->eax = GP4_GLOBAL(uint32_t, 0x00623e30);
    r->ecx = GP4_GLOBAL(uint32_t, 0x00623e38);
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00443db0, FUN_00443db0, "regs(esi:ptr[0x41c]:layout=car_0041e500+car_00424878) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0")

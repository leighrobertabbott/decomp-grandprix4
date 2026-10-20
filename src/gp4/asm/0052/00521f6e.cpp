#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs 0x004364b6 (the car in esi) when bit 5 of byte [esi+0x7b] is clear, or when the words
// 0x0066d886 and 0x0088f6be are equal. In the second case ax holds the word 0x0066d886 on return
// (and the flags are those of the word compare unless 0x004364b6 ran and replaced them).
// FUNCTION: GP4 0x00521f6e
void __cdecl FUN_00521f6e(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t bits = car[0x7b] & 0x20u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(bits));
    if (bits != 0u) {
        const uint16_t ax = GP4_GLOBAL(uint16_t, 0x0066d886);
        const uint16_t other = GP4_GLOBAL(uint16_t, 0x0088f6be);
        r->eax = (r->eax & 0xffff0000u) | ax;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub16(ax, other));
        if (ax != other) {
            return;
        }
    }
    gp4::call_regs(0x004364b6, *r);
}
GP4_IMPL(0x00521f6e, FUN_00521f6e, "regs(eax:u32, esi:ptr[0x100]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=clock_0066d814+state_0043b920+stream_0062d1a0+due_004364b6")

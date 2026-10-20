#include <gp4/gp4.h>

// Marks the object record of the car in esi (index = byte +0x25e, records of 0x78 bytes at 0x0063bd7c) with flag
// bit 0 at +0x6f, then refreshes it through 0x00424878 with the car's remaining-parts mask ~[esi+0x40] & 0x7fffff.
// pushal / popal restore every register; only the flags of the call remain.
// FUNCTION: GP4 0x00425027
void __cdecl FUN_00425027(gp4::Regs* r) {
    const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t* const object = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0063bd7cu + static_cast<uint32_t>(car[0x25e]) * 0x78u));
    object[0x6f] |= 1u;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object));
    call.eax = ~*reinterpret_cast<const uint32_t*>(car + 0x40) & 0x007fffffu;
    gp4::call_regs(0x00424878, call);

    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x00425027, FUN_00425027, "regs(esi:ptr[0x41c]:layout=car_0041e500+rec_00420e1d+car_00424878+car_00424cf5) -> (cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21+recs_0063bd7c")

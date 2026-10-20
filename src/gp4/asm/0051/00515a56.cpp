#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks up a slot number in the dword table at 0x00893ff0 indexed by byte [esi+0x84],
// marks the flag byte 0x00866014[clamp(slot, 0, 0x3f)], loads eax = 0x00891bd0[slot]
// (the UNclamped slot), calls the empty routine 0x00515b63, clears bit 1 of byte
// [esi+0x8a] and stores 0xff at 0x00954169. The flags are those of the final `and`.
// FUNCTION: GP4 0x00515a56
void __cdecl FUN_00515a56(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)(uintptr_t)r->esi;

    const int32_t slot = GP4_ARRAY(int32_t, 0x00893ff0)[rec[0x84]];
    int32_t clamped = slot;
    if (clamped < 0) clamped = 0;
    if (clamped > 0x3f) clamped = 0x3f;
    GP4_ARRAY(uint8_t, 0x00866014)[clamped] = 1;

    r->eax = GP4_ARRAY(uint32_t, 0x00891bd0)[slot];
    gp4::cdecl_<0x00515b63, void>();

    const uint8_t cleared = (uint8_t)(rec[0x8a] & 0xfdu);
    rec[0x8a] = cleared;
    GP4_GLOBAL(uint8_t, 0x00954169) = 0xff;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(cleared));
}
GP4_IMPL(0x00515a56, FUN_00515a56, "regs(esi:ptr[0x92]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tab_00515a56")

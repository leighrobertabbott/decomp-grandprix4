#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips one 16-bit entry at esi (esi += 2, flags of that `add esi, 2`). When the mode byte 0x0086857c is 2 it
// also stores ebx at 0x00866f28 and the sign-extended word at [esi] in 0x00866f3c (and eax).
// FUNCTION: GP4 0x00558b42
void __cdecl FUN_00558b42(gp4::Regs* r) {
    const uint32_t esi = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) == 2u) {
        GP4_GLOBAL(uint32_t, 0x00866f28) = r->ebx;
        const int32_t value = *reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(esi));
        r->eax = static_cast<uint32_t>(value);
        GP4_GLOBAL(uint32_t, 0x00866f3c) = static_cast<uint32_t>(value);
    }
    r->esi = esi + 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi, 2u));
}
GP4_IMPL(0x00558b42, FUN_00558b42, "regs(eax:u32, ebx:u32, esi:ptr[8]:u16{0,0,1,2,3,0x10,0x100,0x7fff,0x8000,0xffff}) -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=mode_0086857c")

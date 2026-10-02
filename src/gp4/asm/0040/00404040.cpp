#include <gp4/gp4.h>

// FUNCTION: GP4 0x00404040
void __cdecl FUN_00404040(gp4::Regs* r) {
    const uint32_t eax = r->eax;
    const uint32_t edx = r->edx;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t carryIn = r->eflags & 1u;

    // rcr ax, 1: rotate the 17-bit value (CF:ax) right by one
    const uint16_t ax = static_cast<uint16_t>(eax);
    const uint16_t rotated = static_cast<uint16_t>((ax >> 1) | (carryIn << 15));
    const uint32_t carryOut = ax & 1u;
    const uint32_t overflow = ((rotated >> 15) ^ (rotated >> 14)) & 1u;

    *reinterpret_cast<uint16_t*>(record + 4) = rotated;
    *reinterpret_cast<uint16_t*>(record + 6) = static_cast<uint16_t>(edx);

    r->eax = (eax & 0xffff0000u) | rotated;
    r->eflags = (r->eflags & ~0x801u) | carryOut | (overflow << 11);
}
GP4_IMPL(0x00404040, FUN_00404040, "regs(eax:u32, edx:u32, esi:ptr[0x40]:bytes, cf:u32[0..1]) -> (eax, cf, of)")

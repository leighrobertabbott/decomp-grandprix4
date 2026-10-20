#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051e467
// Force-feedback update for the car record in [0x66d820] (esi; null does nothing, flags of the
// null test). The six force dwords 0x6f2d50/54/58/60/64/68 are zeroed, then 0x410cef (car byte
// +0xcf bit 2 -> eax = 0/-1) and 0x51e319 (the car's steering word +0x5e, sign-extended, with that
// value in edx) produce the force; when 0x51e319 leaves CF clear the effect gate 0x51e405 runs.
// Registers and flags are whatever the last callee left (all three work through call_regs).
void __cdecl FUN_0051e467(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d820);
    r->esi = car;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(car));
    if (car == 0u) {
        return;
    }

    GP4_GLOBAL(uint32_t, 0x006f2d50) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d54) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d58) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d60) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d64) = 0u;
    GP4_GLOBAL(uint32_t, 0x006f2d68) = 0u;

    gp4::Regs c = *r;
    gp4::call_regs(0x00410cef, c);
    c.edx = c.eax;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(car));
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(GP4_FIELD(uint16_t, record, 0x5e))));
    gp4::call_regs(0x0051e319, c);
    if ((c.eflags & gp4::flags::CF) == 0u) {
        gp4::call_regs(0x0051e405, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051e467, FUN_0051e467, "regs(ebx:u32) -> (eax, ebx, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=layout=state_0041104f+cf_0041a2b2")

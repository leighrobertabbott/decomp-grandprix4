#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call target so the compiler keeps two distinct call sites.
static __declspec(noinline) void CallKindTest(gp4::Regs& c) {
    gp4::call_regs(0x004081fc, c);
}
static __declspec(noinline) void CallPhaseStep(gp4::Regs& c) {
    gp4::call_regs(0x00552595, c);
}

// Works on the selected car record [0x679d30] (esi): [0x967a18] = (clamp(byte +0x73, 1, 0xb) - 1) * 0x100;
// when bit 0x20 of byte +0x7b is set the phase [0x66d886] is stepped through 0x552595 - unless it is
// exactly 0x8000 and the record's kind (byte +0xe4 == 4, tested by 0x4081fc, CF) says so, in which case
// it returns at once, else the phase is reloaded from [0x88f6be] first. esi keeps the record pointer;
// eax, edx and the flags are what the last step left (eax = the table value, the flags of the
// `test byte [esi+0x7b], 0x20` / of 0x4081fc / of 0x552595).
// FUNCTION: GP4 0x0055263a
void __cdecl FUN_0055263a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    int32_t kind = GP4_FIELD(uint8_t, c.esi, 0x73);
    if (kind < 1) kind = 1;
    if (kind > 0xb) kind = 0xb;
    c.eax = static_cast<uint32_t>(kind - 1) << 8;
    GP4_GLOBAL(uint32_t, 0x00967a18) = c.eax;

    const uint32_t bits = GP4_FIELD(uint8_t, c.esi, 0x7b) & 0x20u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(bits));
    if (bits != 0u) {
        const uint16_t phase = GP4_GLOBAL(uint16_t, 0x0066d886);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub16(phase, 0x8000u));
        if (phase == 0x8000u) {
            CallKindTest(c);
            if ((c.eflags & gp4::flags::CF) == 0u) {
                GP4_GLOBAL(uint16_t, 0x0066d886) = GP4_GLOBAL(uint16_t, 0x0088f6be);
                CallPhaseStep(c);
            }
        } else {
            CallPhaseStep(c);
        }
    }
    r->eax = c.eax;
    r->edx = c.edx;
    r->esi = c.esi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0055263a, FUN_0055263a, "regs(edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")

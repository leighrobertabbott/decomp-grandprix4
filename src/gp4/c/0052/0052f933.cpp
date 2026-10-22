#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Unknown_00826e62 {
    uint8_t Unknown00[0x3c];
    uint32_t Unknown3c;
    uint32_t Unknown40;
    uint32_t Unknown44;
    uint32_t Unknown48;
    uint8_t Unknown4c[0x10];
};
#pragma pack(pop)

static __declspec(noinline) uint32_t spanBytes(uint32_t end, uint32_t begin) {
    return end - begin;
}

// Fills the size fields (+0x3c, +0x44, +0x48) of the three records named "f1gstate.sav",
// "f1gstate.bak" and "f1gstate.tmp" (stride 0x5c) with 0x007c2e70 - 0x007ab028 (the .sav record's
// +0x44 is 0xc4 less). Leaves ebx = the span, ecx = span - 0xc4, eax = the last record, and the
// flags of `sub ecx, 0xc4`.
// FUNCTION: GP4 0x0052f933
void __cdecl FUN_0052f933(gp4::Regs* r) {
    const uint32_t span = spanBytes(0x007c2e70u, 0x007ab028u);
    const uint32_t shortSpan = span - 0xc4u;

    Unknown_00826e62* sav = reinterpret_cast<Unknown_00826e62*>(0x00826e62u);
    sav->Unknown3c = span;
    sav->Unknown44 = shortSpan;
    sav->Unknown48 = span;

    Unknown_00826e62* bak = reinterpret_cast<Unknown_00826e62*>(0x00826ebeu);
    bak->Unknown3c = span;
    bak->Unknown44 = span;
    bak->Unknown48 = span;

    Unknown_00826e62* tmp = reinterpret_cast<Unknown_00826e62*>(0x00826f1au);
    tmp->Unknown3c = span;
    tmp->Unknown44 = span;
    tmp->Unknown48 = span;

    r->ebx = span;
    r->ecx = shortSpan;
    r->eax = 0x00826f1au;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(span, 0xc4u));
}
GP4_IMPL(0x0052f933, FUN_0052f933, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// One of the 0x400 six-dword slots at 0x829cac (fields named only by offset; +0x0c is the in-use word).
struct Unknown_005388dc_Slot {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
};

// Returns the in-use word (+0x0c) of slot (eax & 0x3ff) of the table at 0x829cac. The flags are those of
// `add eax, 0x829cac` (the address computation).
// FUNCTION: GP4 0x005388dc
void __cdecl FUN_005388dc(gp4::Regs* r) {
    const uint32_t offset = (r->eax & 0x3ffu) * 0x18u;
    const Unknown_005388dc_Slot* slot = GP4_ARRAY(Unknown_005388dc_Slot, 0x00829cac) + (offset / 0x18u);
    r->eax = slot->Unknown0C;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x00829cacu));
}
GP4_IMPL(0x005388dc, FUN_005388dc, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_005388ed")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// One of the 0x400 six-dword slots at 0x829cac (fields named only by offset).
struct Unknown_005388ba_Slot {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
};

// Stamps slot (eax & 0x3ff) of the table at 0x829cac with the running counter at 0x82fcb4 (stored
// at +0x14), increments the counter and returns the dword at +0x08 of the slot. ebx is saved and
// restored. The flags are those of the counter `inc dword [0x82fcb4]`, which keeps the CF of
// `add eax, 0x829cac` (the address computation).
// FUNCTION: GP4 0x005388ba
void __cdecl FUN_005388ba(gp4::Regs* r) {
    const uint32_t offset = (r->eax & 0x3ffu) * 0x18u;
    Unknown_005388ba_Slot* slot = GP4_ARRAY(Unknown_005388ba_Slot, 0x00829cac) + (offset / 0x18u);
    const uint32_t address_flags = gp4::flags::add32(offset, 0x00829cacu);
    const uint32_t counter = GP4_GLOBAL(uint32_t, 0x0082fcb4);
    slot->Unknown14 = counter;
    GP4_GLOBAL(uint32_t, 0x0082fcb4) = counter + 1u;
    r->eax = slot->Unknown08;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(counter, address_flags));
}
GP4_IMPL(0x005388ba, FUN_005388ba, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_005388ed+slot8_005388ba")

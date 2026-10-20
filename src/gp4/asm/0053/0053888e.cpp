#include <gp4/gp4.h>
#include <gp4/flags.h>

// One of the 0x400 six-dword slots at 0x829cac (fields named only by offset).
struct Unknown_0053888e_Slot {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
};

// Checks slot (eax & 0x3ff) of the table at 0x829cac against the key eax >> 10: eax = 0xffffffff when the
// slot is in use (+0xc nonzero) and its +4 equals the key, else 0. ebx is saved and restored. The flags are
// those of `cmp [slot+0xc], 0` (slot free) or `cmp [slot+4], key`.
// FUNCTION: GP4 0x0053888e
void __cdecl FUN_0053888e(gp4::Regs* r) {
    const uint32_t key = r->eax >> 10;
    const uint32_t offset = (r->eax & 0x3ffu) * 0x18u;
    const Unknown_0053888e_Slot* slot = GP4_ARRAY(Unknown_0053888e_Slot, 0x00829cac) + (offset / 0x18u);
    uint32_t arith;
    uint32_t result;

    if (slot->Unknown0C == 0u) {
        arith = gp4::flags::sub32(slot->Unknown0C, 0u);
        result = 0u;
    } else {
        arith = gp4::flags::sub32(slot->Unknown04, key);
        result = (slot->Unknown04 == key) ? 0xffffffffu : 0u;
    }
    r->eax = result;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0053888e, FUN_0053888e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_005388ed")

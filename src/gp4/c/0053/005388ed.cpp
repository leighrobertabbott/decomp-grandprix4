#include <gp4/gp4.h>
#include <gp4/flags.h>

// One of the 0x400 six-dword slots at 0x829cac (fields named only by offset: +0x04 is the key
// that tightens the limit, +0x0c is the in-use word, +0x14 is compared with the limit).
struct Unknown_005388ed_Slot {
    uint32_t Unknown00;
    uint32_t Unknown04;
    uint32_t Unknown08;
    uint32_t Unknown0C;
    uint32_t Unknown10;
    uint32_t Unknown14;
};

// FUNCTION: GP4 0x005388ed
// Scans the 0x400 slots of 0x18 bytes at 0x829cac. A slot qualifies when its +0x0c word is
// nonzero and its +0x14 word is below the running limit (initially the dword at 0x82fcb4); each
// qualifying slot becomes the answer and its +0x04 word becomes the new limit. eax = last
// qualifying slot, or 0 when none. ebx/ecx/edx/esi are saved and restored; the flags are those
// of the final `dec ecx` (1 -> 0) with CF still clear from `add esi, 0x18`: ZF = PF = 1.
void __cdecl FUN_005388ed(gp4::Regs* r) {
    const Unknown_005388ed_Slot* slot = GP4_ARRAY(Unknown_005388ed_Slot, 0x00829cac);
    const Unknown_005388ed_Slot* found = 0;
    uint32_t limit = GP4_GLOBAL(uint32_t, 0x0082fcb4);
    for (int i = 0; i < 0x400; ++i) {
        if (slot->Unknown0C != 0u && slot->Unknown14 < limit) {
            found = slot;
            limit = slot->Unknown04;
        }
        ++slot;
    }
    r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(found));
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, 0u));
}
GP4_IMPL(0x005388ed, FUN_005388ed, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_005388ed")

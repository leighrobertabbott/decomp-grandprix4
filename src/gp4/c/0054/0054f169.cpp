#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores 0x91 + the per-circuit byte table entry [0x838ea0 + circuit] into the field at +0x12 of two
// linked nodes: the 6th node reached from the head at 0x8429b0 and the 3rd node reached from the
// head at 0x842e56. pushal/popal restore every register; the flags are the add's carry kept through
// the final `dec ecx` (1 -> 0) of the second walk.
// FUNCTION: GP4 0x0054f169
void __cdecl FUN_0054f169(gp4::Regs* r) {
    const uint32_t circuit = GP4_GLOBAL(uint32_t, 0x007ad894);
    const uint32_t entry = GP4_ARRAY(uint8_t, 0x00838ea0)[circuit];
    const uint32_t value = entry + 0x91u;

    uint32_t node = 0x008429b0u;
    for (int n = 0; n < 6; ++n) node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(node + 0x12u)) = value;

    node = 0x00842e56u;
    for (int n = 0; n < 3; ++n) node = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(node));
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(node + 0x12u)) = value;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, gp4::flags::add32(entry, 0x91u)));
}
GP4_IMPL(0x0054f169, FUN_0054f169, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=trk_007ad894")

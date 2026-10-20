#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the used slots (first dword nonzero) among the 22 records of 0x1c bytes at 0x00b1b8d8; eax = the count.
// ebx and ecx are saved and restored. The flags are those of the final `dec ecx` (1 -> 0), which keeps the CF of
// the last `add ebx, 0x1c`.
// FUNCTION: GP4 0x005419ea
void __cdecl FUN_005419ea(gp4::Regs* r) {
    const uint8_t* slot = GP4_ARRAY(uint8_t, 0x00b1b8d8);
    uint32_t used = 0u;
    uint32_t flags = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        if (*reinterpret_cast<const uint32_t*>(slot) != 0u) {
            ++used;
        }
        flags = gp4::flags::add32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot)), 0x1cu);
        slot += 0x1c;
    }
    r->eax = used;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, flags));
}
GP4_IMPL(0x005419ea, FUN_005419ea, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=slots_00541d5a")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the nonzero words among the 22 at 0x007a93fa and stores the count in [0x008236ac]. Leaves
// eax = the count, ebx = the end of the table (0x007a9426), ecx = 0 and the flags of the final
// `add ebx, 2` (`loop` does not touch them).
// FUNCTION: GP4 0x0054888e
void __cdecl FUN_0054888e(gp4::Regs* r) {
    uint32_t cursor = 0x007a93fau;
    uint32_t count = 0u;
    uint32_t flags = 0u;
    for (uint32_t left = 0x16u; left != 0u; --left) {
        if (*reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(cursor)) != 0u) {
            ++count;
        }
        flags = gp4::flags::add32(cursor, 2u);
        cursor += 2u;
    }
    GP4_GLOBAL(uint32_t, 0x008236ac) = count;
    r->eax = count;
    r->ebx = cursor;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054888e, FUN_0054888e, "regs() -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=words_0054888e")

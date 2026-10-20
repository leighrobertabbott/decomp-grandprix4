#include <gp4/gp4.h>
#include <gp4/flags.h>

// Fills in the missing members of a ring-segment bounds block at 0x866f24..0x866f48 from the track
// ring limits: each of [0x866f38], [0x866f24], [0x866f34], [0x866f28] that is zero is set from
// [0x866ef4] - 0x420, [0x866ef0] + 0x420, [0x866f04] + 0x2940 and [0x866f00] - 0x2940; then a nonzero
// [0x866f3c] gives [0x866f48] = [0x866f3c] * 0x160 + [0x866ed0] and a nonzero [0x866f40] gives
// [0x866f44] = [0x866ed4] - [0x866f40] * 0x160 (32-bit products). eax ends as the last value loaded
// or computed and edx as the high dword of the last `mul` (0x160 when none ran, else the incoming
// edx); the flags are those of the last flag writer (a `test`/`or`, or the final add).
// FUNCTION: GP4 0x00558cf6
void __cdecl FUN_00558cf6(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags = r->eflags;

    flags = gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x00866f38));
    if (GP4_GLOBAL(uint32_t, 0x00866f38) == 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x00866ef4);
        flags = gp4::flags::add32(eax, 0xfffffbe0u);
        eax += 0xfffffbe0u;
        GP4_GLOBAL(uint32_t, 0x00866f38) = eax;
    }
    flags = gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x00866f24));
    if (GP4_GLOBAL(uint32_t, 0x00866f24) == 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x00866ef0);
        flags = gp4::flags::add32(eax, 0x420u);
        eax += 0x420u;
        GP4_GLOBAL(uint32_t, 0x00866f24) = eax;
    }
    flags = gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x00866f34));
    if (GP4_GLOBAL(uint32_t, 0x00866f34) == 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x00866f04);
        flags = gp4::flags::add32(eax, 0x2940u);
        eax += 0x2940u;
        GP4_GLOBAL(uint32_t, 0x00866f34) = eax;
    }
    flags = gp4::flags::logic32(GP4_GLOBAL(uint32_t, 0x00866f28));
    if (GP4_GLOBAL(uint32_t, 0x00866f28) == 0u) {
        eax = GP4_GLOBAL(uint32_t, 0x00866f00);
        flags = gp4::flags::add32(eax, 0xffffd6c0u);
        eax += 0xffffd6c0u;
        GP4_GLOBAL(uint32_t, 0x00866f28) = eax;
    }

    eax = GP4_GLOBAL(uint32_t, 0x00866f3c);
    flags = gp4::flags::logic32(eax);
    if (eax != 0u) {
        const uint64_t product = static_cast<uint64_t>(eax) * 0x160u;
        edx = static_cast<uint32_t>(product >> 32);
        eax = static_cast<uint32_t>(product);
        flags = gp4::flags::add32(eax, GP4_GLOBAL(uint32_t, 0x00866ed0));
        eax += GP4_GLOBAL(uint32_t, 0x00866ed0);
        GP4_GLOBAL(uint32_t, 0x00866f48) = eax;
    }

    eax = GP4_GLOBAL(uint32_t, 0x00866f40);
    flags = gp4::flags::logic32(eax);
    if (eax != 0u) {
        const uint64_t product = static_cast<uint64_t>(eax) * 0x160u;
        edx = static_cast<uint32_t>(product >> 32);
        eax = static_cast<uint32_t>(product);
        eax = 0u - eax;
        flags = gp4::flags::add32(eax, GP4_GLOBAL(uint32_t, 0x00866ed4));
        eax += GP4_GLOBAL(uint32_t, 0x00866ed4);
        GP4_GLOBAL(uint32_t, 0x00866f44) = eax;
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00558cf6, FUN_00558cf6, "regs(edx:u32) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

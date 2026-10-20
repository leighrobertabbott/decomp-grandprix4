#include <gp4/gp4.h>
#include <gp4/flags.h>

// Picks the pair (0x0066d8d4, 0x0066d8d8) from the mode byte 0x007c2ea9: bit 7 set -> (2, 6),
// else bit 6 set -> (1, 3), else (0, 0). 0x008660a0 receives the second value as well. eax/edx
// end as the pair; the flags are those of the last `test` (byte & 0xff on the negative path,
// byte & 0x40 otherwise).
// FUNCTION: GP4 0x00521d29
void __cdecl FUN_00521d29(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t first;
    uint32_t second;
    uint32_t arith;
    if ((mode & 0x80u) != 0u) {
        second = 6u;
        first = 2u;
        arith = gp4::flags::logic8(mode);
    } else {
        arith = gp4::flags::logic8(mode & 0x40u);
        if ((mode & 0x40u) != 0u) {
            second = 3u;
            first = 1u;
        } else {
            second = 0u;
            first = 0u;
        }
    }
    GP4_GLOBAL(uint32_t, 0x0066d8d4) = first;
    GP4_GLOBAL(uint32_t, 0x0066d8d8) = second;
    GP4_GLOBAL(uint32_t, 0x008660a0) = second;

    r->eax = first;
    r->edx = second;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00521d29, FUN_00521d29, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

#include <gp4/gp4.h>

// When the byte flag at 0x007aafd4 is set, 0x00444f4a appends the two bytes (0x0c, [0x0066d8b4]) to the
// buffer at [0x0062d1a0] and advances that pointer by 2. All registers are preserved; the flags are
// those of the byte test, or of the pointer advance inside 0x00444f4a.
// FUNCTION: GP4 0x00436504
void __cdecl FUN_00436504(gp4::Regs* r) {
    uint32_t flags = 0x44u;  // test byte [0x7aafd4], 0xff on zero: PF and ZF
    if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x00444f4a, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00436504, FUN_00436504, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

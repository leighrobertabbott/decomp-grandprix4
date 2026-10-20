#include <gp4/gp4.h>
#include <gp4/flags.h>

// Skips the run of ASCII digits at esi. The original accumulates a decimal value but its loop jumps
// back to the `mov eax, 0` before the load, so eax is always 0 on return; ebx and edx are saved and
// restored. esi ends on the first non-digit byte; the flags are those of the exit compare
// (`cmp ebx, 0x30` for a byte below '0', else `cmp ebx, 0x39`).
// FUNCTION: GP4 0x0054d834
void __cdecl FUN_0054d834(gp4::Regs* r) {
    const uint8_t* cursor = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t arith;
    for (;;) {
        const uint32_t ch = *cursor;
        if (ch < 0x30u) {
            arith = gp4::flags::sub32(ch, 0x30u);
            break;
        }
        if (ch > 0x39u) {
            arith = gp4::flags::sub32(ch, 0x39u);
            break;
        }
        ++cursor;
    }
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(cursor));
    r->eax = 0u;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054d834, FUN_0054d834, "regs(esi:ptr[0x40]:u8{=0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x00,0x20,0x2f,0x3a,0x41}) -> (eax, esi, cf, pf, af, zf, sf, of)")

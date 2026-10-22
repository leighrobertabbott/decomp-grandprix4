#include <gp4/gp4.h>
#include <gp4/flags.h>

// Restores the saved state blocks: when the byte [0x00830894] is nonzero, the blocks stored one
// after the other from 0x00830895 are copied back to the destinations listed in the table at
// 0x00830ac8 (8-byte entries: destination pointer, length; a zero length ends the table), then the
// byte is cleared. eax, ebx, ecx, esi and edi are pushed and popped, so only the flags change (the destinations and the length are state): they are
// those of the last `cmp ..., 0` (zero against zero on both paths).
// FUNCTION: GP4 0x00541dfe
void __cdecl FUN_00541dfe(gp4::Regs* r) {
    const int32_t step = ((r->eflags & 0x400u) != 0u) ? -1 : 1;
    if (GP4_GLOBAL(uint8_t, 0x00830894) != 0u) {
        const uint8_t* source = &GP4_GLOBAL(uint8_t, 0x00830895);
        const uint32_t* entry = reinterpret_cast<const uint32_t*>(&GP4_GLOBAL(uint8_t, 0x00830ac8));
        while (entry[1] != 0u) {
            uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(entry[0]));
            for (uint32_t count = entry[1]; count != 0u; --count) {
                *dest = *source;
                dest += step;
                source += step;
            }
            entry += 2;
        }
        GP4_GLOBAL(uint8_t, 0x00830894) = 0u;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0u, 0u));
}
GP4_IMPL(0x00541dfe, FUN_00541dfe, "regs() -> (cf, pf, af, zf, sf, of); gfields={0x00830894:u8{=0,1,0xff}, 0x00830895*563/1:u8}")

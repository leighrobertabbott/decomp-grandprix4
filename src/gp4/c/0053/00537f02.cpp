#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00537f02
// Debug fill of a buffer. DAT_00829ba3 is a mode byte: 0 = nothing; otherwise the buffer start
// DAT_00829ba4 is copied to DAT_00829bac, and when the mode is exactly 1 the bytes
// [DAT_00829ba4, DAT_00829ba8) are filled with 0xa5 (rep stosb).
// Register effects: eax = start (low byte 0xa5 after the fill), ecx = 0 and edi = end after the
// fill. The flags are those of the last compare (cmp mode,0 / cmp mode,1 / sub ecx, start).
void __cdecl FUN_00537f02(gp4::Regs* r) {
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x00829ba3);
    uint32_t f = gp4::flags::sub8(mode, 0u);
    if (mode != 0u) {
        const uint32_t start = GP4_GLOBAL(uint32_t, 0x00829ba4);
        GP4_GLOBAL(uint32_t, 0x00829bac) = start;
        r->eax = start;
        f = gp4::flags::sub8(mode, 1u);
        if (mode == 1u) {
            const uint32_t end = GP4_GLOBAL(uint32_t, 0x00829ba8);
            const uint32_t count = end - GP4_GLOBAL(uint32_t, 0x00829ba4);
            f = gp4::flags::sub32(end, GP4_GLOBAL(uint32_t, 0x00829ba4));
            uint8_t* p = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start));
            for (uint32_t i = 0; i < count; ++i) {
                p[i] = 0xa5u;
            }
            r->eax = (start & 0xffffff00u) | 0xa5u;
            r->ecx = 0u;
            r->edi = start + count;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x00537f02, FUN_00537f02, "regs() -> (eax, ecx, edi, cf, pf, af, zf, sf, of); globals=layout=dlog_00537f02")

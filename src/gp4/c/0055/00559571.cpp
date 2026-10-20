#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the 0x160-byte ring record that precedes [0x00866f00] over the record that precedes
// [0x00891500] (`rep movsb`, forward). edi and esi end at the two pointers, ecx = 0 and the flags are
// those of `sub edi, 0x160`.
// FUNCTION: GP4 0x00559571
void __cdecl FUN_00559571(gp4::Regs* r) {
    const uint32_t dst_end = GP4_GLOBAL(uint32_t, 0x00891500);
    const uint32_t src_end = GP4_GLOBAL(uint32_t, 0x00866f00);
    const uint8_t* src = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(src_end - 0x160u));
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(dst_end - 0x160u));
    for (uint32_t i = 0; i != 0x160u; ++i) {
        dst[i] = src[i];
    }
    r->edi = dst_end;
    r->esi = src_end;
    r->ecx = 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(dst_end, 0x160u));
}
GP4_IMPL(0x00559571, FUN_00559571, "regs() -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_00559571")

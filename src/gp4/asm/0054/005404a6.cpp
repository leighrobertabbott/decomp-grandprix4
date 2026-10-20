#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stores cl at [edi], then copies the 0x40 bytes at 0x0083055f to [edi + 1 .. edi + 0x40]
// (`rep movsb`). Leaves ecx = 0, esi = 0x0083059f, edi = edi + 0x41; the flags are those of the
// `inc edi` (CF kept).
// FUNCTION: GP4 0x005404a6
void __cdecl FUN_005404a6(gp4::Regs* r) {
    const uint32_t start = r->edi;
    uint8_t* dst = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(start));
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x0083055f);

    dst[0] = static_cast<uint8_t>(r->ecx);
    for (uint32_t i = 0; i < 0x40u; ++i) {
        dst[1 + i] = src[i];
    }

    r->ecx = 0u;
    r->esi = 0x0083055fu + 0x40u;
    r->edi = start + 0x41u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(start, r->eflags));
}
GP4_IMPL(0x005404a6, FUN_005404a6, "regs(ecx:u32, edi:ptr[0x48]:bytes) -> (ecx, esi, edi, pf, af, zf, sf, of); globals=fuzz:i32")

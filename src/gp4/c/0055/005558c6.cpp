#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// pushal wrapper for the two side-line distance steps of a ring record (edi): 0x00555636 and
// 0x0055577e each return a distance in eax (an int3 assertion when it exceeds 0xf000) and the
// scale ebp; the distance goes to the word +0x74 (+0x76) and the scale's low word to +0x94 (+0x96).
// 0x00555474 runs between them and 0x00555555 last. popal restores every GPR; the flags are those
// the last callee leaves.
// FUNCTION: GP4 0x005558c6
void __cdecl FUN_005558c6(gp4::Regs* r) {
    uint8_t* const rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x00555636, c);
    if (c.eax > 0xf000u) {
        __debugbreak();
    }
    GP4_FIELD(uint16_t, rec, 0x74) = static_cast<uint16_t>(c.eax);
    GP4_FIELD(uint16_t, rec, 0x94) = static_cast<uint16_t>(c.ebp);

    gp4::call_regs(0x00555474, c);
    gp4::call_regs(0x0055577e, c);
    if (c.eax > 0xf000u) {
        __debugbreak();
    }
    GP4_FIELD(uint16_t, rec, 0x76) = static_cast<uint16_t>(c.eax);
    GP4_FIELD(uint16_t, rec, 0x96) = static_cast<uint16_t>(c.ebp);

    gp4::call_regs(0x00555555, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005558c6, FUN_005558c6, "regs(edi:ptr[0x158]:i16[-200..200], ecx:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:f64+layout=multiply_0040ce7b")

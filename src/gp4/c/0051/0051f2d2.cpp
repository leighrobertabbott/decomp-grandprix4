#include <gp4/gp4.h>
#include <gp4/flags.h>

// The 8-byte record at 0x0094c004: a dword and two words.
struct Unknown_0094c004 {
    uint32_t Dword00;
    uint16_t Word04;
    uint16_t Word06;
};

// Returns the record in edx:eax: eax = the dword at +0, edx = word +6 << 16 | word +4. ebx is
// saved and restored; the flags are those of the final `or edx, eax`.
// FUNCTION: GP4 0x0051f2d2
void __cdecl FUN_0051f2d2(gp4::Regs* r) {
    const Unknown_0094c004* rec = GP4_ARRAY(Unknown_0094c004, 0x0094c004);
    uint32_t edx = static_cast<uint32_t>(rec->Word06) << 16;
    const uint32_t low = rec->Word04;
    edx = edx | low;
    r->edx = edx;
    r->eax = rec->Dword00;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(edx));
}
GP4_IMPL(0x0051f2d2, FUN_0051f2d2, "regs() -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=rec_0051f2d2")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Initialises a two-dword record at ebx: [ebx] = 0x81800700 and bit 4 of [ebx+4] is cleared.
// The flags are those of the final `and dword [ebx+4], 0xffffffef` (CF = OF = AF = 0, ZF/SF/PF
// from the stored result).
// FUNCTION: GP4 0x00531716
void __cdecl FUN_00531716(gp4::Regs* r) {
    uint32_t* rec = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->ebx));
    rec[0] = 0x81800700u;
    const uint32_t result = rec[1] & 0xffffffefu;
    rec[1] = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result));
}
GP4_IMPL(0x00531716, FUN_00531716, "regs(ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")

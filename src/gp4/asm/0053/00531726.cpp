#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets a record at ebx: [ebx+0x10] = 0 and bits 0x14 of [ebx+0x14] are cleared (and ~0x14).
// The flags are those of the final `and dword [ebx+0x14], 0xffffffeb` (CF = OF = AF = 0, ZF/SF/PF
// from the stored result).
// FUNCTION: GP4 0x00531726
void __cdecl FUN_00531726(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint32_t* f10 = reinterpret_cast<uint32_t*>(rec + 0x10);
    uint32_t* f14 = reinterpret_cast<uint32_t*>(rec + 0x14);
    *f10 = 0u;
    const uint32_t result = *f14 & 0xffffffebu;
    *f14 = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(result));
}
GP4_IMPL(0x00531726, FUN_00531726, "regs(ebx:ptr[0x40]:bytes) -> (cf, pf, af, zf, sf, of)")

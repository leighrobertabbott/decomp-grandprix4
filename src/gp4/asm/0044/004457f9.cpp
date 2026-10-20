#include <gp4/gp4.h>

// Copies the record at esi to edi ([0x0062f8b8] bytes, whole dwords: rep movsd), then runs 0x0040b0da on the
// copy (esi = the original edi), which rebases its pointer fields and leaves the flags. ecx ends 0, esi = the
// original edi, edi = the end of the copy.
// FUNCTION: GP4 0x004457f9
void __cdecl FUN_004457f9(gp4::Regs* r) {
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062f8b8) >> 2;
    const uint32_t* source = reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t* dest = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t copyStart = r->edi;

    for (uint32_t i = 0; i < count; ++i) {
        dest[i] = source[i];
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.esi = copyStart;
    c.edi = copyStart + count * 4u;
    c.ecx = 0u;
    gp4::call_regs(0x0040b0da, c);

    r->ecx = 0u;
    r->esi = copyStart;
    r->edi = copyStart + count * 4u;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x004457f9, FUN_004457f9, "regs(esi:ptr[0x41c]:bytes, edi:ptr[0x41c]:bytes) -> (ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=len_0062f8b8")

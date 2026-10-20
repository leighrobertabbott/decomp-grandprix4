#include <gp4/gp4.h>
#include <gp4/flags.h>

// Writes the clamped span of the stream buffer [0x007a9148, 0x007a914c) to the record at edi as two
// dwords (start, end), the span clamped to 0x1c200 bytes (the end becomes start + 0x1c200), and
// runs the command-0x4f wrapper 0x00510fa3 with ecx = the span length and esi = the start. edi
// then advances past the two dwords and by the span length; edx = 0. The registers the wrapper
// leaves are kept; the flags are those of the final `add edi, ecx`.
// FUNCTION: GP4 0x00512eef
void __cdecl FUN_00512eef(gp4::Regs* r) {
    uint32_t ecx = GP4_GLOBAL(uint32_t, 0x007a914c);
    const uint32_t esi = GP4_GLOBAL(uint32_t, 0x007a9148);
    ecx -= esi;

    uint32_t edi = r->edi;
    GP4_FIELD(uint32_t, edi, 0) = GP4_GLOBAL(uint32_t, 0x007a9148);
    edi += 4u;

    uint32_t eax = GP4_GLOBAL(uint32_t, 0x007a914c);
    if (ecx >= 0x1c200u) {
        ecx = 0x1c200u;
        eax = GP4_GLOBAL(uint32_t, 0x007a9148) + ecx;
    }
    GP4_FIELD(uint32_t, edi, 0) = eax;
    edi += 4u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = eax;
    c.ecx = ecx;
    c.edx = 0u;
    c.esi = esi;
    c.edi = edi;
    gp4::call_regs(0x00510fa3, c);

    const uint32_t eflags = r->eflags;
    const uint32_t arith = gp4::flags::add32(c.edi, c.ecx);
    *r = c;
    r->edi = c.edi + c.ecx;
    r->eflags = gp4::flags::merge(eflags, arith);
}
GP4_IMPL(0x00512eef, FUN_00512eef, "regs(edi:ptr[0x10]:bytes) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

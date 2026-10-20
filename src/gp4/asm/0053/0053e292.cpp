#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dialog record 0x84c830 (caller: 0x54fccd). Stores ecx at +0x14, eax into the dword at +0x12 of the
// block behind the pointer at +0, edx into the dword at +8 of the block behind the pointer at +0x18;
// runs the layout routine 0x53e3e9 with eax = 1 (ebx passes through to it) and then writes the four
// words x1 = 0x140, y1 = [0x82feb8] + [0x82fec0] + 0x48, x2 = 0x1df, y2 = y1 + 0x17 into the first
// block (+4..+0xb). The registers the layout routine leaves are kept except esi/ebx/eax, which are
// overwritten; the flags are those of the last `add eax, 0x17`.
// FUNCTION: GP4 0x0053e292
void __cdecl FUN_0053e292(gp4::Regs* r) {
    const uint32_t record = 0x0084c830u;

    GP4_FIELD(uint32_t, record, 0x14) = r->ecx;
    uint32_t block = GP4_FIELD(uint32_t, record, 0);
    GP4_FIELD(uint32_t, block, 0x12) = r->eax;
    uint32_t text = GP4_FIELD(uint32_t, record, 0x18);
    GP4_FIELD(uint32_t, text, 8) = r->edx;

    gp4::Regs c = *r;
    c.edi = record;
    c.esi = text;
    c.eax = 1u;
    gp4::call_regs(0x0053e3e9, c);

    const uint32_t edi = c.edi;
    const uint32_t esi = GP4_FIELD(uint32_t, edi, 0);
    uint32_t ebx = 0x190u - 0x50u;
    uint32_t eax = GP4_GLOBAL(uint32_t, 0x0082feb8);
    eax += GP4_GLOBAL(uint32_t, 0x0082fec0);
    eax += 0x48u;
    GP4_FIELD(uint16_t, esi, 4) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 6) = static_cast<uint16_t>(eax);
    ebx += 0x9fu;
    const uint32_t flags = gp4::flags::add32(eax, 0x17u);
    eax += 0x17u;
    GP4_FIELD(uint16_t, esi, 8) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 0xa) = static_cast<uint16_t>(eax);

    const uint32_t eflags = r->eflags;
    *r = c;
    r->eax = eax;
    r->ebx = ebx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(eflags, flags);
}
GP4_IMPL(0x0053e292, FUN_0053e292, "regs(eax:u32, ebx:u32, ecx:u32[0..0x3ff], edx:u32[0..0x3ff]) -> (eax, ebx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=tab_00835846+dlg_0053e3e9")

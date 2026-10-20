#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dialog record 0x84c93e, three-button layout (caller: 0x54fccd; sibling of the two-button
// 0x0053e2e1). Stores ecx at +0x14, eax into the dword at +0x12 of the block A behind the pointer at
// +0, ebx into the dword at +0x12 of the block B behind A's first dword, ebp into the dword at +0x12
// of the block C behind B's first dword, edx into the dword at +8 of the block behind the pointer at
// +0x18; runs the layout routine 0x53e3e9 with eax = 3 (ebx passes through to it), then writes the
// rectangle x1 = 0xa0, y1 = [0x82feb8] + [0x82fec0] + 0x48, x2 = 0x13c, y2 = y1 + 0x17 into A
// (+4..+0xb), x1 = 0x144, x2 = 0x1dc into B and x1 = 0x1e4, x2 = 0x280 into C. The registers the
// layout routine leaves are kept except eax/ebx/ecx/esi; the flags are those of the last
// `add ebx, 0x9c`.
// FUNCTION: GP4 0x0053e355
void __cdecl FUN_0053e355(gp4::Regs* r) {
    const uint32_t record = 0x0084c93eu;

    GP4_FIELD(uint32_t, record, 0x14) = r->ecx;
    uint32_t a = GP4_FIELD(uint32_t, record, 0);
    GP4_FIELD(uint32_t, a, 0x12) = r->eax;
    uint32_t b = GP4_FIELD(uint32_t, a, 0);
    GP4_FIELD(uint32_t, b, 0x12) = r->ebx;
    uint32_t c3 = GP4_FIELD(uint32_t, b, 0);
    GP4_FIELD(uint32_t, c3, 0x12) = r->ebp;
    uint32_t text = GP4_FIELD(uint32_t, record, 0x18);
    GP4_FIELD(uint32_t, text, 8) = r->edx;

    gp4::Regs c = *r;
    c.edi = record;
    c.esi = text;
    c.eax = 3u;
    gp4::call_regs(0x0053e3e9, c);

    const uint32_t edi = c.edi;
    uint32_t esi = GP4_FIELD(uint32_t, edi, 0);
    uint32_t ebx = 0x190u - 0xf0u;
    uint32_t eax = GP4_GLOBAL(uint32_t, 0x0082feb8);
    eax += GP4_GLOBAL(uint32_t, 0x0082fec0);
    eax += 0x48u;
    GP4_FIELD(uint16_t, esi, 4) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 6) = static_cast<uint16_t>(eax);
    ebx += 0x9cu;
    uint32_t ecx = eax + 0x17u;
    GP4_FIELD(uint16_t, esi, 8) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 0xa) = static_cast<uint16_t>(ecx);
    esi = GP4_FIELD(uint32_t, esi, 0);
    ebx += 8u;
    GP4_FIELD(uint16_t, esi, 4) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 6) = static_cast<uint16_t>(eax);
    ebx += 0x98u;
    GP4_FIELD(uint16_t, esi, 8) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 0xa) = static_cast<uint16_t>(ecx);
    esi = GP4_FIELD(uint32_t, esi, 0);
    ebx += 8u;
    GP4_FIELD(uint16_t, esi, 4) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 6) = static_cast<uint16_t>(eax);
    const uint32_t flags = gp4::flags::add32(ebx, 0x9cu);
    ebx += 0x9cu;
    GP4_FIELD(uint16_t, esi, 8) = static_cast<uint16_t>(ebx);
    GP4_FIELD(uint16_t, esi, 0xa) = static_cast<uint16_t>(ecx);

    const uint32_t eflags = r->eflags;
    *r = c;
    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(eflags, flags);
}
GP4_IMPL(0x0053e355, FUN_0053e355, "regs(eax:u32, ebx:u32, ebp:u32, ecx:u32[0..0x3ff], edx:u32[0..0x3ff]) -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=tab_00835846")

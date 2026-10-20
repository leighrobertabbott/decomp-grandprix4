#include <gp4/gp4.h>
#include <gp4/x87.h>
#include <gp4/flags.h>

// Dword of the table at 0x868598 at byte offset `offset` (ebx).
static __declspec(noinline) uint32_t tableDword(uint32_t offset) {
    return *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(0x00868598u + offset));
}

// FUNCTION: GP4 0x00553b79
// Sibling of 0x00553b21 for the word at +0x62: the vector from 0x00553a5e takes the length
// (word[+0x62] + 0x50 + table[ebx]) << 8 (no negation) and the product uses the negated factor
// -((word + 0x50) << 8). ecx = round(dword[+0x150] * f64[0x6f2a04] * factor) + dword[+0xf4]
// with the flags of that add; eax leaves holding the factor and edx the y component.
void __cdecl FUN_00553b79(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t word = GP4_FIELD(uint16_t, record, 0x62);

    const uint32_t length = (word + 0x50u + tableDword(r->ebx)) << 8;
    gp4::Regs c = {};
    c.eax = length;
    c.edi = r->edi;
    gp4::call_regs(0x00553a5e, c);
    GP4_GLOBAL(uint32_t, 0x00934df0) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00934df4) = c.edx;

    const int32_t slope = GP4_FIELD(int32_t, record, 0x150);
    const uint32_t factor = 0u - ((word + 0x50u) << 8);
    GP4_GLOBAL(uint32_t, 0x006f2bf4) = factor;
    const gp4::x87::Ext product =
        (gp4::x87::ext_i32(slope) * gp4::x87::ext(GP4_GLOBAL(double, 0x006f2a04))) *
        gp4::x87::ext_i32(GP4_GLOBAL(int32_t, 0x006f2bf4));
    const int32_t scaled = gp4::x87::fistp32(product);
    GP4_GLOBAL(int32_t, 0x006f2bf4) = scaled;

    const uint32_t height = GP4_FIELD(uint32_t, record, 0xf4);
    r->eax = factor;
    r->edx = c.edx;
    r->ecx = static_cast<uint32_t>(scaled) + height;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(static_cast<uint32_t>(scaled), height));
}
GP4_IMPL(0x00553b79, FUN_00553b79, "regs(ebx:u32[0..0x1fc], edi:ptr[0x15c]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=tbl_00868598+multiply_0040ce7b")

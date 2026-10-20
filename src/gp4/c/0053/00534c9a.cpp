#include <gp4/gp4.h>
#include <gp4/flags.h>

// Marks one cell of the 2 x 6 grid of 0x10-byte entries at 0x007ab02c; ebx points at the request record
// (+4 row, +8 column, +0xc byte offset), cell = row * 6 + column. When the gate dword [0x007a979c] is zero
// the cell number just goes to [0x00828170] (ecx = cell, flags of the final `add`). Otherwise the dword at
// the entry is copied to entry + offset, bit `cell` is set in the masks [0x007ab0ec] and [0x007a9798], the
// (unaligned) counter [0x00828293] is incremented and the item chain flag walk 0x0053e4db (eax = 1, head
// 0x00852837) and the next-selectable-item search 0x0053cc19 run; registers and flags are those the last
// callee leaves (eax = 1).
// FUNCTION: GP4 0x00534c9a
void __cdecl FUN_00534c9a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t record = c.ebx;
    if (GP4_GLOBAL(uint32_t, 0x007a979c) == 0u) {
        const uint32_t row6 = GP4_FIELD(uint32_t, record, 4) * 6u;
        const uint32_t column = GP4_FIELD(uint32_t, record, 8);
        c.ecx = row6 + column;
        GP4_GLOBAL(uint32_t, 0x00828170) = c.ecx;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(row6, column));
    } else {
        const uint32_t cell = GP4_FIELD(uint32_t, record, 4) * 6u + GP4_FIELD(uint32_t, record, 8);
        const uint32_t entry = cell * 0x10u + 0x007ab02cu;
        c.ecx = entry;
        c.edx = entry + GP4_FIELD(uint32_t, record, 0xc);
        c.eax = GP4_FIELD(uint32_t, entry, 0);
        GP4_FIELD(uint32_t, c.edx, 0) = c.eax;
        c.ecx = cell;
        c.eax = 1u << (cell & 0x1fu);
        GP4_GLOBAL(uint32_t, 0x007ab0ec) |= c.eax;
        GP4_GLOBAL(uint32_t, 0x007a9798) |= c.eax;
        GP4_GLOBAL(uint32_t, 0x00828293) += 1u;
        c.eax = 1u;
        c.esi = 0x00852837u;
        gp4::call_regs(0x0053e4db, c);
        gp4::call_regs(0x0053cc19, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00534c9a, FUN_00534c9a, "regs(ebx:ptr[0x10]:u32[0..11]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0084300a+sel_0053cc19+head_00852837")

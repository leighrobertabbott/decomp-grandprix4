#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream handler: three words at esi (esi += 6) = kind, row count, value. kind 0x8000 selects the
// word field +0xb0 and the flag bit 0x20 of the segment record, any other kind +0xb2 and bit 0x10.
// FUN_005583a3 first gets (bit & dword [ebx + 0x2c]) for the record ebx. With a zero count that is
// all (eax = value, ecx = 0, flags of `or ecx, ecx`). Otherwise the value word is stored into the
// field of `count` consecutive 0x160-byte records starting at ebx (wrapping by [0x866ebc] once the
// address reaches the ring end [0x866eb8]) and FUN_005583a3 gets (bit & dword [last + 0x2c]) for the
// record the walk ended on. ebx and edi are saved and restored; edx = the field offset, ebp = the
// bit, ecx = 0 and the flags are those of the last FUN_005583a3.
// FUNCTION: GP4 0x005587a0
void __cdecl FUN_005587a0(gp4::Regs* r) {
    const uint32_t stream = r->esi;
    uint32_t field = 0xb0u;
    uint32_t bit = 0x20u;
    if (GP4_FIELD(uint16_t, stream, 0) != 0x8000u) {
        field = 0xb2u;
        bit = 0x10u;
    }
    uint32_t record = r->ebx;
    {
        gp4::Regs first = *r;
        first.eax = bit & GP4_FIELD(uint32_t, record, 0x2c);
        first.edi = record;
        gp4::call_regs(0x005583a3, first);
    }
    uint32_t count = GP4_FIELD(uint16_t, stream, 2);
    const uint32_t value = GP4_FIELD(uint16_t, stream, 4);
    r->esi = stream + 6u;
    r->edx = field;
    r->ebp = bit;
    r->ecx = 0u;
    r->eax = value;
    if (count == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }
    do {
        GP4_FIELD(uint16_t, record, field) = static_cast<uint16_t>(value);
        record += 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            record -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        --count;
    } while (count != 0u);
    gp4::Regs last = *r;
    last.eax = bit & GP4_FIELD(uint32_t, record, 0x2c);
    last.edi = record;
    gp4::call_regs(0x005583a3, last);
    r->eax = last.eax;
    r->eflags = gp4::flags::merge(r->eflags, last.eflags);
}
GP4_IMPL(0x005587a0, FUN_005587a0, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:u16{=0x8000,0,1,2,3,5,8,24,0x1234}) -> (eax, ecx, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+wave_005587a0")

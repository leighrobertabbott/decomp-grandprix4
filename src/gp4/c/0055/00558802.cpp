#include <gp4/gp4.h>
#include <gp4/flags.h>

// Stream handler for mode 2 (byte [0x0086857c]; any other mode only does esi += 6, with the flags of
// that add). Three words at esi (esi += 6) = kind, row count, value. kind 0x8000 selects the word
// field +0x8e and the flag bit 0x100000 of the segment record, any other kind +0x90 and 0x80000.
// FUN_005583a3 first gets the bit for the record ebx. With a zero count that is all (eax = value,
// ecx = 0, flags of `or ecx, ecx`). Otherwise for `count` consecutive 0x160-byte records from ebx
// (wrapping by [0x866ebc] once the address reaches the ring end [0x866eb8]) the value word is stored
// into the field and the bit is or-ed into the dword +0x2c; the value is stored once more into the
// record the walk ended on and FUN_005583a3 gets the bit for it. ebx and edi are saved and restored;
// edx = the field offset, ebp = the bit, ecx = 0 and the flags are those of the last
// FUN_005583a3.
// FUNCTION: GP4 0x00558802
void __cdecl FUN_00558802(gp4::Regs* r) {
    const uint32_t stream = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 2u) {
        r->esi = stream + 6u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(stream, 6u));
        return;
    }
    uint32_t field = 0x8eu;
    uint32_t bit = 0x100000u;
    if (GP4_FIELD(uint16_t, stream, 0) != 0x8000u) {
        field = 0x90u;
        bit = 0x80000u;
    }
    uint32_t record = r->ebx;
    {
        gp4::Regs first = *r;
        first.eax = bit;
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
        GP4_FIELD(uint32_t, record, 0x2c) |= bit;
        record += 0x160u;
        if (record >= GP4_GLOBAL(uint32_t, 0x00866eb8)) {
            record -= GP4_GLOBAL(uint32_t, 0x00866ebc);
        }
        --count;
    } while (count != 0u);
    GP4_FIELD(uint16_t, record, field) = static_cast<uint16_t>(value);
    gp4::Regs last = *r;
    last.eax = bit;
    last.edi = record;
    gp4::call_regs(0x005583a3, last);
    r->eax = last.eax;
    r->eflags = gp4::flags::merge(r->eflags, last.eflags);
}
GP4_IMPL(0x00558802, FUN_00558802, "regs(ebx:rec[0x8943a0,0x160,0..29], esi:ptr[8]:u16{=0x8000,0,1,2,3,5,8,24,0x1234}) -> (eax, ecx, edx, ebp, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+mode_0086857c+wave_005587a0")

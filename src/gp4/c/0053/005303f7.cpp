#include <gp4/gp4.h>
#include <gp4/flags.h>

// Render-state record selected by 0x00531803 (0x38 bytes): a qword at +0 whose copy goes to +8 when
// byte +0x1d is 0xff (zero otherwise), a flag dword at +0x10 and one at +0x14 whose bit 1 mirrors
// whether any of the bits 0x99e7ff3c of +0x10 is set.
struct Record {
    uint32_t Low00;
    uint32_t High04;
    uint32_t Low08;
    uint32_t High0c;
    uint32_t Mask10;
    uint32_t Flags14;
    uint8_t Unknown18[5];
    uint8_t Marker1d;
    uint8_t Unknown1e[0x1a];
};

// `pushal ; call 0x531803 ; ... ; popal`: 0x531803 selects the current record (ebx). Bit 1 of the
// dword +0x14 is set when +0x10 has any bit of 0x99e7ff3c, else cleared. The qword at +0 is copied to
// +8 (or zeroed when the byte +0x1d is not 0xff), then the 0x38 record bytes are copied to
// 0x007ad7f8 (`rep movsb`) and 0x00530012 refreshes the reciprocal. popal restores every general
// register; the flags are those 0x00530012 leaves.
// FUNCTION: GP4 0x005303f7
void __cdecl FUN_005303f7(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00531803, c);

    Record* record = reinterpret_cast<Record*>(static_cast<uintptr_t>(c.ebx));
    if ((record->Mask10 & 0x99e7ff3cu) == 0u) {
        record->Flags14 &= 0xfffffffdu;
    } else {
        record->Flags14 |= 2u;
    }

    uint32_t low = record->Low00;
    uint32_t high = record->High04;
    if (record->Marker1d != 0xffu) {
        low = 0u;
        high = 0u;
    }
    record->Low08 = low;
    record->High0c = high;

    const uint8_t* src = reinterpret_cast<const uint8_t*>(record);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007ad7f8);
    for (uint32_t i = 0u; i != 0x38u; ++i) {
        dst[i] = src[i];
    }

    c.eax = low;
    c.edx = high;
    c.esi = c.ebx + 0x38u;
    c.edi = 0x007ad7f8u + 0x38u;
    c.ecx = 0u;
    gp4::call_regs(0x00530012, c);

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005303f7, FUN_005303f7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00531859+rsrec_007ad638")

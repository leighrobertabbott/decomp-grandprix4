#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic operation (`and` / `or` on a byte): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_004037a9_Record {
    uint8_t Unknown00[0x1c];
    uint32_t Unknown1C;
    uint8_t Unknown20[0x66 - 0x20];
    int16_t Unknown66;
    uint8_t Unknown68[0xfe - 0x68];
    uint8_t UnknownFE;
    uint8_t UnknownFF[0x1a4 - 0xff];
    uint32_t Unknown1A4;
    uint8_t Unknown1A8[0x272 - 0x1a8];
    uint8_t Unknown272;
};

// FUNCTION: GP4 0x004037a9
// esi = record. Temporarily replaces the global 0x00601b34 with the signed word at +0x66, then
// classifies the position +0x1c (0x00403a1e -> byte +0x272), runs the mode adjustment of eax
// (0x00403938, edi = +0x1c) and the ratio computation (0x00403ab9). The ratio goes to +0x1a4 and
// bit 7 of +0xfe is set when the ratio call returned edx = -1, else cleared. eax and edx are the
// outputs of 0x00403ab9; ecx, edi and ebp are preserved; the flags come from the or / and.
void __cdecl FUN_004037a9(gp4::Regs* r) {
    Unknown_004037a9_Record* record = reinterpret_cast<Unknown_004037a9_Record*>(static_cast<uintptr_t>(r->esi));

    const int32_t saved = GP4_GLOBAL(int32_t, 0x00601b34);
    GP4_GLOBAL(int32_t, 0x00601b34) = static_cast<int32_t>(record->Unknown66);
    const uint32_t position = record->Unknown1C;

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.eax = position;
    gp4::call_regs(0x00403a1e, call);
    record->Unknown272 = static_cast<uint8_t>(call.eax);

    call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edx = record->Unknown272;
    call.edi = position;
    gp4::call_regs(0x00403938, call);

    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(0x00403ab9, call);
    record->Unknown1A4 = call.eax;

    uint8_t flagsByte;
    if (call.edx == 0xffffffffu) {
        flagsByte = static_cast<uint8_t>(record->UnknownFE | 0x80u);
    } else {
        flagsByte = static_cast<uint8_t>(record->UnknownFE & 0x7fu);
    }
    record->UnknownFE = flagsByte;
    GP4_GLOBAL(int32_t, 0x00601b34) = saved;

    r->eax = call.eax;
    r->edx = call.edx;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(flagsByte);
}
GP4_IMPL(0x004037a9, FUN_004037a9, "regs(eax:i32[-40000..40000], ecx:i32[-40000..40000], esi:ptr[0x420]:layout=rec_00409416) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800")

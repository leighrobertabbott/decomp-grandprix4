#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic operation (and / or on a byte): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// esi = car-like record; [esi+0x1c] = its track-segment record (the next segment is 0x160 bytes
// further: words +0x16 / +0x176 are the pair interpolated by the fraction word [esi+0x66]). With
// DAT_00601b34 temporarily set to that fraction it runs
//   0x00403721 (eax = byte DAT_00601ba5, edi = segment) -> byte [esi+0x294] and the mode,
//   0x00403938 (eax = the interpolated word, edx = mode),
//   0x00403bb6 (ecx = word [esi+0x90]) -> dword [esi+0x1a0],
// and sets bit 3 of byte [esi+0x71] when 0x00403bb6 left edx == -1, clears it otherwise. All registers
// and DAT_00601b34 are restored (pushal / popal); only the flags of the byte and/or survive.
// FUNCTION: GP4 0x0040373b
void __cdecl FUN_0040373b(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedFraction = GP4_GLOBAL(uint32_t, 0x00601b34);

    const int32_t fraction = GP4_FIELD(int16_t, record, 0x66);
    GP4_GLOBAL(int32_t, 0x00601b34) = fraction;
    const uint32_t segment = GP4_FIELD(uint32_t, record, 0x1c);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = GP4_GLOBAL(uint8_t, 0x00601ba5);
    c.edi = segment;
    gp4::call_regs(0x00403721, c);
    GP4_FIELD(uint8_t, record, 0x294) = static_cast<uint8_t>(c.eax);
    const uint32_t mode = c.eax;

    uint8_t* block = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment));
    const int32_t high = GP4_FIELD(int16_t, block, 0x176);
    const int32_t low = GP4_FIELD(int16_t, block, 0x16);
    const int64_t product = static_cast<int64_t>(high - low) * static_cast<int64_t>(fraction);
    c.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) + static_cast<uint32_t>(low);
    c.ecx = static_cast<uint32_t>(low);
    c.edx = mode;
    gp4::call_regs(0x00403938, c);

    c.ecx = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, record, 0x90)));
    gp4::call_regs(0x00403bb6, c);
    GP4_FIELD(uint32_t, record, 0x1a0) = c.eax;

    uint32_t flagsByte = GP4_FIELD(uint8_t, record, 0x71);
    if (c.edx == 0xffffffffu) {
        flagsByte |= 8u;
    } else {
        flagsByte &= 0xf7u;
    }
    GP4_FIELD(uint8_t, record, 0x71) = static_cast<uint8_t>(flagsByte);

    GP4_GLOBAL(uint32_t, 0x00601b34) = savedFraction;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags8(flagsByte);
}
GP4_IMPL(0x0040373b, FUN_0040373b, "regs(esi:ptr[0x420]:layout=rec_00409416) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+div_00403bb6")

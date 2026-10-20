#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the 16-bit `sub lhs, rhs`.
static inline uint32_t subFlags16(uint16_t lhs, uint16_t rhs) {
    const uint32_t result = static_cast<uint16_t>(lhs - rhs);
    return (lhs < rhs ? 1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of an 8-bit logic result (or): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Advances the track-segment record of one of two cars (esi when the word 0x00603e70 is negative,
// otherwise ebx; the pointer at car +0x1b8) by the byte offset 0x00603e9c, wrapping inside the lap
// ring (below the pit start: back by 0x00866ec4 once the lap end 0x00866ecc is reached) or clamping
// at the last pit record (0x00866ef8 - 0x160). The record's flag byte +0x51 is OR-ed into 0x006077c9
// and its word +0x16 is stored at 0x006077c7. Then 0x004035c3 is run for ebx's car (word +0x66) and
// for esi's car; their results go to 0x00603d9c (first) and 0x00604402 (second) and the 16-bit
// difference second - first to 0x00604404. eax = first, cx = that difference (high half of ecx =
// high half of the second result); ebx, edx, esi and ebp are
// whatever the second 0x004035c3 call leaves; edi is preserved.
// FUNCTION: GP4 0x0042b27b
void __cdecl FUN_0042b27b(gp4::Regs* r) {
    uint8_t* const carB = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* const carS = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t edi = (GP4_GLOBAL(int16_t, 0x00603e70) < 0) ? GP4_FIELD(uint32_t, carS, 0x1b8)
                                                           : GP4_FIELD(uint32_t, carB, 0x1b8);
    const uint32_t step = GP4_GLOBAL(uint32_t, 0x00603e9c);
    if (edi >= GP4_GLOBAL(uint32_t, 0x00891500)) {
        edi += step;
        if (edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    } else {
        edi += step;
        if (edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    }

    uint8_t* const segment = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi));
    const uint8_t merged = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x006077c9) | GP4_FIELD(uint8_t, segment, 0x51));
    GP4_GLOBAL(uint8_t, 0x006077c9) = merged;
    const uint16_t word = GP4_FIELD(uint16_t, segment, 0x16);
    GP4_GLOBAL(uint16_t, 0x006077c7) = word;

    gp4::Regs call = *r;
    call.edx = step;
    call.edi = edi;
    call.eflags = (r->eflags & ~0x8d5u) | logicFlags8(merged);
    call.ecx = (r->ecx & 0xffff0000u) | word;
    // xchg ebx, esi; movsx eax, word [esi+0x66]
    call.ebx = r->esi;
    call.esi = r->ebx;
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, carB, 0x66)));
    gp4::call_regs(0x004035c3, call);
    const uint32_t first = call.eax;

    // mov ecx, eax; xchg ebx, esi; movsx eax, word [esi+0x66]
    call.ecx = first;
    const uint32_t swapped = call.ebx;
    call.ebx = call.esi;
    call.esi = swapped;
    call.eax = static_cast<uint32_t>(static_cast<int32_t>(
        GP4_FIELD(int16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.esi)), 0x66)));
    gp4::call_regs(0x004035c3, call);
    const uint32_t second = call.eax;

    const uint16_t firstWord = static_cast<uint16_t>(first);
    const uint16_t secondWord = static_cast<uint16_t>(second);
    GP4_GLOBAL(uint16_t, 0x00604402) = secondWord;
    GP4_GLOBAL(uint16_t, 0x00603d9c) = firstWord;
    const uint16_t difference = static_cast<uint16_t>(secondWord - firstWord);
    GP4_GLOBAL(uint16_t, 0x00604404) = difference;

    r->eax = first;
    r->ebx = call.ebx;
    // sub cx, ax leaves the high half of ecx (the second result) in place
    r->ecx = (second & 0xffff0000u) | difference;
    r->edx = call.edx;
    r->esi = call.esi;
    r->ebp = call.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | subFlags16(secondWord, firstWord);
}
GP4_IMPL(0x0042b27b, FUN_0042b27b, "regs(ebx:ptr[0x29c]:layout=car_00431ffb+car_004035c3+car_0042b17e, esi:ptr[0x29c]:layout=car_00431ffb+car_004035c3+car_0042b17e, ecx:u32) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+step_0042b17e")

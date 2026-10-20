#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of the 16-bit `sub left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 8) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x8000u) >> 4);
}

struct Unknown_0042b318_Car {
    uint8_t Unknown00[0x1c];
    uint32_t Segment1C;
    uint8_t Unknown20[0x66 - 0x20];
    int16_t Word66;
    uint8_t Unknown68[0x1b8 - 0x68];
    uint32_t Segment1B8;
};

// FUNCTION: GP4 0x0042b318
// Looks up the lane word of two car records at the segments they stand on. ebx / esi are the two
// cars (swapped when the word 0x00603e70 is negative). The first segment is [ebx+0x1b8] advanced by
// [0x603e9c] with the ring wraps of the 0x160-byte segment array (as 0x0042b6c2); 0x00431ffb gives its
// word (stored at 0x006077c7), which 0x004035c3 turns into a value for the car at ebx's side
// (movsx [car+0x66] as the blend). The second pair is [ebx+0x1c] with the other car. The two values go
// to 0x00603d9c and 0x00604402 (exchanged back when 0x00603e70 is negative) and their difference to
// 0x00604404. Leaves eax / ecx = the two values, ebx / esi the (swapped) cars, edx / ebp from the last
// 0x004035c3, edi restored, and the flags of the final 16-bit `sub`.
void __cdecl FUN_0042b318(gp4::Regs* r) {
    uint32_t carB = r->ebx;
    uint32_t carS = r->esi;
    if ((GP4_GLOBAL(uint16_t, 0x00603e70) & 0x8000u) != 0u) {
        const uint32_t t = carB;
        carB = carS;
        carS = t;
    }

    uint32_t segment = reinterpret_cast<const Unknown_0042b318_Car*>(static_cast<uintptr_t>(carB))->Segment1B8;
    const uint32_t advance = GP4_GLOBAL(uint32_t, 0x00603e9c);
    if (segment < GP4_GLOBAL(uint32_t, 0x00891500)) {
        segment += advance;
        if (segment >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
            segment -= GP4_GLOBAL(uint32_t, 0x00866ec4);
        }
    } else {
        segment += advance;
        if (segment >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
            segment = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
        }
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.ebx = carB;
    c.esi = carB;
    c.edi = segment;
    c.edx = advance;
    gp4::call_regs(0x00431ffb, c);
    GP4_GLOBAL(uint16_t, 0x006077c7) = static_cast<uint16_t>(c.eax);
    c.ecx = c.eax;

    c.ebx = carS;
    c.esi = carB;
    c.edi = segment;
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(
        reinterpret_cast<const Unknown_0042b318_Car*>(static_cast<uintptr_t>(c.esi))->Word66));
    gp4::call_regs(0x004035c3, c);
    c.ecx = c.eax;
    const uint32_t first = c.eax;

    const uint32_t swap = c.ebx;
    c.ebx = c.esi;
    c.esi = swap;
    c.edi = reinterpret_cast<const Unknown_0042b318_Car*>(static_cast<uintptr_t>(c.ebx))->Segment1C;
    gp4::call_regs(0x00431ffb, c);
    c.eax = static_cast<uint32_t>(static_cast<int32_t>(
        reinterpret_cast<const Unknown_0042b318_Car*>(static_cast<uintptr_t>(c.esi))->Word66));
    gp4::call_regs(0x004035c3, c);
    c.ecx = c.eax;
    c.eax = first;

    if ((GP4_GLOBAL(uint16_t, 0x00603e70) & 0x8000u) != 0u) {
        const uint32_t t = c.esi;
        c.esi = c.ebx;
        c.ebx = t;
        const uint32_t lowCx = c.ecx & 0xffffu;
        c.ecx = (c.ecx & 0xffff0000u) | (c.eax & 0xffffu);
        c.eax = (c.eax & 0xffff0000u) | lowCx;
    }

    GP4_GLOBAL(uint16_t, 0x00604402) = static_cast<uint16_t>(c.ecx);
    GP4_GLOBAL(uint16_t, 0x00603d9c) = static_cast<uint16_t>(c.eax);
    const uint32_t flags = subFlags16(c.ecx & 0xffffu, c.eax & 0xffffu);
    c.ecx = (c.ecx & 0xffff0000u) | ((c.ecx - c.eax) & 0xffffu);
    GP4_GLOBAL(uint16_t, 0x00604404) = static_cast<uint16_t>(c.ecx);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042b318, FUN_0042b318, "regs(ebx:ptr[0x420]:layout=rec_0043569a+car_00431ffb+car_004035c3+car_0042b17e, esi:ptr[0x420]:layout=rec_0043569a+car_00431ffb+car_004035c3+car_0042b17e) -> (eax, ebx, ecx, edx, esi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+step_0042b17e")

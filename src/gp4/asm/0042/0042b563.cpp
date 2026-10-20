#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 16-bit `neg value`.
static inline uint32_t negFlags16(uint32_t value) {
    value &= 0xffffu;
    const uint32_t result = (0u - value) & 0xffffu;
    return (value != 0u ? 0x01u : 0u)
        | parityFlag8(result)
        | (((value ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (value == 0x8000u ? 0x800u : 0u);
}

// Car block (only the fields this routine reads).
struct Unknown_0042b563_Car {
    uint8_t Unknown00[0x8d];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Unknown90;
    uint8_t Unknown92[0x1ce - 0x92];
    uint8_t Flags1CE;
};

// Two separate call sites: MSVC would otherwise merge the identical calls into one with a computed target.
static __declspec(noinline) void callUnlinked(gp4::Regs& c) { gp4::call_regs(0x0042b60f, c); }
static __declspec(noinline) void callLinked(gp4::Regs& c) { gp4::call_regs(0x0042b6c2, c); }

// esi = this car, ebx = the other one. Bit 1 of [esi+0x8d] makes the other car the subject of the
// lookup (xchg ebx, esi); bit 0 of the subject's +0x1ce picks the helper: clear 0x0042b60f, set
// 0x0042b6c2. Both leave ax / cx as two words. Not linked: with the swap the pair is replaced by
// this car's lateral value [esi+0x90] (cx = [esi+0x90] - (cx - ax), ax = [esi+0x90]) and word
// 0x006077c7 is cleared; without the swap the pair is kept. Linked: without the swap the pair is
// replaced the same way, with the swap only word 0x006077c7 is cleared. Stores the pair in words
// 0x00604402 / 0x00603d9c and the negated difference in 0x00604404. ebx, esi and edi are restored;
// eax, ecx (low words), edx, ebp and the flags come from the helper and the final neg.
// FUNCTION: GP4 0x0042b563
void __cdecl FUN_0042b563(gp4::Regs* r) {
    const Unknown_0042b563_Car* self = reinterpret_cast<const Unknown_0042b563_Car*>(static_cast<uintptr_t>(r->esi));
    const bool swapped = (self->Flags8D & 2u) != 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    if (swapped) {
        c.esi = r->ebx;
        c.ebx = r->esi;
    }
    const Unknown_0042b563_Car* subject = reinterpret_cast<const Unknown_0042b563_Car*>(static_cast<uintptr_t>(c.esi));

    bool replace;
    bool clear;
    if ((subject->Flags1CE & 1u) == 0u) {
        callUnlinked(c);
        replace = swapped;
        clear = swapped;
    } else {
        callLinked(c);
        replace = !swapped;
        clear = swapped;
    }

    uint16_t ax = static_cast<uint16_t>(c.eax);
    uint16_t cx = static_cast<uint16_t>(c.ecx);
    if (clear) {
        GP4_GLOBAL(uint16_t, 0x006077c7) = 0u;
    }
    if (replace) {
        cx = static_cast<uint16_t>(0u - static_cast<uint16_t>(cx - ax));
        cx = static_cast<uint16_t>(cx + static_cast<uint16_t>(self->Unknown90));
        ax = static_cast<uint16_t>(self->Unknown90);
    }
    GP4_GLOBAL(uint16_t, 0x00604402) = ax;
    GP4_GLOBAL(uint16_t, 0x00603d9c) = cx;
    const uint16_t gap = static_cast<uint16_t>(cx - ax);
    const uint16_t negated = static_cast<uint16_t>(0u - gap);
    GP4_GLOBAL(uint16_t, 0x00604404) = negated;

    r->eax = (c.eax & 0xffff0000u) | ax;
    r->ecx = (c.ecx & 0xffff0000u) | negated;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (negFlags16(gap) & 0x8d5u);
}
GP4_IMPL(0x0042b563, FUN_0042b563, "regs(eax:u32, ecx:u32, edx:u32, ebx:ptr[0x29c]:layout=car_004035c3+car_00431ffb+car_0043c677+pos_00405cf8, esi:ptr[0x29c]:layout=car_004035c3+car_00431ffb+car_0043c677+pos_00405cf8) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00403800+ring_008943a0+records_008943a0+div_00403bb6+side_00601ba6+words_008943a0+state_00431ffb+widths_00852c44+ring_00405a12+links_00405a12+step_0042b17e")

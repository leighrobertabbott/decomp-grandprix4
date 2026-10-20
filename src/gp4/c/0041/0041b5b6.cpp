#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Record in esi: the dword at +0x3c is either searched on (0x004193fa) or replaced.
struct Unknown_0041b5b6_Esi {
    uint8_t Unknown00[0x3c];
    uint32_t Value3C;
};

// When byte 0x006036d7 is 0xff the original pushes [esi+0x3c], runs 0x004193fa (a regs routine that
// reads esi and ignores the stack argument) and pops it into ecx; eax/edx/ebx/ebp and the flags of
// 0x004193fa leak out. Otherwise [esi+0x3c] = [0x006034b4] << 8 (eax = the shifted value, flags of the shl).
// FUNCTION: GP4 0x0041b5b6
void __cdecl FUN_0041b5b6(gp4::Regs* r) {
    Unknown_0041b5b6_Esi* record = reinterpret_cast<Unknown_0041b5b6_Esi*>(static_cast<uintptr_t>(r->esi));

    if (GP4_GLOBAL(uint8_t, 0x006036d7) == 0xffu) {
        const uint32_t pushed = record->Value3C;
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x004193fa, c);
        r->eax = c.eax;
        r->ebx = c.ebx;
        r->ecx = pushed;
        r->edx = c.edx;
        r->ebp = c.ebp;
        r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
        return;
    }

    const uint32_t value = GP4_GLOBAL(uint32_t, 0x006034b4);
    const uint32_t result = value << 8;
    record->Value3C = result;

    // shl eax, 8 flags (pinned oracle model): CF = last bit out, AF = 0, OF = bit31(value << 7 ^ result).
    const uint32_t flags = ((value >> 24) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);

    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b5b6, FUN_0041b5b6, "regs(esi:ptr[0x300]:layout=rec_004132e5+rec_0041325b+rec_00412b88+rec_00419220) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_006380d8+state_00416dec+state_0041a2ec+state_0041416e+state_00412b88+state_00419220+state_004193fa+state_0041931d+tyre_00601510+floor_00603498+state_00413397+wheel_004193fa+sel_0041b5b6")

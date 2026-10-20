#include <gp4/gp4.h>
#include <gp4/flags.h>

// Entry of the pointer table at 0x006f306c (indexed by the dword at 0x00679bd8).
struct Unknown_006f306c {
    uint8_t Unknown00[4];
    int16_t W04;
    int16_t W06;
    int16_t W08;
};

// FUNCTION: GP4 0x00524fa4
// Selects entry [0x00679bd8] of the pointer table at 0x006f306c, remembers it in 0x00679be0 and
// scales three of its words: [0x006f3b00] = W04 << 6, [0x006f3b04] = W06 << 3,
// [0x006f3b08] = W08 << 6. Hand-written register code: ebx = the index, edi = the entry pointer,
// eax = the last scaled value, flags are those of the final `shl eax, 6`.
void __cdecl FUN_00524fa4(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    const Unknown_006f306c* entry = GP4_ARRAY(Unknown_006f306c*, 0x006f306c)[index];
    GP4_GLOBAL(const Unknown_006f306c*, 0x00679be0) = entry;

    GP4_GLOBAL(int32_t, 0x006f3b00) = (int32_t)entry->W04 << 6;
    GP4_GLOBAL(int32_t, 0x006f3b04) = (int32_t)entry->W06 << 3;
    const uint32_t v = (uint32_t)(int32_t)entry->W08;
    const uint32_t res = v << 6;
    GP4_GLOBAL(uint32_t, 0x006f3b08) = res;

    // shl eax, 6: CF = last bit shifted out; OF (pinned oracle model) = bit31(v << 5) ^ bit31(res);
    // AF = 0; SF/ZF/PF from the result.
    uint32_t arith = gp4::flags::logic32(res);
    if ((v >> 26) & 1u) arith |= gp4::flags::CF;
    if (((v << 5) ^ res) >> 31) arith |= gp4::flags::OF;

    r->eax = res;
    r->ebx = index;
    r->edi = (uint32_t)(uintptr_t)entry;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00524fa4, FUN_00524fa4, "regs() -> (eax, ebx, edi, cf, pf, af, zf, sf, of); globals=layout=ptrtab_00524fa4")

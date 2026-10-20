#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Appends one 10-entry word pair list to two word tables when the mode byte 0x0086857c is 1: the
// slot counter 0x00871c8c is incremented (it must stay below 0x80, else the routine executes int3,
// an assertion) and the 20 words at esi are de-interleaved into slot*0x14 + 2*i of the tables at
// 0x00870670 (even words) and 0x00871070 (odd words). esi advances by 0x28 in both cases. ecx is
// saved and restored; eax keeps its upper half and ends with the last word read, edx ends as the
// byte offset slot*0x14. The flags are those of `add esi, 0x28` when the mode byte is not 1, else of
// the final `cmp ecx, 10` (equal).
// FUNCTION: GP4 0x005579c9
void __cdecl FUN_005579c9(gp4::Regs* r) {
    const uint32_t esi_in = r->esi;
    if (GP4_GLOBAL(uint8_t, 0x0086857c) != 1u) {
        r->esi = esi_in + 0x28u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(esi_in, 0x28u));
        return;
    }
    const uint32_t slot = GP4_GLOBAL(uint32_t, 0x00871c8c) + 1u;
    if (slot >= 0x80u) {
        __debugbreak();
        return;
    }
    GP4_GLOBAL(uint32_t, 0x00871c8c) = slot;
    const uint32_t offset = slot * 0x14u;
    const uint16_t* words = reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(esi_in));
    uint16_t last = 0u;
    for (uint32_t i = 0u; i != 10u; ++i) {
        *reinterpret_cast<uint16_t*>(GP4_ARRAY(uint8_t, 0x00870670) + offset + i * 2u) = words[2u * i];
        last = words[2u * i + 1u];
        *reinterpret_cast<uint16_t*>(GP4_ARRAY(uint8_t, 0x00871070) + offset + i * 2u) = last;
    }
    r->eax = (r->eax & 0xffff0000u) | last;
    r->edx = offset;
    r->esi = esi_in + 0x28u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(10u, 10u));
}
GP4_IMPL(0x005579c9, FUN_005579c9, "regs(eax:u32, esi:ptr[0x30]:bytes) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cnt_0055753f")

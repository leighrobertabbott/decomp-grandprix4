#include <gp4/gp4.h>

struct Unknown_00405bde_Body {
    int32_t UnknownX;
    int32_t UnknownZ;
    uint8_t Unknown08[0x88];
    int16_t Unknown90;
    uint8_t Unknown92[0x1e];
    int16_t UnknownB0;
};

// Locates the track segment (edi) containing the point at the start of the record in esi: when
// |word +0x90| - 0x200 is below word +0xb0 by the plain walk of 0x00405d05, otherwise by 0x00405a70
// after storing the sign of word +0x90 (0 or 0xff) in byte 0x00603c78. ebx and ecx are saved.
// FUNCTION: GP4 0x00405bde
void __cdecl FUN_00405bde(gp4::Regs* r) {
    Unknown_00405bde_Body* body = reinterpret_cast<Unknown_00405bde_Body*>(static_cast<uintptr_t>(r->esi));

    int32_t magnitude = static_cast<int32_t>(body->Unknown90);
    if (magnitude < 0) magnitude = -magnitude;
    const int16_t threshold = static_cast<int16_t>(magnitude - 0x200);

    gp4::Regs c = *r;
    c.ecx = static_cast<uint32_t>(body->UnknownX);
    c.ebx = static_cast<uint32_t>(body->UnknownZ);
    if (threshold < body->UnknownB0) {
        gp4::call_regs(0x00405d05, c);
    } else {
        GP4_GLOBAL(uint8_t, 0x00603c78) = body->Unknown90 < 0 ? 0xffu : 0u;
        gp4::call_regs(0x00405a70, c);
    }

    r->eax = c.eax;
    r->edx = c.edx;
    r->edi = c.edi;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00405bde, FUN_00405bde, "regs(esi:ptr[0xb8]:layout=pos_00405b8c, edi:rec[0x8943a0,0x160,0..11]) -> (eax, edx, edi, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// esi = the record whose word +0xdc receives the result, edi = a track-segment record (ring at
// 0x8943a0). 0x00406123 steps edi along its link (CF set = no link: the routine ends with CF = 1 and
// that call's other flags). Otherwise the walked record's words +4 / +8 become the position
// 0x00623e30 / 0x00623e38 (sign-extended << 11), the original edi is restored for 0x0040460a, and
// the word [esi+0xdc] gets [0x00934df0] >> 8. The flags are those of that sar with CF cleared (clc).
// FUNCTION: GP4 0x0042ac09
void __cdecl FUN_0042ac09(gp4::Regs* r) {
    const uint32_t savedEdi = r->edi;
    gp4::call_regs(0x00406123, *r);
    if ((r->eflags & gp4::CF) != 0u) {
        r->edi = savedEdi;
        return;
    }

    uint8_t* walked = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    GP4_GLOBAL(uint32_t, 0x00623e30) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, walked, 4))) << 11;
    GP4_GLOBAL(uint32_t, 0x00623e38) = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, walked, 8))) << 11;
    r->edi = savedEdi;
    gp4::call_regs(0x0040460a, *r);

    const int32_t result = GP4_GLOBAL(int32_t, 0x00934df0) >> 8;
    r->eax = static_cast<uint32_t>(result);
    uint8_t* subject = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    GP4_FIELD(uint16_t, subject, 0xdc) = static_cast<uint16_t>(result);
    // sar eax, 8 then clc: PF/ZF/SF of the result, CF = OF = AF = 0
    const uint32_t flags = parityFlag(static_cast<uint32_t>(result))
        | (result == 0 ? 0x40u : 0u)
        | (result < 0 ? 0x80u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042ac09, FUN_0042ac09, "regs(esi:ptr[0xe0]:layout=pos_00405cf8, edi:rec[0x8943a0,0x160,0..25]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_00405a12")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00424e1b
// pushal ... popal routine. esi = object record: +0x64 is its segment as a byte offset into the
// 0x160-stride ring at 0x008943a0. The segment walker 0x00405c30 (edi = ring record) moves the
// position to the segment that really contains the object; the new offset is written back to
// +0x64, and +0x58 = (signed word at record+6) << 8. All registers are restored; the flags left
// are those of the final "shl eax, 8" (CF = bit 24 of the sign-extended word).
void __cdecl FUN_00424e1b(gp4::Regs* r) {
    uint8_t* object = reinterpret_cast<uint8_t*>(r->esi);

    gp4::Regs walk = *r;
    walk.edi = GP4_FIELD(uint32_t, object, 0x64) + 0x008943a0u;
    gp4::call_regs(0x00405c30, walk);

    const uint32_t record = walk.edi;
    GP4_FIELD(uint32_t, object, 0x64) = record - 0x008943a0u;

    const int32_t word = GP4_FIELD(int16_t, record, 6);
    const uint32_t value = static_cast<uint32_t>(word);
    const uint32_t result = value << 8;
    GP4_FIELD(uint32_t, object, 0x58) = result;

    // shl by 8: CF = last bit shifted out, AF = 0, OF = bit31((value << 7) ^ result).
    const uint32_t flags = ((value >> 24) & 1u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80000000u ? 0x80u : 0u)
        | ((((value << 7) ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424e1b, FUN_00424e1b, "regs(esi:ptr[0x6c]:layout=obj_00424e1b) -> (cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+seg_00424e1b")

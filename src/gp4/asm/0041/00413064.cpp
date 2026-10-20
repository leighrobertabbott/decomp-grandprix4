#include <gp4/gp4.h>

// Tests a flag bit of the byte +0x92 of the block at edi chosen by the distance between the words
// [esi+0xe] and [edi+0x0]: |difference| (16-bit) doubled has its sign set -> bit 0x02, else bit
// 0x01. CF = 1 when the chosen bit is set (then ZF = PF = 0), CF = 0 when clear (ZF = PF = 1);
// the test leaves SF = OF = AF = 0. eax is saved and restored.
// FUNCTION: GP4 0x00413064
void __cdecl FUN_00413064(gp4::Regs* r) {
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    uint16_t distance = static_cast<uint16_t>(*reinterpret_cast<const uint16_t*>(other + 0xe)
        - *reinterpret_cast<const uint16_t*>(body));
    if ((distance & 0x8000u) != 0u) {
        distance = static_cast<uint16_t>(0u - distance);
    }
    const uint32_t mask = (static_cast<uint16_t>(distance << 1) & 0x8000u) != 0u ? 2u : 1u;
    const bool set = (body[0x92] & mask) != 0u;

    r->eflags = (r->eflags & ~0x8d5u) | (set ? 0x01u : 0x44u);
}
GP4_IMPL(0x00413064, FUN_00413064, "regs(esi:ptr[0x10]:bytes, edi:ptr[0x9a]:bytes) -> (cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide OR: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040eb25
// pushal ... popal wrapper: every register is restored, only the flags of the
// last flag writer remain. The record found by 0x0040ea46 (ebx) gets flag bits
// 0x04 and 0x01 set at +0x82.
void __cdecl FUN_0040eb25(gp4::Regs* r) {
    gp4::Regs saved = *r;
    gp4::call_regs(0x0040ea46, *r);

    // test ebx, ebx with ebx == 0 leaves ZF|PF
    uint32_t flags = 0x44u;
    if (r->ebx != 0u) {
        void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->ebx));
        GP4_FIELD(uint8_t, record, 0x82) |= 4;
        GP4_FIELD(uint8_t, record, 0x82) |= 1;
        flags = logicFlags8(GP4_FIELD(uint8_t, record, 0x82));
    }

    saved.eflags = (saved.eflags & ~0x8d5u) | flags;
    *r = saved;
}
GP4_IMPL(0x0040eb25, FUN_0040eb25, "regs(esi:ptr[0x41d]:layout=car_0040ea09) -> (cf, pf, af, zf, sf, of); globals=layout=carno_0066ff6c")

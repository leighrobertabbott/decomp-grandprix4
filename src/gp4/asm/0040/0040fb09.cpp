#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a logic result of the given width: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityFlag8(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & signBit) != 0u ? 0x80u : 0u);
}

// FUNCTION: GP4 0x0040fb09
// Car record at esi: 0x0043bb15 selects the car's data pointer 0x00601f24 for index (byte +0x7c - 1) & 0x3f
// (it keeps every register); when the sign bit of byte +0x7c is clear and byte 0x00601f20 is non-zero, the
// pointer is replaced by 0x007ad9f4 (also left in eax). The flags are those of the last test / cmp.
void __cdecl FUN_0040fb09(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    r->eax = (static_cast<uint32_t>(GP4_FIELD(uint8_t, car, 0x7c)) - 1u) & 0x3fu;
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(r->eax, 0x80000000u);   // dec / and eax, 0x3f
    gp4::call_regs(0x0043bb15, *r);

    const uint32_t key = GP4_FIELD(uint8_t, car, 0x7c);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(key, 0x80u);           // test byte [esi+0x7c], 0xff
    if ((key & 0x80u) != 0u) {
        return;
    }

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x00601f20);
    r->eflags = (r->eflags & ~0x8d5u) | logicFlags(enabled, 0x80u);       // cmp byte [0x601f20], 0
    if (enabled == 0u) {
        return;
    }

    r->eax = 0x007ad9f4u;
    GP4_GLOBAL(uint32_t, 0x00601f24) = r->eax;
}
GP4_IMPL(0x0040fb09, FUN_0040fb09, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=trk_007ad894+circ_007adb50+codes_0043ba12")

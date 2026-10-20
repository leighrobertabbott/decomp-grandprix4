#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 16-bit `sub left, right`.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    left &= 0xffffu;
    right &= 0xffffu;
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Starting at the segment record [0x00891504] + (cx * 8) * 0x160 the bit 4 of byte +0x1b is set (dh == 0)
// or cleared (dh != 0) in 32 consecutive records, stepping with 0x00406045. Afterwards word +0x1a of the
// record [0x00866f04] is copied into the record [0x00866ef8] (esi / edi are left at those records). eax ends
// as {high half of the start offset, 0xffff}; edx is saved and restored; the flags are those of the final
// `sub ax, 1`.
// FUNCTION: GP4 0x00441d85
void __cdecl FUN_00441d85(gp4::Regs* r) {
    const uint32_t offset = static_cast<uint32_t>(static_cast<uint16_t>(r->ecx << 3)) * 0x160u;
    const bool clearBit = ((r->edx >> 8) & 0xffu) != 0u;
    uint32_t edi = GP4_GLOBAL(uint32_t, 0x00891504) + offset;
    uint32_t counter = 0x1fu;
    uint32_t flags;

    for (;;) {
        uint8_t* flagByte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi + 0x1bu));
        if (clearBit) {
            *flagByte = static_cast<uint8_t>(*flagByte & 0xefu);
        } else {
            *flagByte = static_cast<uint8_t>(*flagByte | 0x10u);
        }
        gp4::Regs call = *r;
        call.edi = edi;
        gp4::call_regs(0x00406045, call);
        edi = call.edi;
        flags = subFlags16(counter, 1u);
        counter = (counter - 1u) & 0xffffu;
        if ((counter & 0x8000u) != 0u) {
            break;
        }
    }

    const uint32_t source = GP4_GLOBAL(uint32_t, 0x00866f04);
    const uint32_t target = GP4_GLOBAL(uint32_t, 0x00866ef8);
    *reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(target + 0x1au)) =
        *reinterpret_cast<const uint16_t*>(static_cast<uintptr_t>(source + 0x1au));

    r->eax = (offset & 0xffff0000u) | counter;
    r->edi = source;
    r->esi = target;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441d85, FUN_00441d85, "regs(ecx:u32[0..1], edx:u32[0..511]) -> (eax, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+records_008943a0+w1a_008943a0")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 32-bit `add left, right`.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Walks the 22 car records at 0x0066fef0 (stride 0x41c). Bit 1 of byte +0x21c is cleared in every record.
// A record whose byte +0xe5 bit 2 and byte +0x21c bit 5 are clear is asked 0x0044360a (carry = it qualifies,
// eax gets its word +0xee). On a qualifying record: when the word [0x007ad89e] is >= 2 or the byte [0x007ad8af] is
// nonzero, bit 5 of +0xfe is set, bit 0 of +0x162 cleared and 0x0044351d is called; otherwise 0x00443629 decides,
// and without its carry bit 2 of +0x21c is set. ecx is saved and restored; esi ends one past the array;
// eax / edx hold the results of the last callee; the flags are those of the final `add esi, 0x41c`.
// FUNCTION: GP4 0x00443648
void __cdecl FUN_00443648(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t esi = 0x0066fef0u;
    uint32_t before = esi;
    for (int remaining = 0x16; remaining != 0; --remaining) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
        record[0x21c] = static_cast<uint8_t>(record[0x21c] & 0xfdu);
        if ((record[0xe5] & 4u) == 0u && (record[0x21c] & 0x20u) == 0u) {
            gp4::Regs call = *r;
            call.eax = eax;
            call.edx = edx;
            call.ecx = static_cast<uint32_t>(remaining);
            call.esi = esi;
            gp4::call_regs(0x0044360a, call);
            eax = call.eax;
            if ((call.eflags & 1u) != 0u) {
                if (GP4_GLOBAL(int16_t, 0x007ad89e) >= 2 || GP4_GLOBAL(uint8_t, 0x007ad8af) != 0u) {
                    record[0xfe] = static_cast<uint8_t>(record[0xfe] | 0x20u);
                    record[0x162] = static_cast<uint8_t>(record[0x162] & 0xfeu);
                    call.eax = eax;
                    call.edx = edx;
                    call.ecx = static_cast<uint32_t>(remaining);
                    call.esi = esi;
                    gp4::call_regs(0x0044351d, call);
                    eax = call.eax;
                    edx = call.edx;
                } else {
                    call.eax = eax;
                    call.edx = edx;
                    call.ecx = static_cast<uint32_t>(remaining);
                    call.esi = esi;
                    gp4::call_regs(0x00443629, call);
                    if ((call.eflags & 1u) == 0u) {
                        record[0x21c] = static_cast<uint8_t>(record[0x21c] | 4u);
                    }
                }
            }
        }
        before = esi;
        esi += 0x41cu;
    }
    r->eax = eax;
    r->edx = edx;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 0x41cu);
}
GP4_IMPL(0x00443648, FUN_00443648, "regs(eax:u32, edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_0042595c+cars_0066fef0+carst_0040f6d0+flags_00408f7b+state_00413f28+cars_004436f9+cars_00443648")

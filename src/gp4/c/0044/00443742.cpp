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

// Walks the 22 car records at 0x0066fef0 (stride 0x41c) and stores a due time into dword +0xf0: the clock
// 0x0066d814 plus a delay. A record with bit 2 of byte +0xe5 set draws a random byte (generator
// 0x00401462), scales it by the table entry 0x00602aa4[word 0x007ad89e] in Q14 ((byte << 6) * entry >> 14);
// otherwise the delay is 0, or the constant 0x006039ec when bit 2 of byte +0x21c was set (the bit is then
// cleared). ecx is saved and restored; esi ends one past the array; eax / edx keep the last delay and
// product high half; the flags are those of the final `add esi, 0x41c`.
// FUNCTION: GP4 0x00443742
void __cdecl FUN_00443742(gp4::Regs* r) {
    uint32_t eax = 0u;
    uint32_t edx = r->edx;
    uint32_t esi = 0x0066fef0u;
    uint32_t before = esi;

    for (int remaining = 0x16; remaining != 0; --remaining) {
        uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi));
        if ((record[0xe5] & 4u) != 0u) {
            gp4::Regs call = *r;
            call.esi = esi;
            call.edx = edx;
            gp4::call_regs(0x00401462, call);
            const int32_t scaled = static_cast<int32_t>((call.eax & 0xffu) << 6);
            const uint32_t index = GP4_GLOBAL(uint16_t, 0x007ad89e);
            const int32_t entry = GP4_ARRAY(int32_t, 0x00602aa4)[index];
            const int64_t product = static_cast<int64_t>(scaled) * entry;
            const uint32_t low = static_cast<uint32_t>(product);
            edx = static_cast<uint32_t>(product >> 32);
            eax = (low >> 14) | (edx << 18);
        } else {
            eax = 0u;
            if ((record[0x21c] & 4u) != 0u) {
                record[0x21c] = static_cast<uint8_t>(record[0x21c] & 0xfbu);
                eax = GP4_GLOBAL(uint32_t, 0x006039ec);
            }
        }
        *reinterpret_cast<uint32_t*>(record + 0xf0) = GP4_GLOBAL(uint32_t, 0x0066d814) + eax;
        before = esi;
        esi += 0x41cu;
    }

    r->eax = eax;
    r->edx = edx;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(before, 0x41cu);
}
GP4_IMPL(0x00443742, FUN_00443742, "regs(edx:u32) -> (eax, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0066d8c0+state_0042595c+clock_0066d814+cars_0066fef0+carst_0040f6d0+flags_00408f7b")

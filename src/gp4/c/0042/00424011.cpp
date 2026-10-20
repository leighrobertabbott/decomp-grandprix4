#include <gp4/gp4.h>
#include <gp4/x87.h>

static inline uint32_t ParityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (~parity & 1u) << 2;
}

static inline uint32_t AddFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return static_cast<uint32_t>((static_cast<uint64_t>(lhs) + rhs) >> 32)
        | ParityFlag(result) | ((lhs ^ rhs ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 31) << 7)
        | ((~(lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

static inline uint32_t SubtractFlags(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (lhs < rhs ? 1u : 0u) | ParityFlag(result)
        | ((lhs ^ rhs ^ result) & 0x10u) | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80000000u) >> 20);
}

// FUNCTION: GP4 0x00424011
void __cdecl FUN_00424011(gp4::Regs* r) {
    using namespace gp4::x87;
    uint32_t edx = r->edx;

    GP4_GLOBAL(uint32_t, 0x00608184) = 0;
    GP4_GLOBAL(uint32_t, 0x00608274) = 0;
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x006097ac)));
    uint8_t* slot = GP4_ARRAY(uint8_t, 0x006211a0);
    uint8_t* coefficients = GP4_ARRAY(uint8_t, 0x00622e98);
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x006097cc);
    uint32_t i = 0;

    do {
        bool skip = false;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006082b4);
        if (i < limit) {
            if (GP4_FIELD(uint32_t, record, 0xc) != 0u) skip = true;
        } else if (i == limit) {
            record = GP4_ARRAY(uint8_t, 0x00620fc0);
        }
        if (!skip) {
            edx = 0x00622e98u;
            Ext acc = (ext_i32(GP4_FIELD(int32_t, record, 0)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b4)))
                * ext(GP4_FIELD(double, coefficients, 0x60));
            const Ext y = (ext_i32(GP4_FIELD(int32_t, record, 4)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097b8)))
                * ext(GP4_FIELD(double, coefficients, 0x78));
            acc = acc + y;
            const Ext z = (ext_i32(GP4_FIELD(int32_t, record, 8)) - ext_i32(GP4_GLOBAL(int32_t, 0x006097bc)))
                * ext(GP4_FIELD(double, coefficients, 0x90));
            acc = acc + z;
            const int32_t rounded = fistp32(acc);
            GP4_GLOBAL(int32_t, 0x006f2bf4) = rounded;
            GP4_FIELD(int32_t, slot, 4) = rounded;
            const uint32_t balance = static_cast<uint32_t>(rounded)
                + GP4_GLOBAL(uint32_t, 0x00607f2c) - GP4_GLOBAL(uint32_t, 0x00607fbc);
            if (static_cast<int32_t>(balance) < 0) {
                const uint32_t deficit = 0u - balance;
                const int64_t product = static_cast<int64_t>(static_cast<int32_t>(deficit))
                    * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608010));
                edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
                GP4_GLOBAL(uint32_t, 0x00608184) += static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
                GP4_GLOBAL(uint32_t, 0x00608274) += 1u;
            }
        }
        slot += 0x20;
        record += 0x14;
        ++i;
    } while (i != count);

    // The loop exit compare was equal: ZF | PF. "or eax, eax" with eax == 0 gives the same.
    uint32_t eax = GP4_GLOBAL(uint32_t, 0x00608274);
    uint32_t flags = 0x44u;
    if (eax != 0u) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(eax))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00608010));
        const uint32_t divisor = static_cast<uint32_t>(product);
        r->ebp = divisor;
        const uint32_t shifted = GP4_GLOBAL(uint32_t, 0x00639630) << 8;
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x00608184);
        const uint32_t diff = before - shifted;
        eax = diff;
        edx = shifted;
        flags = SubtractFlags(before, shifted, diff);
        if (static_cast<int32_t>(diff) >= 0) {
            int32_t rem;
            const int32_t quotient = gp4::idiv64(static_cast<int64_t>(static_cast<uint64_t>(diff) << 14),
                                                static_cast<int32_t>(divisor), &rem);
            const uint32_t old = GP4_GLOBAL(uint32_t, 0x00607f2c);
            const uint32_t sum = old + static_cast<uint32_t>(quotient);
            GP4_GLOBAL(uint32_t, 0x00607f2c) = sum;
            flags = AddFlags(old, static_cast<uint32_t>(quotient), sum);
            eax = static_cast<uint32_t>(quotient);
            edx = static_cast<uint32_t>(rem);
        }
    } else {
        eax = 0u;
    }

    r->eax = eax;
    r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
    r->ecx = count;
    r->edx = edx;
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slot));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00424011, FUN_00424011, "regs(edx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, edi, cf, pf, af, zf, sf, of); globals=layout=state_00424011")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `shrd low, high, n` (1 < n < 32), the pinned oracle model: CF = last bit shifted out,
// AF = 0, OF = bit31(previous ^ result) with previous the 64-bit value shifted by n - 1.
static inline uint32_t shrdFlags(uint32_t low, uint32_t high, uint32_t n) {
    const uint64_t wide = (static_cast<uint64_t>(high) << 32) | low;
    const uint32_t result = static_cast<uint32_t>(wide >> n);
    const uint32_t previous = static_cast<uint32_t>(wide >> (n - 1u));
    return ((previous & 1u) != 0u ? 1u : 0u)
        | parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0041b852
// Per-car drive-force step for the body in esi. Clears [0x00601ead]. When byte +0x7b bit 1 is clear
// [0x0062474c] = 0 and nothing else happens (flags of "test": ZF, PF). Otherwise
//   [0x00624748] = 0x41b600(esi); [0x00623b78] = (([0x00624748] * [0x00601e84]) >> 6) / max(0x400, [0x00623c34] >> 8)
// (64-bit idiv), then 0x41b7e2 fills [0x00623c1c]. A term t = min((([0x006247a4] * [0x0062479c]) >> 14), [0x006247a0])
// (only when byte [0x00622424] is zero, else 0) gives [0x00624750] = -t; sum = -t + [0x00623b78] + [0x00623c1c]
// sets [0x00601ead] = 0xff when it is negative. The word +0xa0 is scaled by [0x0062d278] (Q14) when byte +0x81 bit 4
// is set and signed word +0x9a >= 0x755; [0x0062474c] = (((sum * word) >> 14) * [0x00601ffc]) >> 14, times
// [0x0062eca0] (>> 14) again when byte +0x8d bit 0 is set. eax = [0x0062474c], edx = high half of the
// last product, ecx/ebp from 0x41b7e2; the flags are those of the last shrd, or of "test +0x8d,1" (ZF, PF).
void __cdecl FUN_0041b852(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    GP4_GLOBAL(uint8_t, 0x00601ead) = 0;
    if ((car[0x7b] & 2u) == 0u) {
        GP4_GLOBAL(uint32_t, 0x0062474c) = 0;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    // test byte [0x00622424], 0xff; jne X; jmp X: both reach the call.
    gp4::Regs w = *r;
    w.st_in = 0;
    w.st_out = 0;
    w.st_xin_mask = 0;
    w.st_xout_mask = 0;
    gp4::call_regs(0x0041b600, w);

    const uint32_t drive = w.eax;
    GP4_GLOBAL(uint32_t, 0x00624748) = drive;
    const int64_t scaled = static_cast<int64_t>(static_cast<int32_t>(drive))
        * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601e84));
    const int64_t dividend = scaled >> 6;
    int32_t divisor = GP4_GLOBAL(int32_t, 0x00623c34) >> 8;
    if (divisor < 0x400) {
        divisor = 0x400;
    }
    int32_t remainder;
    const int32_t quotient = gp4::idiv64(dividend, divisor, &remainder);
    GP4_GLOBAL(int32_t, 0x00623b78) = quotient;

    w.eax = static_cast<uint32_t>(quotient);
    w.edx = static_cast<uint32_t>(remainder);
    w.ebp = static_cast<uint32_t>(divisor);
    gp4::call_regs(0x0041b7e2, w);

    int32_t term = 0;
    if (GP4_GLOBAL(uint8_t, 0x00622424) == 0u) {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x006247a4))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062479c));
        term = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
        if (term > GP4_GLOBAL(int32_t, 0x006247a0)) {
            term = GP4_GLOBAL(int32_t, 0x006247a0);
        }
    }
    uint32_t eax = 0u - static_cast<uint32_t>(term);
    GP4_GLOBAL(uint32_t, 0x00624750) = eax;
    eax += GP4_GLOBAL(uint32_t, 0x00623b78);
    eax += GP4_GLOBAL(uint32_t, 0x00623c1c);
    if ((eax & 0x80000000u) != 0u) {
        GP4_GLOBAL(uint8_t, 0x00601ead) = 0xff;
    }

    uint32_t factor = *reinterpret_cast<const uint16_t*>(car + 0xa0);
    if ((car[0x81] & 0x10u) != 0u && *reinterpret_cast<const int16_t*>(car + 0x9a) >= 0x755) {
        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(factor))
            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062d278));
        factor = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    }

    int64_t product = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(static_cast<int32_t>(factor));
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    product = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00601ffc));
    eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);

    uint32_t flags = 0x44u;
    if ((car[0x8d] & 1u) != 0u) {
        product = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0062eca0));
        flags = shrdFlags(static_cast<uint32_t>(product), static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32), 14u);
        eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
    }
    GP4_GLOBAL(uint32_t, 0x0062474c) = eax;

    r->eax = eax;
    r->ebx = w.ebx;
    r->ecx = w.ecx;
    r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    r->edi = w.edi;
    r->ebp = w.ebp;
    r->eflags = (w.eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0041b852, FUN_0041b852, "regs(esi:ptr[0x228]:bytes) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0041b950+state_00411109+state_0041b600+frac_00624790+state_0041b7e2")

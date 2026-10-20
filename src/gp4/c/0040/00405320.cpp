#include <gp4/gp4.h>

// 64-bit product viewed as edx:eax; shrd eax, edx, 14 keeps the low dword of product >> 14.
static inline uint32_t shiftedProduct(int32_t left, int32_t right, uint32_t* high) {
    const int64_t product = static_cast<int64_t>(left) * static_cast<int64_t>(right);
    *high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    return static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14);
}

// FUNCTION: GP4 0x00405320
// Fills DAT_00603104/08/0c/10 with 0x00411c75(DAT_0060309c/a0/a4/a8, 0..3), then derives
// DAT_00602d78 and DAT_00602d74 from the pair sums scaled by record fields (+0xa2, +0x10e) and,
// unless record flag +0xe5 bit 0 is set, by (1 - [+0xba] * DAT_0062d29c) in 18.14 fixed point.
// edx is left with the high dword of the final product, flags with the final shrd.
void __cdecl FUN_00405320(gp4::Regs* r) {
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    call.eax = GP4_GLOBAL(uint32_t, 0x0060309c);
    call.ecx = 0;
    gp4::call_regs(0x00411c75, call);
    GP4_GLOBAL(uint32_t, 0x00603104) = call.eax;

    call.eax = GP4_GLOBAL(uint32_t, 0x006030a0);
    call.ecx = 1;
    gp4::call_regs(0x00411c75, call);
    GP4_GLOBAL(uint32_t, 0x00603108) = call.eax;

    call.eax = GP4_GLOBAL(uint32_t, 0x006030a4);
    call.ecx = 2;
    gp4::call_regs(0x00411c75, call);
    GP4_GLOBAL(uint32_t, 0x0060310c) = call.eax;

    call.eax = GP4_GLOBAL(uint32_t, 0x006030a8);
    call.ecx = 3;
    gp4::call_regs(0x00411c75, call);
    GP4_GLOBAL(uint32_t, 0x00603110) = call.eax;

    const int32_t width = GP4_FIELD(const uint16_t, record, 0xa2);
    const int32_t gain = GP4_FIELD(const uint16_t, record, 0x10e);
    uint32_t high = 0;

    uint32_t eax = shiftedProduct(
        static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00603104) + GP4_GLOBAL(uint32_t, 0x00603108)),
        width, &high);
    uint32_t ebp = 0x4000u;
    if ((GP4_FIELD(const uint8_t, record, 0xe5) & 1u) == 0u) {
        const uint32_t drag = shiftedProduct(
            static_cast<int32_t>(GP4_FIELD(const uint16_t, record, 0xba)),
            GP4_GLOBAL(int32_t, 0x0062d29c), &high);
        ebp = 0x4000u - drag;
    }
    eax = shiftedProduct(static_cast<int32_t>(eax), static_cast<int32_t>(ebp), &high);
    eax = shiftedProduct(static_cast<int32_t>(eax), gain, &high);
    GP4_GLOBAL(uint32_t, 0x00602d78) = eax;

    eax = shiftedProduct(
        static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0060310c) + GP4_GLOBAL(uint32_t, 0x00603110)),
        width, &high);
    eax = shiftedProduct(static_cast<int32_t>(eax), static_cast<int32_t>(ebp), &high);

    // Last product: imul edx, then shrd eax, edx, 14 whose flags the routine returns.
    const int64_t last = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(gain);
    const uint64_t bits = static_cast<uint64_t>(last);
    const uint32_t low = static_cast<uint32_t>(bits);
    const uint32_t result = static_cast<uint32_t>(bits >> 14);
    const uint32_t previous = static_cast<uint32_t>(bits >> 13);
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    // SHRD by 14: CF = last bit shifted out, AF = 0, OF = bit31(previous ^ result).
    const uint32_t flags = ((low >> 13) & 1u) | ((~parity & 1u) << 2)
        | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((previous ^ result) >> 31) << 11);
    GP4_GLOBAL(uint32_t, 0x00602d74) = result;

    r->eax = result;
    r->ecx = call.ecx;
    r->edx = static_cast<uint32_t>(bits >> 32);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00405320, FUN_00405320, "regs(esi:ptr[0x118]:bytes) -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=fuzz:i32")

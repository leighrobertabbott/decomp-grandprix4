#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// pushal wrapper for the car record in esi. When the pointer [0x00602d84] (a segment record) is non-null it runs
// 0x0041f1c1 with edi = that pointer and 0x0044182d, keeps the larger of the ecx it leaves and the dword [0x00602fb0]
// (signed) and stores it as a word at +0xa2 of the car record. popal restores every register; the flags are those of
// `or edi, edi` when the pointer is null and of the final `cmp ecx, [0x00602fb0]` otherwise.
// FUNCTION: GP4 0x00425c87
void __cdecl FUN_00425c87(gp4::Regs* r) {
    const uint32_t segment = GP4_GLOBAL(uint32_t, 0x00602d84);
    if (segment == 0u) {
        const uint32_t flags = parityFlag(0u) | 0x40u;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        return;
    }

    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;
    call.edi = segment;
    gp4::call_regs(0x0041f1c1, call);
    gp4::call_regs(0x0044182d, call);

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00602fb0);
    const uint32_t flags = cmpFlags32(call.ecx, limit);
    uint32_t value = call.ecx;
    if (static_cast<int32_t>(call.ecx) < static_cast<int32_t>(limit)) {
        value = limit;
    }
    GP4_FIELD(uint16_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.esi)), 0xa2) =
        static_cast<uint16_t>(value);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00425c87, FUN_00425c87, "regs(esi:ptr[0x41c]:layout=rec_0043259c) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=surf_0041f02b+state_0043c1ba+state_004320f2+counter_0040f0dd+scale_0066ec60+seg_00602d84")

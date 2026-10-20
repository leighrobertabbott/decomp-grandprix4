#include <gp4/gp4.h>

enum : uint32_t {
    kArithFlags = 0x8d5u,   // OF SF ZF AF PF CF
    kPF = 0x4u, kAF = 0x10u, kZF = 0x40u, kSF = 0x80u, kOF = 0x800u
};

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? kPF : 0u;
}

// Flags of a 32-bit add a, b.
static inline uint32_t addFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a + b;
    return (result < a ? 1u : 0u) | ((a ^ b ^ result) & kAF) | parityFlag8(result) |
           (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u) |
           ((((~(a ^ b)) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x0040afb2
// pushal ... popal routine over the 0x160-byte segment ring at [0x00891504]. Marks the lap-start
// records with 1/2 in byte +0x12 together with the free group found by 0x0040af67 from record
// (lap length >> 1), remembers that group (0x00866f10, offset 0x00866ed8, key 0x0086856c), then
// marks byte +0x64 of the two groups found from lap length / 3 (bit 1, pointer 0x006279ec) and
// twice that (bit 2, pointer 0x006279f0). Only the flags of the last "add edi, 0x160" escape.
void __cdecl FUN_0040afb2(gp4::Regs* r) {
    gp4::Regs call = *r;
    call.st_in = 0;
    call.st_out = 0;

    uint8_t* lap = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00891504)));
    GP4_GLOBAL(uint32_t, 0x006279f4) = reinterpret_cast<uintptr_t>(lap);

    call.eax = static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x00868558)) >> 1;
    gp4::call_regs(0x0040af67, call);

    uint8_t* group = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.edi));
    GP4_GLOBAL(uint32_t, 0x00866f10) = call.edi;
    GP4_GLOBAL(uint32_t, 0x00866ed8) = call.edi - 0x008943a0u;
    GP4_GLOBAL(uint16_t, 0x0086856c) = static_cast<uint16_t>(GP4_FIELD(uint16_t, group, 0x1a) & 0x7ffu);

    for (int i = 0; i < 8; ++i) {
        lap[0x12] = static_cast<uint8_t>(lap[0x12] | 0x01u);
        group[0x12] = static_cast<uint8_t>(group[0x12] | 0x01u);
        lap[0x12] = static_cast<uint8_t>(lap[0x12] | 0x02u);
        lap += 0x160;
        group += 0x160;
    }

    const uint32_t length = GP4_GLOBAL(uint16_t, 0x00868558);
    const uint32_t third = length / 3u;
    call.eax = third;
    gp4::call_regs(0x0040af67, call);

    group = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.edi));
    GP4_GLOBAL(uint32_t, 0x006279ec) = call.edi;
    for (int i = 0; i < 8; ++i) {
        group[0x64] = static_cast<uint8_t>(group[0x64] | 0x01u);
        group += 0x160;
    }

    call.eax = third << 1;
    gp4::call_regs(0x0040af67, call);

    group = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.edi));
    GP4_GLOBAL(uint32_t, 0x006279f0) = call.edi;
    for (int i = 0; i < 8; ++i) {
        group[0x64] = static_cast<uint8_t>(group[0x64] | 0x02u);
        group += 0x160;
    }

    r->eflags = (r->eflags & ~kArithFlags) | addFlags(call.edi + 7u * 0x160u, 0x160u);
}
GP4_IMPL(0x0040afb2, FUN_0040afb2, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040af67")

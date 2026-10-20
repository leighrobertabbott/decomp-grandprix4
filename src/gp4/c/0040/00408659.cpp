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

// Flags left by a logic operation (TEST) on an 8-bit result: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result) | ((result & 0xffu) == 0u ? kZF : 0u) | ((result & 0x80u) != 0u ? kSF : 0u);
}

// Flags of a 32-bit cmp a, b.
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u) | ((a ^ b ^ result) & kAF) | parityFlag8(result) |
           (result == 0u ? kZF : 0u) | ((result >> 31) != 0u ? kSF : 0u) |
           ((((a ^ b) & (a ^ result)) >> 31) != 0u ? kOF : 0u);
}

// FUNCTION: GP4 0x00408659
// Car record in esi (22 records of 0x41c bytes from 0x0066fef0). When the mode byte 0x007c2ea9 has
// bit 7 clear it only stores the word 0x28 at +0xea. Otherwise it sets flag 0x40 of byte +0xfe,
// clears word +0xea, sets 0x10 of +0xfe when word +0x90 is negative, refreshes the segment state
// (0x004036ca, registers preserved by its pushal/popal) and stores into dword +0x114 the pointer
// to the neighbouring record (next, or previous when bit 1 of byte +0x7d is set), not below the
// first record. eax is left holding that pointer; the flags are those of the last cmp.
void __cdecl FUN_00408659(gp4::Regs* r) {
    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t flags = logicFlags8(mode);
    if ((mode & 0x80u) == 0u) {
        GP4_FIELD(uint16_t, rec, 0xea) = 0x28;
    } else {
        rec[0xfe] = static_cast<uint8_t>(rec[0xfe] | 0x40u);
        GP4_FIELD(uint16_t, rec, 0xea) = 0;
        if (GP4_FIELD(int16_t, rec, 0x90) < 0) {
            rec[0xfe] = static_cast<uint8_t>(rec[0xfe] | 0x10u);
        }

        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x004036ca, call);

        uint32_t neighbour = r->esi;
        if ((rec[0x7d] & 0x02u) == 0u) {
            neighbour += 0x41cu;
        } else {
            neighbour -= 0x41cu;
        }
        flags = cmpFlags(neighbour, 0x0066fef0u);
        if (neighbour < 0x0066fef0u) {
            neighbour = 0x0066fef0u;
        }
        GP4_FIELD(uint32_t, rec, 0x114) = neighbour;
        r->eax = neighbour;
    }
    r->eflags = (r->eflags & ~kArithFlags) | flags;
}
GP4_IMPL(0x00408659, FUN_00408659, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+links_00405a12+state_00403800+div_00403bb6")

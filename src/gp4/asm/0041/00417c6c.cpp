#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by OR/TEST on a byte: CF = OF = AF = 0, PF/ZF/SF from the byte result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// One of the four 0x300-byte slots of the record array at 0x006380d8: a status byte at +0x2e and a
// dword at +0x1d4.
struct Unknown_00417c6c_Slot {
    uint8_t Unknown00[0x2e];
    uint8_t Flags2E;
    uint8_t Unknown2F[0x1d4 - 0x2f];
    uint32_t Unknown1D4;
    uint8_t Unknown1D8[0x300 - 0x1d8];
};

// The caller's record (only the bytes/words this routine reads).
struct Unknown_00417c6c_Source {
    uint8_t Unknown00[0x75];
    uint8_t Slot75;
    uint8_t Unknown76[0x84 - 0x76];
    uint8_t Kind84;
    uint8_t Unknown85[0x87 - 0x85];
    uint8_t Flags87;
    uint8_t Unknown88[0x8d - 0x88];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x9a - 0x8e];
    int16_t Unknown9A;
};

// FUNCTION: GP4 0x00417c6c
// Only runs when bit 4 of byte +0x8d is set. Then ebx = 0x006380d8 (four 0x300-byte slots, picked by
// byte +0x75 & 3) and the low byte of ecx is byte +0x84. If that byte is 2 the stamp 0x601e18 is
// stored into the slot's dword +0x1d4. Next, if byte +0x87 has bit 4 the slot's status byte +0x2e
// gets bit 5 and 0x601f1d becomes 0xff; otherwise, if it has bit 3, the dword +0x1d4 is replaced by
// the table cosine (Q14) of 0x4000 - (((0x66d814 * 0x876efc >> 14) * word[+0x9a]) >> 14), scaled by
// 0x601e1c >> 14. The registers and flags are those of the last instruction executed.
void __cdecl FUN_00417c6c(gp4::Regs* r) {
    const Unknown_00417c6c_Source* src =
        reinterpret_cast<const Unknown_00417c6c_Source*>(static_cast<uintptr_t>(r->esi));
    Unknown_00417c6c_Slot* slots = reinterpret_cast<Unknown_00417c6c_Slot*>(GP4_ARRAY(uint8_t, 0x006380d8));

    uint32_t flags = logicFlags8(0u);   // test byte, imm with a zero result (the skipped paths)

    if ((src->Flags8D & 0x10u) != 0u) {
        r->ebx = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(slots));
        const uint32_t kind = src->Kind84;
        r->ecx = (r->ecx & 0xffffff00u) | kind;

        if (kind == 2u) {
            const uint32_t slot = src->Slot75 & 3u;
            const uint32_t stamp = GP4_GLOBAL(uint32_t, 0x00601e18);
            slots[slot].Unknown1D4 = stamp;
            r->eax = slot * 0x300u;
            r->edx = stamp;
        }

        if ((src->Flags87 & 0x10u) != 0u) {
            GP4_GLOBAL(uint8_t, 0x00601f1d) = 0xffu;
            const uint32_t slot = src->Slot75 & 3u;
            slots[slot].Flags2E = static_cast<uint8_t>(slots[slot].Flags2E | 0x20u);
            r->eax = slot * 0x300u;
            r->edx = 0u;
            flags = logicFlags8(slots[slot].Flags2E);
        } else if ((src->Flags87 & 8u) != 0u) {
            const uint32_t slot = src->Slot75 & 3u;
            r->ecx = slot * 0x300u;

            const int64_t p1 = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0066d814)) *
                               GP4_GLOBAL(int32_t, 0x00876efc);
            const uint32_t q1 = static_cast<uint32_t>(static_cast<uint64_t>(p1) >> 14);
            const int64_t p2 = static_cast<int64_t>(static_cast<int32_t>(q1)) * src->Unknown9A;
            const uint32_t q2 = static_cast<uint32_t>(static_cast<uint64_t>(p2) >> 14);

            uint16_t angle = static_cast<uint16_t>(0x4000u - static_cast<uint16_t>(q2));
            if (static_cast<int16_t>(angle) < 0) angle = static_cast<uint16_t>(0u - angle);
            angle = static_cast<uint16_t>(angle >> 2);
            const uint32_t byteOffset = static_cast<uint32_t>(angle) & 0xfffeu;
            const int32_t cosine = *reinterpret_cast<const int16_t*>(GP4_ARRAY(const uint8_t, 0x007c5c5c) + byteOffset);

            const int64_t p3 = static_cast<int64_t>(cosine) * GP4_GLOBAL(int32_t, 0x00601e1c);
            const uint32_t result = static_cast<uint32_t>(static_cast<uint64_t>(p3) >> 14);
            slots[slot].Unknown1D4 = result;

            // shrd eax, edx, 14: CF = last bit out, AF = 0 (pinned oracle model), OF = bit31(prev ^ result)
            const uint32_t prev = static_cast<uint32_t>(static_cast<uint64_t>(p3) >> 13);
            flags = (prev & 1u) | parityFlag8(result) | (result == 0u ? 0x40u : 0u)
                  | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
                  | ((((prev ^ result) >> 31) & 1u) != 0u ? 0x800u : 0u);

            r->eax = result;
            r->edx = static_cast<uint32_t>(static_cast<uint64_t>(p3) >> 32);
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00417c6c, FUN_00417c6c, "regs(eax:u32, ecx:u32, edx:u32, esi:ptr[0xa2]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_004178e0")

#include <gp4/gp4.h>

static inline uint32_t subtractionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (left < right ? 1u : 0u)
        | ((parity & 1u) == 0u ? 4u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004134df
// Blends the signed word at [esi+0x23a] with a term built from DAT_00638d20, DAT_00603578 and
// DAT_0060384c (Q14 / Q16 fixed point), clamps the magnitude to DAT_006013ac and stores the
// signed result back. Registers eax/edx/ebp and the flags of the final neg/cmp are left behind.
void __cdecl FUN_004134df(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t ebp = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, body, 0x23a)));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    const uint32_t input = GP4_GLOBAL(uint32_t, 0x00638d20);

    bool scaled = true;
    if (GP4_GLOBAL(uint8_t, 0x006036d7) != 0) {
        if (input == 0) {
            eax = 0;
            scaled = false;
        } else if (ebp != 0) {
            edx = input ^ ebp;
            if (static_cast<int32_t>(edx) < 0) {
                ebp = 0;
            }
        }
    } else if (input == 0) {
        const int64_t product = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060357c)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 16);
        edx = ebp;
        if (static_cast<int32_t>(ebp) >= 0) {
            eax = 0u - eax;
        }
        eax += ebp;
        edx ^= eax;
        if (static_cast<int32_t>(edx) < 0) {
            eax = 0;
        }
        scaled = false;
    }

    if (scaled) {
        const int64_t first = static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603578)) * static_cast<int64_t>(static_cast<int32_t>(input));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(first) >> 14);
        const int64_t second = static_cast<int64_t>(static_cast<int32_t>(eax)) * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x0060384c));
        eax = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 16);
        edx = static_cast<uint32_t>(static_cast<uint64_t>(second) >> 32);
        eax += ebp;
    }

    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006013ac);
    uint32_t flags;
    if (static_cast<int32_t>(eax) < 0) {
        uint32_t magnitude = 0u - eax;
        if (!(magnitude < limit)) {
            magnitude = limit;
        }
        eax = 0u - magnitude;
        flags = subtractionFlags32(0u, magnitude);
    } else {
        flags = subtractionFlags32(eax, limit);
        if (!(eax < limit)) {
            eax = limit;
        }
    }

    GP4_FIELD(uint16_t, body, 0x23a) = static_cast<uint16_t>(eax);

    r->eax = eax;
    r->edx = edx;
    r->ebp = ebp;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004134df, FUN_004134df, "regs(edx:u32, esi:ptr[0x242]:bytes) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

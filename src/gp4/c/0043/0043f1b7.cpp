#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a 32-bit `cmp left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0043f1b7
// Scans the 0x160-byte track-segment records from 0x8943a0 up to the lap end [0x866ecc]. Each record's
// signed word +0x16 goes through the clamp test 0x43ecff (bytes [0x627b61] sign, [0x627b62] inside the
// limit). The byte [0x627b60] says whether a run is open: closed, 0x43edc1 decides (carry) whether this
// record opens one (start index [0x627b28] = record index, sign [0x627b65]); open, 0x43edaf decides
// whether it ends, and then 0x43ef49 processes the range [0x627b28]..[0x627b24] (record index) and the
// run closes again when the clamp byte is set, else restarts at this record. [0x627b63] / [0x627b64]
// follow the sign / inside bytes of each record. Registers on return: ecx = record count, edi = the
// pointer past the last record, eax / edx = the last clamp results (al replaced by the sign byte);
// the flags are those of `cmp edi, [0x866ecc]`.
void __cdecl FUN_0043f1b7(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.st_xin_mask = 0;
    s.st_xout_mask = 0;

    GP4_GLOBAL(uint8_t, 0x00627b60) = 0u;
    s.edi = 0x008943a0u;
    s.ecx = 0u;

    // The first record only primes the sign / inside copies.
    s.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, s.edi, 0x16)));
    gp4::call_regs(0x0043ecff, s);
    GP4_GLOBAL(uint8_t, 0x00627b64) = GP4_GLOBAL(uint8_t, 0x00627b62);
    GP4_GLOBAL(uint8_t, 0x00627b63) = GP4_GLOBAL(uint8_t, 0x00627b61);
    s.edi += 0x160u;
    s.ecx += 1u;

    do {
        s.eax = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int16_t, s.edi, 0x16)));
        gp4::call_regs(0x0043ecff, s);

        if (GP4_GLOBAL(uint8_t, 0x00627b60) == 0u) {
            gp4::call_regs(0x0043edc1, s);
            if ((s.eflags & gp4::CF) != 0u) {
                GP4_GLOBAL(uint8_t, 0x00627b65) = GP4_GLOBAL(uint8_t, 0x00627b61);
                GP4_GLOBAL(uint32_t, 0x00627b28) = s.ecx;
                GP4_GLOBAL(uint8_t, 0x00627b60) = 0xffu;
            }
        } else {
            gp4::call_regs(0x0043edaf, s);
            if ((s.eflags & gp4::CF) != 0u) {
                GP4_GLOBAL(uint32_t, 0x00627b24) = s.ecx;
                gp4::call_regs(0x0043ef49, s);
                if (GP4_GLOBAL(uint8_t, 0x00627b62) != 0u) {
                    GP4_GLOBAL(uint8_t, 0x00627b60) = 0u;
                } else {
                    GP4_GLOBAL(uint8_t, 0x00627b60) = 0xffu;
                    s.eax = GP4_GLOBAL(uint32_t, 0x00627b24);
                    GP4_GLOBAL(uint32_t, 0x00627b28) = s.eax;
                    GP4_GLOBAL(uint8_t, 0x00627b65) = GP4_GLOBAL(uint8_t, 0x00627b61);
                }
            }
        }

        GP4_GLOBAL(uint8_t, 0x00627b64) = GP4_GLOBAL(uint8_t, 0x00627b62);
        s.eax = (s.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x00627b61);
        GP4_GLOBAL(uint8_t, 0x00627b63) = GP4_GLOBAL(uint8_t, 0x00627b61);
        s.edi += 0x160u;
        s.ecx += 1u;
    } while (s.edi < GP4_GLOBAL(uint32_t, 0x00866ecc));

    r->eax = s.eax;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->edi = s.edi;
    r->eflags = (r->eflags & ~kArithFlags) | cmpFlags32(s.edi, GP4_GLOBAL(uint32_t, 0x00866ecc));
}
GP4_IMPL(0x0043f1b7, FUN_0043f1b7, "regs() -> (eax, ecx, edx, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0043ed48+ring_0043edd3+ring_008943a0+words_008943a0+idx_0043ef49")

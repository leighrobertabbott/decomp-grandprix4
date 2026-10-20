#include <gp4/gp4.h>

static inline void swapRegs(uint32_t& a, uint32_t& b) {
    const uint32_t t = a;
    a = b;
    b = t;
}

static inline uint32_t withLow16(uint32_t reg, uint32_t value) {
    return (reg & 0xffff0000u) | (value & 0xffffu);
}

// FUNCTION: GP4 0x0040abc1
// Replays a packed key-frame stream at esi on car record 0 (0x66fef0): every record is decoded by 0x40aadb
// (header in bp, count at 0x62ac44), the car's lateral/heading words (+0x90, +0x18) are updated from it,
// 0x40a217 orients the car and then, for each of the record's steps, the segment at edi (starting at the
// lap start [0x891504]) receives the relative heading (+0x18) and lateral (+0x16) words, edi advances one
// 0x160-byte segment (pit/lap wrap) and 0x40a572 projects the car onto it; a per-step delta
// [0x62ac68] is added to the running value [0x62ac64] (0x40aa7c). A zero header (CF) ends the walk.
// Returns eax = the stream cursor after the last record; every other register is restored (the callees'
// register results are threaded through the same register file as the original).
void __cdecl FUN_0040abc1(gp4::Regs* r) {
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;

    swapRegs(st.esi, st.ebx);               // xchg esi, ebx: ebx = stream, esi = (old ebx)
    st.esi = 0x0066fef0u;
    st.edi = GP4_GLOBAL(uint32_t, 0x00891504);
    GP4_FIELD(uint32_t, st.esi, 0x1c) = st.edi;
    GP4_FIELD(uint16_t, st.esi, 0x18) = GP4_FIELD(uint16_t, st.edi, 0);
    const uint16_t first = GP4_FIELD(uint16_t, st.ebx, 0);
    if ((first & 0x800u) == 0u && (first & 0x8000u) != 0u) {
        const uint16_t word = GP4_FIELD(uint16_t, st.ebx, 2);
        st.eax = withLow16(st.eax, word);
        GP4_FIELD(uint16_t, st.esi, 0x90) = word;
    }

    for (;;) {
        swapRegs(st.esi, st.ebx);           // 0x40abf7
        GP4_GLOBAL(uint32_t, 0x0062ac60) = st.esi;
        gp4::call_regs(0x0040aadb, st);
        GP4_GLOBAL(uint32_t, 0x0062ac44) = GP4_GLOBAL(uint32_t, 0x0062ac48);
        swapRegs(st.esi, st.ebx);
        if ((st.eflags & 1u) != 0u) {
            break;
        }

        const uint32_t header = st.ebp & 0xffffu;
        if ((header & 0x800u) != 0u && (header & 0x8000u) != 0u) {
            GP4_FIELD(uint16_t, st.esi, 0x90) = GP4_GLOBAL(uint16_t, 0x0062ac5c);
            const uint16_t heading = GP4_GLOBAL(uint16_t, 0x0062ac5e);
            st.eax = withLow16(st.eax, heading);
            GP4_FIELD(uint16_t, st.esi, 0x18) = heading;
        }
        if (GP4_GLOBAL(uint32_t, 0x0062ac64) == 0u) {
            const uint16_t delta = GP4_GLOBAL(uint16_t, 0x0062ac5a);
            st.ecx = withLow16(st.ecx, delta);
            GP4_FIELD(uint16_t, st.esi, 0x18) = static_cast<uint16_t>(GP4_FIELD(uint16_t, st.esi, 0x18) + delta);
        }
        if ((header & 0x800u) == 0u && (header & 0x8000u) != 0u) {
            st.edx = GP4_GLOBAL(uint32_t, 0x0062ac60);
            GP4_FIELD(uint16_t, st.edx, 2) = GP4_FIELD(uint16_t, st.esi, 0x90);
            const uint16_t heading = GP4_FIELD(uint16_t, st.esi, 0x18);
            st.eax = withLow16(st.eax, heading);
            GP4_FIELD(uint16_t, st.edx, 4) = heading;
        }
        gp4::call_regs(0x0040a217, st);

        for (;;) {
            const uint16_t relative = static_cast<uint16_t>(GP4_FIELD(uint16_t, st.esi, 0x18) - GP4_FIELD(uint16_t, st.edi, 0));
            GP4_FIELD(uint16_t, st.edi, 0x18) = relative;
            const uint16_t lateral = GP4_FIELD(uint16_t, st.esi, 0x90);
            st.eax = withLow16(st.eax, lateral);
            GP4_FIELD(uint16_t, st.edi, 0x16) = lateral;
            st.edi += 0x160u;
            if (st.edi >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                if (st.edi < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    st.edi -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                } else if (st.edi >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    st.edi = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                }
            }
            GP4_FIELD(uint32_t, st.esi, 0x1c) = st.edi;
            gp4::call_regs(0x0040a572, st);

            const uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0062ac44) - 1u;
            GP4_GLOBAL(uint32_t, 0x0062ac44) = remaining;
            if (remaining == 0u) {
                break;
            }
            if (GP4_GLOBAL(uint32_t, 0x0062ac64) != 0u) {
                st.eax = GP4_GLOBAL(uint32_t, 0x0062ac68);
                if (st.eax != 0u) {
                    GP4_GLOBAL(uint32_t, 0x0062ac64) += st.eax;
                    gp4::call_regs(0x0040aa7c, st);
                }
            }
        }
    }

    r->eax = st.ebx;                        // 0x40acff: xchg esi, ebx; mov eax, esi
    r->eflags = (r->eflags & ~0x8d5u) | (st.eflags & 0x8d5u);
}
GP4_IMPL(0x0040abc1, FUN_0040abc1, "regs(esi:ptr[0x40]:layout=stream_0040abc1) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_008943a0+cars_0066fef0")

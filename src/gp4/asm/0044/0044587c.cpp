#include <gp4/gp4.h>

static inline void putByte(uint32_t& cursor, int32_t step, uint8_t value) {
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(cursor)) = value;
    cursor += static_cast<uint32_t>(step);
}

static inline void putDword(uint32_t& cursor, int32_t step, uint32_t value) {
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(cursor)) = value;
    cursor += static_cast<uint32_t>(step * 4);
}

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit `inc value` (CF is not touched).
static inline uint32_t incFlags32(uint32_t value) {
    const uint32_t result = value + 1u;
    return parityFlag8(result)
        | (((value ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x7fffffffu ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0044587c
// Builds a network state packet at 0x7a97b0 (pushal/popal: every register is preserved, the flags are those
// of the last flag writer) and hands it to 0x51ef49 (message 0x6c, or 0x6d when [0x7a8e00] is set).
// al = the packet kind kept at [0x62f8eb]. The bytes are stored with stos in the caller's direction;
// the optional parts follow the bits of [0x62f8f4]: 0x20 the dword [0x62f8e0] (also moved to [0x62f8dc]
// and cleared), 1 the two dwords at 0x66d8be/0x66d8c2, 2 the clock [0x66d814], 4 the record at esi and 8 the
// record at [0x62f8b4] (both copied by 0x4457f9 when esi is not 0 / [0x62f8b4] is not -1). [0x7a97ac] gets the packet
// length and [0x62f93c] counts the packets.
void __cdecl FUN_0044587c(gp4::Regs* r) {
    gp4::Regs st = *r;
    st.st_in = 0;
    st.st_out = 0;
    const int32_t step = (r->eflags & 0x400u) != 0u ? -1 : 1;

    GP4_GLOBAL(uint8_t, 0x0062f8eb) = static_cast<uint8_t>(r->eax);
    uint32_t cursor = 0x007a97b0u;
    putByte(cursor, step, GP4_GLOBAL(uint8_t, 0x0062f8ef));
    putByte(cursor, step, GP4_GLOBAL(uint8_t, 0x007c2ead));
    putByte(cursor, step, 0u);
    st.ebx = cursor;
    putByte(cursor, step, 0u);
    putByte(cursor, step, GP4_GLOBAL(uint8_t, 0x007ab022));
    putByte(cursor, step, GP4_GLOBAL(uint8_t, 0x0062f8eb));

    uint8_t mask = 0u;
    if (GP4_GLOBAL(uint8_t, 0x007a8dfa) == 0xffu) {
        mask |= 0x40u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007a8df8) == 0xffu) {
        mask |= 0x80u;
    }
    if (GP4_GLOBAL(uint8_t, 0x007a8dfd) == 0xffu) {
        mask |= 0x08u;
        if (GP4_GLOBAL(uint8_t, 0x007a8df8) == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x007a8dfc) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x007a8e00) = 0xffu;
        }
    }
    if (GP4_GLOBAL(uint8_t, 0x007a8df8) == 0xffu && GP4_GLOBAL(uint8_t, 0x0062f8ff) == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x007a8e00) = 0xffu;
        mask |= 0x04u;
        GP4_GLOBAL(uint8_t, 0x007a8dff) = 0xffu;
    }
    GP4_GLOBAL(uint8_t, 0x0062f8ed) = mask;
    putByte(cursor, step, mask);
    putByte(cursor, step, GP4_GLOBAL(uint8_t, 0x008914e2));
    st.eax = (st.eax & ~0xffu) | GP4_GLOBAL(uint8_t, 0x008914e2);

    st.eax = GP4_GLOBAL(uint32_t, 0x0062f8f4);
    putDword(cursor, step, st.eax);
    if ((GP4_GLOBAL(uint32_t, 0x0062f8f4) & 0x20u) != 0u) {
        st.eax = GP4_GLOBAL(uint32_t, 0x0062f8e0);
        GP4_GLOBAL(uint32_t, 0x0062f8dc) = st.eax;
        putDword(cursor, step, st.eax);
        if (st.eax != 0u) {
            GP4_GLOBAL(uint8_t, 0x007a8e00) = 0xffu;
        }
        GP4_GLOBAL(uint32_t, 0x0062f8e0) = 0u;
    }
    if ((GP4_GLOBAL(uint32_t, 0x0062f8f4) & 1u) != 0u) {
        st.eax = GP4_GLOBAL(uint32_t, 0x0066d8be);
        putDword(cursor, step, st.eax);
        st.eax = GP4_GLOBAL(uint32_t, 0x0066d8c2);
        putDword(cursor, step, st.eax);
    }
    if ((GP4_GLOBAL(uint32_t, 0x0062f8f4) & 2u) != 0u) {
        st.eax = GP4_GLOBAL(uint32_t, 0x0066d814);
        putDword(cursor, step, st.eax);
    }
    if ((GP4_GLOBAL(uint32_t, 0x0062f8f4) & 4u) != 0u && st.esi != 0u) {
        st.edi = cursor;
        gp4::call_regs(0x004457f9, st);
        cursor = st.edi;
    }
    if ((GP4_GLOBAL(uint32_t, 0x0062f8f4) & 8u) != 0u) {
        st.esi = GP4_GLOBAL(uint32_t, 0x0062f8b4);
        if (st.esi != 0xffffffffu) {
            st.edi = cursor;
            gp4::call_regs(0x004457f9, st);
            cursor = st.edi;
        }
    }
    st.ecx = cursor - 0x007a97b0u;
    st.edi = cursor;
    GP4_GLOBAL(uint32_t, 0x007a97ac) = st.ecx;
    gp4::call_regs(0x0051ef49, st);

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x0062f93c);
    GP4_GLOBAL(uint32_t, 0x0062f93c) = count + 1u;
    const uint32_t flags = (st.eflags & 0x01u) | incFlags32(count);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0044587c, FUN_0044587c, "regs(eax:u32, esi:nptr[0x41c]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_0062f8b4")

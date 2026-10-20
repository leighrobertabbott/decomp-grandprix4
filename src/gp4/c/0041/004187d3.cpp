#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x004187d3
// Rebuilds the status bits 0..5 of the first dword of the entry selected through esi
// (0x00418901): bits 0, 1, 3 from the flag bytes 0x00625ab8/9/b (suppressed while the byte
// 0x00638d1c is set), bit 2 from 0x00625aba, bits 4 and 5 from "any of the dwords +0x48..0x54
// / +0x58..0x64 non-zero". The byte 0x00625abb is cleared first when esi+0x7a has bit 3.
// pushal / popal: every register is restored; only the flags of the last flag writer remain.
void __cdecl FUN_004187d3(gp4::Regs* r) {
    gp4::Regs callee = *r;
    gp4::call_regs(0x00418901, callee);
    uint32_t* const entry = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(callee.eax));
    const uint8_t* const record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    *entry &= 0xfffffffeu;
    if (GP4_GLOBAL(uint8_t, 0x00625ab8) != 0 && GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
        *entry |= 1u;
    }
    *entry &= 0xfffffffdu;
    if (GP4_GLOBAL(uint8_t, 0x00625ab9) != 0 && GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
        *entry |= 2u;
    }
    *entry &= 0xfffffffbu;
    if (GP4_GLOBAL(uint8_t, 0x00625aba) != 0) {
        *entry |= 4u;
    }
    if ((record[0x7a] & 8u) != 0) {
        GP4_GLOBAL(uint8_t, 0x00625abb) = 0;
    }
    *entry &= 0xfffffff7u;
    if (GP4_GLOBAL(uint8_t, 0x00625abb) != 0 && GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
        *entry |= 8u;
    }
    const uint32_t firstGroup = entry[0x12] | entry[0x13] | entry[0x14] | entry[0x15];
    *entry &= 0xffffffefu;
    if (firstGroup != 0 && GP4_GLOBAL(uint8_t, 0x00638d1c) == 0) {
        *entry |= 0x10u;
    }
    const uint32_t secondGroup = entry[0x16] | entry[0x17] | entry[0x18] | entry[0x19];
    *entry &= 0xffffffdfu;

    // Flags of the last writer: "cmp eax, 0" (zero), "test byte [0x638d1c], 0xff" or the
    // "or dword [edi], 0x20" result.
    uint32_t flags;
    if (secondGroup == 0) {
        flags = 0x44u;
    } else {
        const uint8_t gate = GP4_GLOBAL(uint8_t, 0x00638d1c);
        if (gate != 0) {
            flags = parityFlag(gate) | ((gate & 0x80u) != 0u ? 0x80u : 0u);
        } else {
            *entry |= 0x20u;
            const uint32_t value = *entry;
            flags = parityFlag(value)
                | (value == 0u ? 0x40u : 0u)
                | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004187d3, FUN_004187d3, "regs(esi:ptr[0x20a]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

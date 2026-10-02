#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// FUNCTION: GP4 0x0043b9ae
void __cdecl FUN_0043b9ae(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t selector = GP4_FIELD(uint8_t, body, 0x7c);
    uint32_t flags;
    if (selector == 0u) {
        // test al, 0xff of a zero byte: ZF=1, PF=1, the rest clear
        flags = 0x40u | 0x04u;
    } else {
        // and byte [(selector & 0x3f) - 1 + 0x0066d90a], 0x7f: clears bit 7 of the flag byte
        uint8_t* flagByte = GP4_ARRAY(uint8_t, 0x0066d90a) + ((selector & 0x3fu) - 1u);
        const uint32_t result = static_cast<uint32_t>(*flagByte) & 0x7fu;
        *flagByte = static_cast<uint8_t>(result);
        // logical op: CF=OF=AF=SF=0 (bit 7 is clear), ZF and PF from the stored byte
        flags = parityFlag(result) | (result == 0u ? 0x40u : 0u);
    }
    // eax is saved and restored around the body
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043b9ae, FUN_0043b9ae, "regs(esi:ptr[0x84]:bytes) -> (cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// pushal wrapper: when the signed byte at +0x7c of the record in esi is negative, stores a table
// word ((index & 0xff) << 6) at +0x60 and refreshes the derived values with 0x00408783. popal restores
// every register but not the flags (those of the last flag writer: the byte test, or 0x00408783).
// FUNCTION: GP4 0x00408892
void __cdecl FUN_00408892(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t flagByte = GP4_FIELD(uint8_t, record, 0x7c);
    // test byte [esi+0x7c], 0xff
    uint32_t flags = parityFlag(flagByte) | (flagByte == 0u ? 0x40u : 0u) | ((flagByte & 0x80u) != 0u ? 0x80u : 0u);
    if ((flagByte & 0x80u) != 0u) {
        uint32_t index;
        if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x40u) == 0u) {
            index = 4u;
        } else {
            index = GP4_GLOBAL(uint32_t, 0x00679b50) + 1u;
        }
        GP4_FIELD(uint16_t, record, 0x60) = static_cast<uint16_t>((index & 0xffu) << 6);
        gp4::Regs call = *r;
        gp4::call_regs(0x00408783, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408892, FUN_00408892, "regs(esi:ptr[0x1a4]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")

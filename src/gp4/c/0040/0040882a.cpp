#include <gp4/gp4.h>

// pushal wrapper: picks a small index from the byte flags at 0x007c2ea9 (sign bit: the +0x198 /
// +0x199 pair of the record in esi or the word at 0x007c2e96; bit 0x40: the dword at 0x00679b50
// plus one; otherwise 2 or 4 from the byte at 0x007c2e75), stores (index & 0xff) << 6 at word +0x60
// of the record, then lets 0x00408783 refresh the derived fields. popal restores every register but
// not the flags, which are those 0x00408783 leaves.
// FUNCTION: GP4 0x0040882a
void __cdecl FUN_0040882a(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    uint32_t index;
    if (static_cast<int8_t>(mode) < 0) {
        if (GP4_FIELD(uint8_t, record, 0x198) == 0u) {
            index = GP4_GLOBAL(uint16_t, 0x007c2e96) + 2u;
        } else {
            index = GP4_FIELD(uint8_t, record, 0x199) + 2u;
        }
    } else if ((mode & 0x40u) == 0u) {
        index = (GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u) ? 4u : 2u;
    } else {
        index = GP4_GLOBAL(uint32_t, 0x00679b50) + 1u;
    }
    GP4_FIELD(uint16_t, record, 0x60) = static_cast<uint16_t>((index & 0xffu) << 6);
    gp4::Regs call = *r;
    gp4::call_regs(0x00408783, call);
    r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u);
}
GP4_IMPL(0x0040882a, FUN_0040882a, "regs(esi:ptr[0x1a4]:layout=rec_0040882a+rec_004087aa) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=gain_00408790+state_004087aa")

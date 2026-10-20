#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags left by a byte-wide TEST: CF = OF = AF = 0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

struct Unknown_00434d98_Record {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0x122 - 0x92];
    uint8_t Flags122;
    uint8_t Unknown123[0x161 - 0x123];
    uint8_t Flags161;
    uint8_t Unknown162;
    uint8_t Flags163;
    uint8_t Unknown164[0x179 - 0x164];
    uint8_t Flags179;
};

// FUNCTION: GP4 0x00434d98
// `pushal ... popal` wrapper: every register comes back unchanged; the flags are those of the last
// flag writer, either the callee 0x0042784b or the `test byte [esi+0x163], 0x40` that exits early.
// With bit 6 of +0x179 and bit 1 of +0x161 both clear it clears bits 7 and 6 of +0x163 and bit 7 of
// +0x122; otherwise, unless bit 6 of +0x163 is set, it rebuilds bits 5 and 6 of +0x163 from the sign
// of the word at +0x90 and clears bit 7 of +0x122. Both reset paths call 0x0042784b.
void __cdecl FUN_00434d98(gp4::Regs* r) {
    Unknown_00434d98_Record* record =
        reinterpret_cast<Unknown_00434d98_Record*>(static_cast<uintptr_t>(r->esi));
    gp4::Regs saved = *r;
    gp4::Regs call = *r;
    uint32_t flags;

    if ((record->Flags179 & 0x40u) == 0u && (record->Flags161 & 2u) == 0u) {
        record->Flags163 = static_cast<uint8_t>(record->Flags163 & 0x7fu);
        record->Flags122 = static_cast<uint8_t>(record->Flags122 & 0x7fu);
        record->Flags163 = static_cast<uint8_t>(record->Flags163 & 0xbfu);
        gp4::call_regs(0x0042784b, call);
        flags = call.eflags;
    } else if ((record->Flags163 & 0x40u) != 0u) {
        flags = logicFlags8(record->Flags163 & 0x40u);
    } else {
        record->Flags163 = static_cast<uint8_t>(record->Flags163 & 0xdfu);
        if (record->Unknown90 < 0) {
            record->Flags163 = static_cast<uint8_t>(record->Flags163 | 0x20u);
        }
        record->Flags163 = static_cast<uint8_t>(record->Flags163 | 0x40u);
        record->Flags122 = static_cast<uint8_t>(record->Flags122 & 0x7fu);
        gp4::call_regs(0x0042784b, call);
        flags = call.eflags;
    }
    saved.eflags = (saved.eflags & ~0x8d5u) | (flags & 0x8d5u);
    *r = saved;
}
GP4_IMPL(0x00434d98, FUN_00434d98, "regs(esi:ptr[0x420]:layout=car_0042ba87) -> (cf, pf, af, zf, sf, of); globals=layout=state_00403800+state_004059ef")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// One 0x160-byte record of the array at 0x8943a0 (only the six bit-set dwords are named).
struct Unknown_00555c49_Record {
    uint8_t Unknown00[0x2c];
    uint32_t Mask2C;
    uint32_t Mask30;
    uint32_t Mask34;
    uint32_t Mask38;
    uint32_t Mask3C;
    uint32_t Mask40;
    uint8_t Unknown44[0x160 - 0x44];
};

// FUNCTION: GP4 0x00555c49
// pushal wrapper. Slot ecx (0..31) of the 32 record pointers at 0x874d38 marks where that user's
// walk starts; the record ebx must lie beyond it (equal is an int3 assertion). Unless the byte
// 0x874db8[ecx] is 0xff, every record strictly between the slot's record and ebx has bit ecx
// cleared from its six dwords +0x2c..+0x40. popal restores every GPR; the flags always end as the
// equal `cmp esi, ebx` (ZF = PF = 1).
void __cdecl FUN_00555c49(gp4::Regs* r) {
    const uint32_t target = r->ebx;
    const uint32_t index = r->ecx;
    const uint32_t bit = 1u << (index & 31u);

    uint32_t cursor = GP4_ARRAY(uint32_t, 0x00874d38)[index];
    if (target == cursor) {
        __debugbreak();
    }
    cursor = cursor + 0x160u;
    if (cursor != target) {
        if (GP4_ARRAY(uint8_t, 0x00874db8)[index] == 0xffu) {
            do {
                cursor = cursor + 0x160u;
            } while (cursor != target);
        } else {
            const uint32_t keep = ~bit;
            do {
                Unknown_00555c49_Record* record = reinterpret_cast<Unknown_00555c49_Record*>(static_cast<uintptr_t>(cursor));
                record->Mask30 = record->Mask30 & keep;
                record->Mask38 = record->Mask38 & keep;
                record->Mask34 = record->Mask34 & keep;
                record->Mask40 = record->Mask40 & keep;
                record->Mask3C = record->Mask3C & keep;
                record->Mask2C = record->Mask2C & keep;
                cursor = cursor + 0x160u;
            } while (cursor != target);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(cursor, target));
}
GP4_IMPL(0x00555c49, FUN_00555c49, "regs(ebx:rec[0x8943a0,0x160,1..25], ecx:i32[0..31]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00555c49")

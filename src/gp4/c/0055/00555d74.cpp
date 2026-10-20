#include <gp4/gp4.h>
#include <gp4/flags.h>

// One 0x160-byte record of the array at 0x008943a0 (only the six bit-set dwords are named).
#pragma pack(push, 1)
struct Unknown_00555d74_Record {
    uint8_t Unknown00[0x2c];
    uint32_t Mask2C;
    uint32_t Mask30;
    uint32_t Mask34;
    uint32_t Mask38;
    uint32_t Mask3C;
    uint32_t Mask40;
};
#pragma pack(pop)

// pushal wrapper (every GPR restored; the flags are those of the final `or [ebx+0x3c], eax`).
// Slot ecx (0..31) takes record ebx: 0x555c49 first clears bit ecx from the records between the slot's old
// record and ebx; then the mark byte 0x874db8[ecx] is set to 0xff, the record pointer 0x874d38[ecx] is set
// to ebx, and the bit ecx of the record's +0x2c mask is copied (kept in +0x2c) into the masks +0x30, +0x38,
// +0x34, +0x40 and +0x3c.
// FUNCTION: GP4 0x00555d74
void __cdecl FUN_00555d74(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00555c49, c);

    const uint32_t index = c.ecx;
    GP4_ARRAY(uint8_t, 0x00874db8)[index] = 0xffu;
    GP4_ARRAY(uint32_t, 0x00874d38)[index] = c.ebx;

    Unknown_00555d74_Record* record = reinterpret_cast<Unknown_00555d74_Record*>(static_cast<uintptr_t>(c.ebx));
    const uint32_t bits = (1u << (index & 31u)) & record->Mask2C;
    record->Mask30 = record->Mask30 | bits;
    record->Mask38 = record->Mask38 | bits;
    record->Mask34 = record->Mask34 | bits;
    record->Mask40 = record->Mask40 | bits;
    record->Mask3C = record->Mask3C | bits;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(record->Mask3C));
}
GP4_IMPL(0x00555d74, FUN_00555d74, "regs(ebx:rec[0x8943a0,0x160,1..25], ecx:i32[0..31]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=walk_00555c49")

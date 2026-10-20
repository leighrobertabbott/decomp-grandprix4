#include <gp4/gp4.h>

struct Unknown_00419070 {
    uint8_t Unknown00[0x2ac];
    uint32_t Deadlines[3];     // +0x2ac: tick limits, 0 = unused
    uint8_t Pending[3];        // +0x2b8: 1-based bit numbers to publish, 0 = none
};

// For each of the three pending slots (+0x2b8) of the record at esi whose deadline (+0x2ac) is
// non-zero and not after DAT_0066d814 (sign of limit - deadline clear): sets bit
// min(pending - 1, 31) in dword +0xa4 of the 0xa8-byte entry returned by 0x418901 and clears the
// slot. eax is the last entry address (unchanged when no slot fired), ecx the last bit number,
// ebx = 3; the flags are those of the final "cmp ebx, 3" (ZF and PF).
// FUNCTION: GP4 0x00419070
void __cdecl FUN_00419070(gp4::Regs* r) {
    Unknown_00419070* record = reinterpret_cast<Unknown_00419070*>(static_cast<uintptr_t>(r->esi));
    uint32_t entryAddress = r->eax;
    uint32_t bitNumber = r->ecx;

    for (uint32_t i = 0; i < 3u; ++i) {
        if (record->Pending[i] == 0u || record->Deadlines[i] == 0u) {
            continue;
        }
        const uint32_t remaining = GP4_GLOBAL(uint32_t, 0x0066d814) - record->Deadlines[i];
        if ((remaining & 0x80000000u) != 0u) {
            continue;
        }
        int32_t bit = static_cast<int32_t>(record->Pending[i]) - 1;
        if (bit < 0) {
            bit = 0;
        }
        if (bit > 0x1f) {
            bit = 0x1f;
        }
        gp4::Regs callee = *r;
        gp4::call_regs(0x00418901, callee);
        entryAddress = callee.eax;
        bitNumber = static_cast<uint32_t>(bit);
        GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(entryAddress)), 0xa4) |= 1u << bit;
        record->Pending[i] = 0u;
    }

    r->eax = entryAddress;
    r->ebx = 3u;
    r->ecx = bitNumber;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00419070, FUN_00419070, "regs(eax:u32, ecx:u32, esi:ptr[0x2c2]:bytes) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

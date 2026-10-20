#include <gp4/gp4.h>
#include <gp4/flags.h>

// Refreshes the HUD snapshot of the selected car when the object index [0x00679bd8] differs from the
// cached one [0x006f3b94], or either cache byte 0x006f3c00 / 0x006f3c02 is not 0xff: runs 0x00526465
// (register state passed through, eax = the index), copies the three snapshot dwords 0x00679c0c /
// 10 / 14 to 0x006f3b00 / 04 / 08, stores the record's dword +0x1c in 0x00679be0 and marks
// 0x006f3c02 = 0xff. Registers: eax = the index (no refresh) or the last copied dword, esi = the
// record [0x00679d30], edi = its dword +0x1c; flags = those of the last compare (no refresh) or of
// 0x00526465.
// FUNCTION: GP4 0x00524f4b
void __cdecl FUN_00524f4b(gp4::Regs* r) {
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00679bd8);
    uint32_t arith = gp4::flags::sub32(index, GP4_GLOBAL(uint32_t, 0x006f3b94));
    bool refresh = index != GP4_GLOBAL(uint32_t, 0x006f3b94);
    if (!refresh) {
        const uint8_t a = GP4_GLOBAL(uint8_t, 0x006f3c00);
        arith = gp4::flags::sub8(a, 0xffu);
        refresh = a != 0xffu;
        if (!refresh) {
            const uint8_t b = GP4_GLOBAL(uint8_t, 0x006f3c02);
            arith = gp4::flags::sub8(b, 0xffu);
            refresh = b != 0xffu;
        }
    }

    r->eax = index;
    if (!refresh) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    gp4::Regs c = *r;
    gp4::call_regs(0x00526465, c);
    GP4_GLOBAL(uint32_t, 0x006f3b00) = GP4_GLOBAL(uint32_t, 0x00679c0c);
    GP4_GLOBAL(uint32_t, 0x006f3b04) = GP4_GLOBAL(uint32_t, 0x00679c10);
    const uint32_t last = GP4_GLOBAL(uint32_t, 0x00679c14);
    GP4_GLOBAL(uint32_t, 0x006f3b08) = last;
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t tag = GP4_FIELD(uint32_t, record, 0x1c);
    GP4_GLOBAL(uint32_t, 0x00679be0) = tag;
    GP4_GLOBAL(uint8_t, 0x006f3c02) = 0xffu;

    r->eax = last;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = record;
    r->edi = tag;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00524f4b, FUN_00524f4b, "regs() -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30+gate_00524f4b")

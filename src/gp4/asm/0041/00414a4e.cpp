#include <gp4/gp4.h>

// FUNCTION: GP4 0x00414a4e
void __cdecl FUN_00414a4e(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t ebp = r->ebp;

    // dx = [esi+0xe] - [edi] (16-bit), ax = [esi+0x90]
    const uint16_t difference = static_cast<uint16_t>(
        GP4_FIELD(uint16_t, record, 0x0e) - GP4_FIELD(uint16_t, const_cast<uint8_t*>(other), 0));
    const uint16_t reference = GP4_FIELD(uint16_t, record, 0x90);

    // cx is the scratch: |difference| (mode >= 2) or difference ^ reference (mode < 2)
    uint16_t scratch;
    bool clear;
    if (GP4_FIELD(uint8_t, record, 0x235) < 2u) {
        scratch = static_cast<uint16_t>(difference ^ reference);
        clear = static_cast<int16_t>(scratch) >= 0;
    } else {
        scratch = difference;
        if (static_cast<int16_t>(scratch) < 0) {
            scratch = static_cast<uint16_t>(0u - scratch);
        }
        clear = static_cast<int16_t>(scratch) < 0x4000;
    }
    r->edx = (r->edx & 0xffff0000u) | difference;
    r->ecx = (r->ecx & 0xffff0000u) | scratch;
    r->eax = (r->eax & 0xffff0000u) | reference;

    uint8_t al;
    if (clear) {
        if (ebp == 0u && (GP4_FIELD(uint8_t, record, 0x71) & 0x10u) == 0u) {
            // test byte [esi+0x71], 0x10 -> 0: ZF = PF = 1, the rest clear
            r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
            return;
        }
        al = 0;
        GP4_FIELD(uint8_t, record, 0x71) &= 0xefu;
    } else {
        if (ebp == 0u && (GP4_FIELD(uint8_t, record, 0x71) & 0x10u) != 0u) {
            // test byte [esi+0x71], 0x10 -> 0x10: ZF = PF = 0, the rest clear
            r->eflags = r->eflags & ~0x8d5u;
            return;
        }
        al = 0xff;
        GP4_FIELD(uint8_t, record, 0x71) |= 0x10u;
    }

    gp4::Regs call = *r;
    call.eax = (call.eax & 0xffffff00u) | al;
    call.edx = 0x18;
    gp4::call_regs(0x00434090, call);
    call.eax = 0xc;
    gp4::call_regs(0x00434044, call);
    *r = call;
}
GP4_IMPL(0x00414a4e, FUN_00414a4e, "regs(eax:u32, ecx:u32, edx:u32, ebp:u32[0..1], edi:ptr[0x10]:bytes, esi:ptr[0x23d]:bytes) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

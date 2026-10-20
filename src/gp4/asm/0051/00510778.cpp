#include <gp4/gp4.h>
#include <gp4/flags.h>

// Car record (esi) test: when record flag byte +0x8d bit 1 is set, +0x7b bit 6 is clear and the
// car's telemetry slot (0x00418901: 0x625abc + byte[+0x202]*0xa8) has bit 4 of its dword +4 set,
// the byte 0x0089104f[eax] is bumped by 0x20 and the result is 1 unless that byte went negative.
// Every other case asks 0x0040b61c (record byte +0x230 bit 0) and returns its carry. The routine
// saves and restores ebx/ecx/edx/ebp/esi/edi, so only eax and the flags change; the flags are
// those of the last writer (the byte add, or the 0x0040b61c callee).
// FUNCTION: GP4 0x00510778
void __cdecl FUN_00510778(gp4::Regs* r) {
    uint8_t* rec = (uint8_t*)(uintptr_t)r->esi;
    uint32_t flags = r->eflags;
    uint32_t result = 0u;
    bool slow = true;

    uint32_t b = GP4_FIELD(uint8_t, rec, 0x8d) & 2u;
    flags = gp4::flags::merge(flags, gp4::flags::logic8(b));
    if (b != 0u) {
        b = GP4_FIELD(uint8_t, rec, 0x7b) & 0x40u;
        flags = gp4::flags::merge(flags, gp4::flags::logic8(b));
        if (b == 0u) {
            gp4::Regs c = *r;
            gp4::call_regs(0x00418901, c);
            const uint32_t slot = *(const uint32_t*)(uintptr_t)(c.eax + 4u);
            const uint32_t bit = slot & 0x10u;
            flags = gp4::flags::merge(flags, gp4::flags::logic32(bit));
            if (bit != 0u) {
                uint8_t& cell = GP4_ARRAY(uint8_t, 0x0089104f)[r->eax];
                flags = gp4::flags::merge(flags, gp4::flags::add8(cell, 0x20u));
                cell = (uint8_t)(cell + 0x20u);
                result = ((cell & 0x80u) != 0u) ? 0u : 1u;
                slow = false;
            }
        }
    }

    if (slow) {
        gp4::Regs c = *r;
        gp4::call_regs(0x0040b61c, c);
        flags = gp4::flags::merge(flags, c.eflags);
        result = (c.eflags & 1u) ? 1u : 0u;   // jae -> 0, else 1
    }

    r->eax = result;
    r->eflags = flags;
}
GP4_IMPL(0x00510778, FUN_00510778, "regs(eax:u32[0..255], esi:ptr[772]:layout=slot_00418942) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00418d9a+arr_00510778")

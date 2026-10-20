#include <gp4/gp4.h>

// pushal wrapper over the 22 cars (esi = 0..21): 0x0043bb15 (eax = car) selects the car, 0x0051f837 (eax = car,
// ebx = [0x007ad894], ecx = slot) returns the address of the car's slot-0 record (0x007adb54 + car * 0xf28 +
// ebx * 0xe4 + slot * 0x38), 0x0043d008 runs on it (ebx = record) and its four bytes +0x33..+0x36 are then copied
// to the same bytes of the slot 1, 2 and 3 records. All registers come back through popal; the flags are those
// of the final `cmp esi, 0x16` (equal: ZF and PF).
// FUNCTION: GP4 0x0043bcff
void __cdecl FUN_0043bcff(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    for (uint32_t car = 0; car != 0x16u; ++car) {
        c.esi = car;
        c.eax = car;
        gp4::call_regs(0x0043bb15, c);

        c.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
        c.ecx = 0;
        c.eax = car;
        gp4::call_regs(0x0051f837, c);
        c.ebx = c.eax;
        gp4::call_regs(0x0043d008, c);

        const uint8_t* source = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(c.ebx));
        const uint8_t byte33 = source[0x33];
        const uint8_t byte34 = source[0x34];
        const uint8_t byte35 = source[0x35];
        const uint8_t byte36 = source[0x36];
        const uint32_t savedEax = (c.eax & 0xffff0000u) | (static_cast<uint32_t>(byte34) << 8) | byte33;
        const uint32_t savedEdx = (c.edx & 0xffff0000u) | (static_cast<uint32_t>(byte36) << 8) | byte35;
        c.eax = savedEax;
        c.edx = savedEdx;

        for (uint32_t slot = 1; slot != 4u; ++slot) {
            c.ecx = slot;
            c.ebx = GP4_GLOBAL(uint32_t, 0x007ad894);
            c.eax = car;
            gp4::call_regs(0x0051f837, c);
            c.ebx = c.eax;
            c.edx = savedEdx;
            c.eax = savedEax;
            uint8_t* target = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.ebx));
            target[0x33] = byte33;
            target[0x34] = byte34;
            target[0x35] = byte35;
            target[0x36] = byte36;
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x0043bcff, FUN_0043bcff, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0042595c+scale_0062eade+trk_007ad894+circ_007adb50+codes_0043ba12+adj_0066d802+codes_007adb54+state_0043bcd4")

#include <gp4/gp4.h>

// Destination record (esi): three 16-bit angles are stored at +0x0e, +0x12 and +0x16.
struct Unknown_0040e6da {
    uint8_t Unknown00[0x0e];
    uint16_t Unknown0E;
    uint8_t Unknown10[0x02];
    uint16_t Unknown12;
    uint8_t Unknown14[0x02];
    uint16_t Unknown16;
    uint8_t Unknown18[0x08];
};
static_assert(sizeof(Unknown_0040e6da) == 0x20, "record size");

// FUNCTION: GP4 0x0040e6da
// Orientation update. The three 16-bit angles at 0x00638032 / 0x00638036 / 0x0063803a become a
// quaternion-style vector via 0x0040e0e5; its last three components are rotated by the 3x3 Q14
// block through 0x00401893 (result at 0x00623e30/34/38), multiplied with the stored orientation
// (0x0040dff6) and turned back into three angles by 0x0040e230, which are written to
// record.+0x0e / +0x12 / +0x16. The registers on exit are what those routines leave behind.
void __cdecl FUN_0040e6da(gp4::Regs* r) {
    Unknown_0040e6da* record = (Unknown_0040e6da*)(uintptr_t)r->esi;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.eax = (c.eax & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638032);
    c.ebx = (c.ebx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x00638036);
    c.ecx = (c.ecx & 0xffff0000u) | GP4_GLOBAL(uint16_t, 0x0063803a);
    gp4::call_regs(0x0040e0e5, c);

    const uint32_t savedEax = c.eax;   // push eax
    GP4_GLOBAL(uint32_t, 0x00623e24) = c.ebx;
    GP4_GLOBAL(uint32_t, 0x00623e28) = c.ecx;
    GP4_GLOBAL(uint32_t, 0x00623e2c) = c.edx;
    gp4::call_regs(0x00401893, c);

    c.ebx = GP4_GLOBAL(uint32_t, 0x00623e30);
    c.ecx = GP4_GLOBAL(uint32_t, 0x00623e34);
    c.edx = GP4_GLOBAL(uint32_t, 0x00623e38);
    c.eax = savedEax;                  // pop eax
    gp4::call_regs(0x0040dff6, c);
    gp4::call_regs(0x0040e230, c);

    record->Unknown0E = (uint16_t)c.eax;
    record->Unknown12 = (uint16_t)c.ebx;
    record->Unknown16 = (uint16_t)c.ecx;

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0040e6da, FUN_0040e6da, "regs(esi:ptr[0x20]:bytes) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rot_00623e3c")

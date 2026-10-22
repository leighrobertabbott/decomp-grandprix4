#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __cpuid(int cpuInfo[4], int function_id);

// Under pushal/popal: builds the processor description line in the buffer 0x00823100. The script
// text 0xcd is expanded there (0x54d798, edi ends after it), then the text pointed to by [0x83392c]
// when it is set (0x54d7c0), a blank, the " (mmx) " string 0x833985 when the byte [0x83392b] is
// set, and finally the 12-byte vendor identification of cpuid leaf 0 (ebx, edx, ecx), NUL
// terminated. popal restores every register and leaves the flags of the last flag writer,
// `add edi, 0xc`; then the line is drawn with 0x53c135 (esi = the menu item record) and its
// registers and flags come back.
// FUNCTION: GP4 0x0053b902
void __cdecl FUN_0053b902(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    c.edi = 0x00823100u;
    c.esi = 0xcdu;
    gp4::call_regs(0x0054d798, c);

    c.esi = GP4_GLOBAL(uint32_t, 0x0083392c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.esi));
    if (c.esi != 0u) {
        gp4::call_regs(0x0054d7c0, c);
    }

    c.eax = (c.eax & 0xffffff00u) | 0x20u;
    GP4_FIELD(uint8_t, c.edi, 0) = 0x20u;
    c.edi += 1u;

    const uint8_t mmx = GP4_GLOBAL(uint8_t, 0x0083392b);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(mmx, 0u));
    if (mmx != 0u) {
        c.esi = 0x00833985u;
        gp4::call_regs(0x0054d7c0, c);
    }

    int id[4];
    __cpuid(id, 0);
    GP4_FIELD(uint32_t, c.edi, 0) = static_cast<uint32_t>(id[1]);
    GP4_FIELD(uint32_t, c.edi, 4) = static_cast<uint32_t>(id[3]);
    GP4_FIELD(uint32_t, c.edi, 8) = static_cast<uint32_t>(id[2]);
    const uint32_t flags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.edi, 0xcu));
    c.edi += 0xcu;
    GP4_FIELD(uint8_t, c.edi, 0) = 0u;

    // popal: every register comes back, the flags stay.
    gp4::Regs out = *r;
    out.st_in = 0;
    out.st_out = 0;
    out.eflags = flags;
    gp4::call_regs(0x0053c135, out);
    *r = out;
}
GP4_IMPL(0x0053b902, FUN_0053b902, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x20]:bytes, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_all_00835846+list_00549db9+pool_00549db9+textout_0054d211; gfields={0x0083392c:nptr[0x40]:u8{=0,1,2,3,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,0x10,0x11,0x12,0x13,0x14,0x20,0x41,0x7f,0x80,0xff}, 0x0083392b:u8{=0,1}}")

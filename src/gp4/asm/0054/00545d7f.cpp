#include <gp4/gp4.h>
#include <gp4/flags.h>

// Race-time text: formats the millisecond count in eax as "H M S.mmm" at the cursor edi (script text
// ids 0x3d/0x3e/0x3f supply the unit labels, a number is printed through the decimal formatter
// 0x0054642c: hours unpadded, minutes and seconds padded to 2 with '0', milliseconds padded to 3).
// Each unit is only printed once a larger one was (ebp = -1 after a unit, 0 on entry from the callers)
// or when it is nonzero; a time that is zero all through, or eax >= 0x3e80000, prints eight '-'.
// Every register except edi is restored (push/pop); the flags are those of the last flag writer on the
// path taken.
static inline void PutByte(gp4::Regs& c, uint8_t value) {
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.edi)) = value;
    c.edi += 1u;
}

// FUNCTION: GP4 0x00545d7f
void __cdecl FUN_00545d7f(gp4::Regs* r) {
    gp4::Regs c = *r;

    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 0x3e80000u));
    bool dashes = true;
    if (c.eax < 0x3e80000u) {
        dashes = false;
        gp4::call_regs(0x0052ae08, c);

        // hours
        c.eax = GP4_GLOBAL(uint32_t, 0x008236b4);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
        if (c.eax != 0u) {
            c.ebx &= ~0xff00u;
            gp4::call_regs(0x0054642c, c);
            c.ebp = 0xffffffffu;
            c.esi = 0x3du;
            gp4::call_regs(0x0054d798, c);
            PutByte(c, 0x20);
        }

        // minutes
        c.eax = GP4_GLOBAL(uint32_t, 0x008236b8);
        c.ebx = (c.ebx & 0xffff0000u) | 0x4030u;
        c.ecx = (c.ecx & 0xffffff00u) | 2u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebp));
        bool minutes = true;
        if (c.ebp == 0u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
            if (c.eax == 0u) {
                minutes = false;
            } else {
                c.ebx &= ~0xff00u;
            }
        }
        if (minutes) {
            gp4::call_regs(0x0054642c, c);
            c.ebp = 0xffffffffu;
            c.esi = 0x3eu;
            gp4::call_regs(0x0054d798, c);
            PutByte(c, 0x20);
        }

        // seconds and milliseconds
        c.eax = GP4_GLOBAL(uint32_t, 0x008236bc);
        c.ebx = (c.ebx & 0xffff0000u) | 0x4030u;
        c.ecx = (c.ecx & 0xffffff00u) | 2u;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.ebp));
        bool seconds = true;
        if (c.ebp == 0u) {
            c.ebx &= ~0xff00u;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
            if (c.eax == 0u) {
                const uint32_t ms = GP4_GLOBAL(uint32_t, 0x008236c0);
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(ms, 0u));
                if (ms == 0u) {
                    seconds = false;
                    dashes = true;
                }
            }
        }
        if (seconds) {
            gp4::call_regs(0x0054642c, c);
            PutByte(c, GP4_GLOBAL(uint8_t, 0x0082974a));
            c.eax = GP4_GLOBAL(uint32_t, 0x008236c0);
            c.ebx = (c.ebx & 0xffff0000u) | 0x4030u;
            c.ecx = (c.ecx & 0xffffff00u) | 3u;
            gp4::call_regs(0x0054642c, c);
            c.esi = 0x3fu;
            gp4::call_regs(0x0054d798, c);
        }
    }
    if (dashes) {
        for (uint32_t i = 0; i < 8u; ++i) {
            PutByte(c, 0x2d);
        }
    }
    PutByte(c, 0);
    c.edi -= 1u;

    r->edi = c.edi;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00545d7f, FUN_00545d7f, "regs(eax:u32{0,1,500,999,1000,1001,59999,60000,60001,3599999,3600000,3600001,3660000,3661000,3661001,0x3e7ffff,0x3e80000,0xffffffff,0x80000000}, ebx:u32, ecx:u32, edx:u32, esi:u32, ebp:u32{0,0,0,1,0xffffffff}, edi:ptr[0x200]:bytes) -> (edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+script_0052ad37")

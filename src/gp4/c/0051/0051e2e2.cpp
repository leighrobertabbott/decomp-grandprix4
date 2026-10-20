#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets the status bits of the record in esi (+0x6f) from two input tests made through 0x0051e2b5 (CF = active):
// bit 0x10 when the input selected by the byte at 0x7ab558 is NOT active; when the bytes at 0x7ab559 and 0x7ab558
// differ, bits 0x80 and 0x10 when the input selected by the byte at 0x7ab559 is NOT active. cl is cleared and bl
// holds the last selector read; the flags are those of the last flag writer (the callee's, an `or`, or the `cmp`).
// FUNCTION: GP4 0x0051e2e2
void __cdecl FUN_0051e2e2(gp4::Regs* r) {
    uint8_t* const record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    r->ecx = r->ecx & 0xffffff00u;
    r->ebx = (r->ebx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007ab558);
    gp4::call_regs(0x0051e2b5, *r);
    if ((r->eflags & gp4::flags::CF) == 0u) {
        const uint32_t v = GP4_FIELD(uint8_t, record, 0x6f) | 0x10u;
        GP4_FIELD(uint8_t, record, 0x6f) = static_cast<uint8_t>(v);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(v));
    }

    const uint32_t second = GP4_GLOBAL(uint8_t, 0x007ab559);
    r->ebx = (r->ebx & 0xffffff00u) | second;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(second, GP4_GLOBAL(uint8_t, 0x007ab558)));
    if (second == GP4_GLOBAL(uint8_t, 0x007ab558)) {
        return;
    }

    r->ebx = (r->ebx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x007ab559);
    gp4::call_regs(0x0051e2b5, *r);
    if ((r->eflags & gp4::flags::CF) == 0u) {
        const uint32_t v1 = GP4_FIELD(uint8_t, record, 0x6f) | 0x80u;
        GP4_FIELD(uint8_t, record, 0x6f) = static_cast<uint8_t>(v1);
        const uint32_t v2 = v1 | 0x10u;
        GP4_FIELD(uint8_t, record, 0x6f) = static_cast<uint8_t>(v2);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(v2));
    }
}
GP4_IMPL(0x0051e2e2, FUN_0051e2e2, "regs(esi:ptr[0x80], ebx:u32, ecx:u32, eax:u32) -> (eax, ebx, ecx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=bits_006f2d90+ctl_007ab548+sel_0051e2e2")

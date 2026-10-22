#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Parser_0045a2c0 {
    uint8_t Unknown00[8];
    const uint8_t* Cursor08;
    uint8_t Unknown0C[4];
    uint8_t Flag10;
};
#pragma pack(pop)

// Unless the flag at +0x10 is set, moves the cursor at +8 to the next line break ('\n' or '\r') and then
// past the line-break run: at most one '\n' and one '\r' (in either order) are consumed. The original
// only loads al (the flag byte, or the character that ended the run; the other bytes of eax keep the
// phase-one cursor), sets dl to 0 / 1 and ends on the flags of its last compare. Its two "seen" flag
// bytes live in the slot reserved by `push ecx`, so the final `pop ecx` leaves bytes 2/3 of ecx set to
// the seen-'\n' / seen-'\r' flags.
// FUNCTION: GP4 0x0045a2c0
void __cdecl FUN_0045a2c0(gp4::Regs* r) {
    Parser_0045a2c0* self = (Parser_0045a2c0*)r->ecx;
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx & ~0xffu;
    uint8_t flag = self->Flag10;
    uint32_t flags;
    if (flag != 0) {
        r->eax = (eax & ~0xffu) | flag;
        r->edx = edx;
        flags = gp4::flags::sub8(flag, 0);
    } else {
        while (*self->Cursor08 != 0x0a && *self->Cursor08 != 0x0d) {
            self->Cursor08 = self->Cursor08 + 1;
        }
        uint32_t phaseOne = (uint32_t)(uintptr_t)self->Cursor08;
        bool seen[2] = {false, false};
        uint8_t c;
        for (;;) {
            const uint8_t* p = self->Cursor08;
            c = *p;
            if (c == 0x0a && !seen[0]) {
                seen[0] = true;
            } else if (c == 0x0d && !seen[1]) {
                seen[1] = true;
            } else {
                break;
            }
            self->Cursor08 = p + 1;
        }
        flags = (c == 0x0d) ? gp4::flags::logic8(1) : gp4::flags::sub8(c, 0x0d);
        r->eax = (phaseOne & ~0xffu) | c;
        r->edx = edx | 1u;
        r->ecx = (ecx & 0xffffu) | (seen[0] ? 0x00010000u : 0u) | (seen[1] ? 0x01000000u : 0u);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0045a2c0, FUN_0045a2c0, "regs(ecx:ptr[0x20]:{8:ptr[0x40]:u8{0x0a,0x0d,0x41}, 0x10:u8{=0,0,1}}, eax:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Dispatches the pending command word [0x7a97b0]: command = (word >> 24) & 0x7f. Commands above 0xc
// are returned as they are (eax = command). Otherwise, except for command 9, the handler
// [0x8307d8 + 4 * command] runs (eax = its table slot, esi = 0x7a97b4, the packet after the word;
// the handler consumes packet bytes and leaves esi wherever it stopped) and the three "refresh"
// bytes 0x81e4fb, 0x81e4fe and 0x82fd78 are set to 0xff. Then, if the current menu header
// [0x82fd34] is non-null, has bit 0x40 in its byte +0x11 and a nonzero handler dword +0x24, the
// byte 0x82fd5f is set to 0xff. eax = -1 for commands 0..0xc. [0x7a97ac] is cleared in every case.
// The flags are those of the last compare/test that ran (the handler's own flags are overwritten by
// the `or eax, eax` that follows).

// FUNCTION: GP4 0x00540574
void __cdecl FUN_00540574(gp4::Regs* r) {
    const uint32_t command = (GP4_GLOBAL(uint32_t, 0x007a97b0) >> 24) & 0x7fu;
    uint32_t eax = command;
    uint32_t esi = 0x007a97b4u;
    uint32_t flags = gp4::flags::sub32(command, 0xcu);
    if (command <= 0xcu) {
        flags = gp4::flags::sub32(command, 9u);
        if (command != 9u) {
            const uint32_t slot = (command << 2) + 0x008307d8u;
            gp4::Regs c = *r;
            c.eax = slot;
            c.esi = 0x007a97b4u;
            c.eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(command << 2, 0x008307d8u));
            gp4::call_regs(*reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(slot)), c);
            r->ecx = c.ecx;
            r->edx = c.edx;
            r->ebx = c.ebx;
            r->edi = c.edi;
            r->ebp = c.ebp;
            esi = c.esi;
            GP4_GLOBAL(uint8_t, 0x0081e4fb) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x0081e4fe) = 0xffu;
            GP4_GLOBAL(uint8_t, 0x0082fd78) = 0xffu;
        }
        const uint32_t header = GP4_GLOBAL(uint32_t, 0x0082fd34);
        flags = gp4::flags::logic32(header);
        if (header != 0u) {
            const uint8_t bits = GP4_FIELD(uint8_t, header, 0x11);
            flags = gp4::flags::logic8(bits & 0x40u);
            if ((bits & 0x40u) != 0u) {
                const uint32_t handler = GP4_FIELD(uint32_t, header, 0x24);
                flags = gp4::flags::sub32(handler, 0u);
                if (handler != 0u) {
                    GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
                }
            }
        }
        eax = 0xffffffffu;
    }
    GP4_GLOBAL(uint32_t, 0x007a97ac) = 0u;
    r->eax = eax;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00540574, FUN_00540574, "regs(ecx:u32, edx:u32, ebx:u32, edi:u32, ebp:u32) -> (eax, ecx, edx, ebx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=cmd_00540574")

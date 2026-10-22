#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands the control-coded string at esi into the buffer at edi (lodsb/stosb/movsb, so the direction
// flag in the incoming eflags selects the stepping; `dec esi`/`inc esi` always step by one):
//   0x00       end: store a NUL at [edi] and return (esi is past the terminator, edi on the NUL);
//   0x04       skip the digits that follow (0x0054d834, which returns eax = 0), skip one more byte and call
//              the handler table entry at 0x0054d7ac + eax * 4 (entry 0 is the only reachable one) with
//              esi/edi as the cursors; an eax of 5 or more would call the empty routine 0x0054e232 instead
//              (block 0x0054d7f4: dead, since 0x0054d834 always returns 0; exempted by the lead);
//   0x01..0x13 copy the run: the code byte itself and the following ones, dword [0x00833f98 + code * 4]
//              bytes in all (rep movsb);
//   others     copied as they are (stosb).
// eax, ebx, ecx and edx are pushed and popped around the loop, so they come back unchanged; the handler
// sees the live ecx (zero after a run copy). The flags on return are those of the `or al, al` that saw
// the NUL (CF = OF = AF = SF = 0, ZF = PF = 1).
// FUNCTION: GP4 0x0054d7c0
void __cdecl FUN_0054d7c0(gp4::Regs* r) {
    const uint32_t step = ((r->eflags & 0x400u) != 0u) ? 0xffffffffu : 1u;
    gp4::Regs live = *r;
    live.st_in = 0;
    live.st_out = 0;
    live.st_xin_mask = 0;
    live.st_xout_mask = 0;

    for (;;) {
        const uint32_t code = GP4_FIELD(uint8_t, live.esi, 0);
        live.esi += step;
        live.eax = (live.eax & 0xffffff00u) | code;
        if (code == 0u) {
            GP4_FIELD(uint8_t, live.edi, 0) = 0u;
            break;
        }
        if (code == 4u) {
            gp4::call_regs(0x0054d834, live);
            live.esi += 1u;
            const uint32_t index = live.eax;
            if (index >= 5u) {
                gp4::Regs unreachable = live;
                unreachable.esi = 0x0054d7dcu;
                gp4::call_regs(0x0054e232, unreachable);
                continue;
            }
            const uint32_t entry = (index << 2) + 0x0054d7acu;
            live.eax = entry;
            live.eflags = gp4::flags::merge(live.eflags, gp4::flags::sub32(index, 5u));
            gp4::call_regs(GP4_GLOBAL(uint32_t, entry), live);
        } else if (code < 0x14u) {
            live.esi -= 1u;
            live.eax = (code << 2) + 0x00833f98u;
            live.ecx = GP4_GLOBAL(uint32_t, live.eax);
            while (live.ecx != 0u) {
                GP4_FIELD(uint8_t, live.edi, 0) = GP4_FIELD(uint8_t, live.esi, 0);
                live.esi += step;
                live.edi += step;
                live.ecx -= 1u;
            }
        } else {
            GP4_FIELD(uint8_t, live.edi, 0) = static_cast<uint8_t>(code);
            live.edi += step;
        }
    }

    r->esi = live.esi;
    r->edi = live.edi;
    r->ebp = live.ebp;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(0u));
}
GP4_IMPL(0x0054d7c0, FUN_0054d7c0, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x40]:u8{0,0,1,2,3,4,5,0x13,0x14,0x30,0x31,0x41,0x42}, edi:ptr[0x400]:bytes) -> (esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=items_0054d7c0")

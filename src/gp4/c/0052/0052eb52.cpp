#include <gp4/gp4.h>
#include <gp4/flags.h>

// Builds the 0x1b-byte record at 0x00826d2a (after clearing it with 0x0052e976 and stamping 0xabadfeed at
// 0x00826d25 and the command byte al at 0x00826d29) for the command in eax & 0x7f, dispatched through the
// table at 0x0052eb98 (entries >= 0x14 use entry 0x13, command 0x20 does nothing):
//   0          the driver names of the table at 0x007c2b3c (0x18 bytes apart), their characters >= 0x20
//              copied and separated by commas, at most 0x1b bytes;
//   3, 0xd-0xf the low byte of [0x007ad894];
//   5-0xc, 0x13  that byte, then the counts of 0x0052ecce, the byte [0x008293fc] and, when it is 0xff, the
//              items of 0x0052ecdc / 0x0052ecb0 / 0x0052ec6c;
//   1, 2, 4, 0x10-0x12  nothing.
// pushal/popal restore every register; the flags are those of the last flag writer on the path.
// FUNCTION: GP4 0x0052eb52
void __cdecl FUN_0052eb52(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0052e976, c);
    GP4_GLOBAL(uint32_t, 0x00826d25) = 0xabadfeedu;
    GP4_GLOBAL(uint8_t, 0x00826d29) = static_cast<uint8_t>(c.eax);
    uint32_t edi = 0x00826d2au;
    uint32_t ecx = 0x1bu;
    const uint32_t edx = GP4_GLOBAL(uint32_t, 0x007ad894);
    uint32_t index = c.eax & 0x7fu;

    uint32_t flags = gp4::flags::sub32(index, 0x20u);
    if (index != 0x20u) {
        flags = gp4::flags::sub32(index, 0x14u);
        if (index >= 0x14u) {
            index = 0x13u;
        }
        flags = gp4::flags::add32(index << 2, 0x0052eb98u);

        switch (index) {
        case 0: {
            uint32_t ebx = 0x007c2b3cu;
            uint32_t ebp = 0x28u;
            for (;;) {
                const uint8_t* esi = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(ebx));
                bool exhausted = false;
                uint32_t al;
                for (;;) {
                    al = *esi++;
                    flags = gp4::flags::sub8(al, 0x20u);
                    if (al < 0x20u) {
                        break;
                    }
                    GP4_FIELD(uint8_t, edi, 0) = static_cast<uint8_t>(al);
                    ++edi;
                    --ecx;
                    if (ecx == 0u) {
                        exhausted = true;
                        break;
                    }
                }
                if (exhausted) {
                    break;
                }
                flags = gp4::flags::sub32(ecx, 8u);
                if (ecx < 8u) {
                    break;
                }
                GP4_FIELD(uint8_t, edi, 0) = 0x2cu;
                ++edi;
                flags = gp4::flags::dec32(ecx, flags);
                --ecx;
                flags = gp4::flags::add32(ebx, 0x18u);
                ebx += 0x18u;
                flags = gp4::flags::dec32(ebp, flags);
                --ebp;
                if (ebp == 0u) {
                    break;
                }
            }
            break;
        }
        case 3:
        case 0xd:
        case 0xe:
        case 0xf:
            GP4_FIELD(uint8_t, edi, 0) = static_cast<uint8_t>(edx);
            flags = gp4::flags::inc32(edi, flags);
            ++edi;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 0x13: {
            GP4_FIELD(uint8_t, edi, 0) = static_cast<uint8_t>(edx);
            flags = gp4::flags::inc32(edi, flags);
            ++edi;
            c.edi = edi;
            c.eflags = gp4::flags::merge(c.eflags, flags);
            gp4::call_regs(0x0052ecce, c);
            const uint32_t item = GP4_GLOBAL(uint8_t, 0x008293fc);
            GP4_FIELD(uint8_t, c.edi, 0) = static_cast<uint8_t>(item);
            ++c.edi;
            c.eax = (c.eax & 0xffffff00u) | item;
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(item, 0xffu));
            if (item == 0xffu) {
                gp4::call_regs(0x0052ecdc, c);
                gp4::call_regs(0x0052ecb0, c);
                gp4::call_regs(0x0052ec6c, c);
            }
            flags = c.eflags;
            break;
        }
        default:
            break;
        }
    }

    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052eb52, FUN_0052eb52, "regs(eax:u32[0..0x24]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=drv_005117d3+strs_0052ec6c+msg_0052eb52")

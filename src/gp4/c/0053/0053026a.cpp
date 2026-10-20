#include <gp4/gp4.h>
#include <gp4/flags.h>

// Toggles bit 7 of the byte at esi + 0x1f according to the state word pair of the current record
// ebx (selected by 0x00531803) and the mode in edx. A record is "full" when
// (dword0 & 0x1867f83c) == 0x1867f83c and (dword1 & 0x16) == 0x16.
//   edx == 1: bit 7 := full.
//   edx == 2: a full record is cleared (dword0 &= 0xe79807c3, dword1 &= ~0x16, bit 7 cleared); any
//             other record is filled (dword0 |= 0x99e7ff3c, dword1 |= 0x16, bit 7 set).
// edx is cleared on exit; eax keeps the last masked value and the flags are those of the last
// compare / logic operation.
// FUNCTION: GP4 0x0053026a
void __cdecl FUN_0053026a(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00531803, c);

    uint32_t* rec = reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(c.ebx));
    uint8_t* flagByte = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(c.esi + 0x1fu));
    uint32_t eax = c.eax;
    uint32_t flags;
    if (c.edx == 1u) {
        *flagByte = static_cast<uint8_t>(*flagByte & 0x7fu);
        eax = rec[0] & 0x1867f83cu;
        flags = gp4::flags::sub32(eax, 0x1867f83cu);
        if (eax == 0x1867f83cu) {
            eax = rec[1] & 0x16u;
            flags = gp4::flags::sub32(eax, 0x16u);
            if (eax == 0x16u) {
                *flagByte = static_cast<uint8_t>(*flagByte | 0x80u);
                flags = gp4::flags::logic8(*flagByte);
            }
        }
    } else {
        flags = gp4::flags::sub32(c.edx, 2u);
        if (c.edx == 2u) {
            bool full = false;
            eax = rec[0] & 0x1867f83cu;
            if (eax == 0x1867f83cu) {
                eax = rec[1] & 0x16u;
                full = (eax == 0x16u);
            }
            if (full) {
                rec[0] &= 0xe79807c3u;
                rec[1] &= 0xffffffe9u;
                *flagByte = static_cast<uint8_t>(*flagByte & 0x7fu);
            } else {
                rec[0] |= 0x99e7ff3cu;
                rec[1] |= 0x16u;
                *flagByte = static_cast<uint8_t>(*flagByte | 0x80u);
            }
            flags = gp4::flags::logic8(*flagByte);
        }
    }
    c.eax = eax;
    c.edx = 0u;
    *r = c;
    r->eflags = gp4::flags::merge(c.eflags, flags);
}
GP4_IMPL(0x0053026a, FUN_0053026a, "regs(edx:u32{=0,1,2,3}, esi:ptr[0x40]:bytes) -> (eax, ebx, edx, cf, pf, af, zf, sf, of); globals=layout=sel_00531859+rsrec_007ad638")

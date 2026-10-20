#include <gp4/gp4.h>
#include <gp4/flags.h>

// Searches the rows of the surface [0x7a8e18] (row pitch [0x6975c4]), starting at row
// 0x679b9c[2], for at most [0x7a8e1c] rows whose first dword is 3 and whose second dword + 0xcb2
// is above (signed) the clock [0x66d814]. Sets [0x679b44] = 2. Found: CF cleared (flags of that
// compare, ebp = row index, esi = row address, edx = second dword + 0xcb2). Not found: CF set
// (flags of the last `add esi, pitch`, ebp = row index after the last row, ecx = 0). eax ends as
// the first dword of the last row read; edx is the high half of the imul of row index * pitch
// unless a row with first dword 3 was met.
// FUNCTION: GP4 0x00520c08
void __cdecl FUN_00520c08(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00679b44) = 2u;
    uint32_t esi = GP4_GLOBAL(uint32_t, 0x007a8e18);
    uint32_t ebp = GP4_ARRAY(uint32_t, 0x00679b9c)[2];
    const int64_t product = static_cast<int64_t>(static_cast<int32_t>(ebp)) *
                            static_cast<int64_t>(static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x006975c4)));
    uint32_t eax = static_cast<uint32_t>(product);
    uint32_t edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    esi += eax;
    uint32_t ecx = GP4_GLOBAL(uint32_t, 0x007a8e1c);
    uint32_t arith;
    for (;;) {
        eax = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        if (eax == 3u) {
            edx = GP4_FIELD(uint32_t, esi, 4) + 0xcb2u;
            arith = gp4::flags::sub32(edx, GP4_GLOBAL(uint32_t, 0x0066d814));
            if (static_cast<int32_t>(edx) > static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d814))) {
                arith &= ~gp4::flags::CF;
                break;
            }
        }
        ebp += 1u;
        const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x006975c4);
        arith = gp4::flags::add32(esi, pitch);
        esi += pitch;
        ecx -= 1u;
        if (ecx == 0u) {
            arith |= gp4::flags::CF;
            break;
        }
    }
    r->eax = eax;
    r->ebp = ebp;
    r->ecx = ecx;
    r->edx = edx;
    r->esi = esi;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00520c08, FUN_00520c08, "regs() -> (eax, ebp, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rows_00520584+rows_00520c08+scan_00520c08")

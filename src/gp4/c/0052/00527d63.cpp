#include <gp4/gp4.h>
#include <gp4/flags.h>

// Reads and checks the header of the open file [0x0081ea28]: ReadFile (thunk 0x005a48f4) of 0x20
// bytes into 0x008174d8 (count -> [0x0081ea38]); on success the first 0x1c bytes are compared
// (`repe cmpsb`) with the signature at 0x008174bc. A match reads [0x008174f4] * 0x18 more bytes to
// 0x008174f8 (the request size is kept in [0x0081ea34]); CF is clear only when that second ReadFile
// returned 1 with the full count (ebx = the count read), else DebugBreak (thunk 0x005a4930) runs.
// Every failure calls GetLastError (thunk 0x005a48b2) and ends with `stc`. All calls keep the live
// registers and flags (ecx/esi/edi as left by the compare).
// FUNCTION: GP4 0x00527d63
void __cdecl FUN_00527d63(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    const uint32_t countAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x0081ea38)));
    const uint32_t headerAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x008174d8)));

    const uint32_t first[5] = {GP4_GLOBAL(uint32_t, 0x0081ea28), headerAddr, 0x20u, countAddr, 0u};
    gp4::call_regs_stdcall(0x005a48f4, c, first);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    bool failed = c.eax == 0u;
    if (!failed) {
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(GP4_GLOBAL(uint32_t, 0x0081ea38), 0x20u));
        failed = GP4_GLOBAL(uint32_t, 0x0081ea38) != 0x20u;
    }
    if (failed) {
        gp4::call_regs(0x005a48b2, c);
        c.eflags |= gp4::CF;
        r->eax = c.eax;
        r->ecx = c.ecx;
        r->edx = c.edx;
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }

    const uint8_t* const signature = GP4_ARRAY(uint8_t, 0x008174bc);
    const uint8_t* const header = GP4_ARRAY(uint8_t, 0x008174d8);
    uint32_t done = 0u;
    uint32_t arith;
    bool equal;
    do {
        arith = gp4::flags::sub8(signature[done], header[done]);
        equal = signature[done] == header[done];
        ++done;
    } while (equal && done < 0x1cu);
    c.esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(signature)) + done;
    c.edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(header)) + done;
    c.ecx = 0x1cu - done;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    if (!equal) {
        gp4::call_regs(0x005a48b2, c);
        c.eflags |= gp4::CF;
    } else {
        const uint32_t size = GP4_GLOBAL(uint32_t, 0x008174f4) * 0x18u;
        GP4_GLOBAL(uint32_t, 0x0081ea34) = size;
        const uint32_t second[5] = {GP4_GLOBAL(uint32_t, 0x0081ea28),
                                    static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&GP4_GLOBAL(uint8_t, 0x008174f8))),
                                    size, countAddr, 0u};
        gp4::call_regs_stdcall(0x005a48f4, c, second);
        c.ebx = GP4_GLOBAL(uint32_t, 0x0081ea38);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.eax, 1u));
        if (c.eax == 1u) {
            c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ebx, GP4_GLOBAL(uint32_t, 0x0081ea34)));
            if (c.ebx == GP4_GLOBAL(uint32_t, 0x0081ea34)) {
                c.eflags &= ~gp4::CF;
                r->eax = c.eax;
                r->ebx = c.ebx;
                r->ecx = c.ecx;
                r->edx = c.edx;
                r->esi = c.esi;
                r->edi = c.edi;
                r->eflags = gp4::flags::merge(r->eflags, c.eflags);
                return;
            }
        }
        gp4::call_regs(0x005a4930, c);
        c.eflags |= gp4::CF;
    }
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00527d63, FUN_00527d63, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")

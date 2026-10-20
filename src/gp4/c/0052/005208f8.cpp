#include <gp4/gp4.h>
#include <gp4/flags.h>

// With byte [0x007aafd4] == 0 (otherwise: flags of that test, nothing else), writes the clock
// [0x0066d814] into the record at the pointer [0x007a8e30] as words (+0 low half, +2 = 0x8000,
// +4 high half), clears [0x006f2f69] and asks 0x00510fad (esi = the record, edi = [0x0066d834],
// ecx = 8, edx = 0, eax = clock >> 16) to run the dispatcher (a cached temp-file write of the
// record at offset edi); when that leaves CF clear [0x0066d834] advances by 8. Registers: eax is
// the callee's, ecx/edx/esi/edi keep the values loaded for it; the flags are the callee's, or
// those of `add [0x0066d834], 8`.
// FUNCTION: GP4 0x005208f8
void __cdecl FUN_005208f8(gp4::Regs* r) {
    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x007aafd4);
    if (gate != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(gate));
        return;
    }

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x007a8e30)));
    GP4_FIELD(uint16_t, record, 2) = 0x8000u;
    const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
    GP4_FIELD(uint16_t, record, 0) = static_cast<uint16_t>(clock);
    GP4_FIELD(uint16_t, record, 4) = static_cast<uint16_t>(clock >> 16);
    GP4_GLOBAL(uint8_t, 0x006f2f69) = 0u;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = clock >> 16;
    c.edx = 0u;
    c.esi = GP4_GLOBAL(uint32_t, 0x007a8e30);
    c.edi = GP4_GLOBAL(uint32_t, 0x0066d834);
    c.ecx = 8u;
    gp4::call_regs(0x00510fad, c);

    r->eax = c.eax;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }
    const uint32_t counter = GP4_GLOBAL(uint32_t, 0x0066d834);
    GP4_GLOBAL(uint32_t, 0x0066d834) = counter + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(counter, 8u));
}
GP4_IMPL(0x005208f8, FUN_005208f8, "regs(eax:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cache_0053832b+rec_005208f8")

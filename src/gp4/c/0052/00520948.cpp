#include <gp4/gp4.h>
#include <gp4/flags.h>

// Replay-log writer for the current car [0x0066d820] (null: nothing happens, eax = 0 and the flags of
// `or eax, eax`). The car's event fields are packed into the 8-byte record at [0x007a8e30]: byte 0 = key byte
// [0x00679d1b] & 0x60 | car +0x6f & 0x9f, byte 1 = [0x00679d1c], word +2 = car +0x6a & 0x7fff, bytes 4..6 =
// car +0x6e / +0x6c / +0x6d. 0x0043d262 then reports (CF) whether the car's segment (+0x1c) record has both
// low flag bits set; CF clear stores 0 into the marker byte [0x006f2f69], CF set stores 0xff (and edx = -1)
// unless it already is 0xff. 0x00510fad (esi = record, edi = write offset [0x0066d834], ecx = 8, edx = 0 or
// -1) writes the record to the replay file; when it leaves CF clear the offset advances by 8. The routine
// returns CF clear after the advance (clc) or CF set (stc) when the write failed.
// FUNCTION: GP4 0x00520948
void __cdecl FUN_00520948(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x0066d820);
    if (car == 0u) {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    const uint32_t record = GP4_GLOBAL(uint32_t, 0x007a8e30);
    GP4_FIELD(uint8_t, record, 0) =
        static_cast<uint8_t>((GP4_GLOBAL(uint8_t, 0x00679d1b) & 0x60u) | (GP4_FIELD(uint8_t, car, 0x6f) & 0x9fu));
    GP4_FIELD(uint8_t, record, 1) = GP4_GLOBAL(uint8_t, 0x00679d1c);
    GP4_FIELD(uint16_t, record, 2) = static_cast<uint16_t>(GP4_FIELD(uint16_t, car, 0x6a) & 0x7fffu);
    GP4_FIELD(uint8_t, record, 4) = GP4_FIELD(uint8_t, car, 0x6e);
    GP4_FIELD(uint8_t, record, 5) = GP4_FIELD(uint8_t, car, 0x6c);
    GP4_FIELD(uint8_t, record, 6) = GP4_FIELD(uint8_t, car, 0x6d);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.ebx = record;
    c.esi = GP4_GLOBAL(uint32_t, 0x007a8e30);
    c.edi = GP4_GLOBAL(uint32_t, 0x0066d834);
    c.ecx = 8u;
    c.edx = 0u;
    c.eax = GP4_GLOBAL(uint32_t, 0x0066d820);
    gp4::call_regs(0x0043d262, c);

    if ((c.eflags & gp4::flags::CF) == 0u) {
        GP4_GLOBAL(uint8_t, 0x006f2f69) = 0u;
    } else {
        const uint8_t marker = GP4_GLOBAL(uint8_t, 0x006f2f69);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(marker, 0xffu));
        if (marker != 0xffu) {
            GP4_GLOBAL(uint8_t, 0x006f2f69) = 0xffu;
            c.edx = 0xffffffffu;
        }
    }

    gp4::call_regs(0x00510fad, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        r->eflags = gp4::flags::merge(r->eflags, c.eflags);
        return;
    }
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0066d834);
    GP4_GLOBAL(uint32_t, 0x0066d834) = offset + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 8u) & ~gp4::flags::CF);
}
GP4_IMPL(0x00520948, FUN_00520948, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cache_0053832b+ring_008943a0+cars_0066fef0+event_005207a9+rec_005208f8")

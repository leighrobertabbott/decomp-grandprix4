#include <gp4/gp4.h>
#include <gp4/flags.h>

#pragma pack(push, 1)
struct Event_00520815 {
    uint8_t Flags00;    // bits 0x9f -> record +0x6f, bits 0x60 -> 0x00679d1b
    uint8_t Unknown01;  // -> 0x00679d1c
    uint16_t Word02;    // bit 15: marker entry; low 15 bits (sign-extended) -> record +0x6a
    uint8_t Unknown04;  // -> record +0x6e
    uint8_t Unknown05;  // -> record +0x6c
    uint8_t Unknown06;  // -> record +0x6d
};
#pragma pack(pop)

// Reads the next 8-byte event for the car record [0x0066d820] through command 0x4f of the
// dispatcher (0x00510fa3: ecx = 8 bytes, esi = the stream offset [0x0066d834], edi = the event
// buffer [0x007a8e30], edx = 0). stc (no read) when the record is null or the offset is not below
// [0x006975bc]. An event without bit 15 of word +2 is copied into the record (+0x6f = byte0 & 0x9f,
// [0x679d1b] = byte0 & 0x60, [0x679d1c] = byte1, +0x6a = word2 with bit 15 replaced by bit 14,
// +0x6e/+0x6c/+0x6d = bytes 4/5/6) and the offset advances by 8. A marker (bit 15 set) clears those
// record fields and sets both bytes to 0xff; the offset advances only when the dword
// (word4 << 16) | word0 equals the clock [0x0066d814]. clc on every read path (flags of the last
// `add`/`cmp`). esi/edi are restored; eax/ecx/edx keep the working values.
// FUNCTION: GP4 0x00520815
void __cdecl FUN_00520815(gp4::Regs* r) {
    const uint32_t record = GP4_GLOBAL(uint32_t, 0x0066d820);
    if (record == 0u) {
        r->eax = 0u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u) | gp4::flags::CF);
        return;
    }
    r->eax = record;
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0066d834);
    const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006975bc);
    const uint32_t cmp = gp4::flags::sub32(offset, limit);
    if (!(static_cast<int32_t>(offset) < static_cast<int32_t>(limit))) {
        r->eflags = gp4::flags::merge(r->eflags, cmp | gp4::flags::CF);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.eax = record;
    c.esi = offset;
    c.edi = GP4_GLOBAL(uint32_t, 0x007a8e30);
    c.ecx = 8u;
    c.edx = 0u;
    c.eflags = gp4::flags::merge(c.eflags, cmp);
    gp4::call_regs(0x00510fa3, c);

    uint8_t* rec = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0066d820)));
    const Event_00520815* ev =
        reinterpret_cast<const Event_00520815*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x007a8e30)));

    uint32_t eax = c.eax;
    uint32_t edx = c.edx;
    uint32_t flags;
    bool advance;
    if ((ev->Word02 & 0x8000u) != 0u) {
        rec[0x6f] = 0u;
        GP4_GLOBAL(uint8_t, 0x00679d1b) = 0xffu;
        GP4_GLOBAL(uint8_t, 0x00679d1c) = 0xffu;
        GP4_FIELD(uint16_t, rec, 0x6a) = 0u;
        rec[0x6e] = 0u;
        rec[0x6c] = 0u;
        rec[0x6d] = 0u;
        eax = (static_cast<uint32_t>(ev->Unknown04) << 16) | (static_cast<uint32_t>(ev->Unknown05) << 24) | ev->Flags00 |
              (static_cast<uint32_t>(ev->Unknown01) << 8);
        const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
        flags = gp4::flags::sub32(eax, clock);
        advance = eax == clock;
    } else {
        const uint8_t b0 = ev->Flags00;
        rec[0x6f] = static_cast<uint8_t>(b0 & 0x9fu);
        const uint8_t hi = static_cast<uint8_t>(b0 & 0x60u);
        GP4_GLOBAL(uint8_t, 0x00679d1b) = hi;
        edx = (edx & 0xffffff00u) | hi;
        GP4_GLOBAL(uint8_t, 0x00679d1c) = ev->Unknown01;
        const int16_t doubled = static_cast<int16_t>(static_cast<uint16_t>(ev->Word02 << 1));
        const uint16_t word = static_cast<uint16_t>(doubled >> 1);
        GP4_FIELD(uint16_t, rec, 0x6a) = word;
        rec[0x6e] = ev->Unknown04;
        rec[0x6c] = ev->Unknown05;
        const uint8_t last = ev->Unknown06;
        rec[0x6d] = last;
        eax = (eax & 0xffff0000u) | (static_cast<uint32_t>(word) & 0xff00u) | last;
        flags = 0u;
        advance = true;
    }
    if (advance) {
        const uint32_t pos = GP4_GLOBAL(uint32_t, 0x0066d834);
        flags = gp4::flags::add32(pos, 8u);
        GP4_GLOBAL(uint32_t, 0x0066d834) = pos + 8u;
    }

    r->eax = eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = edx;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags & ~gp4::flags::CF);
}
GP4_IMPL(0x00520815, FUN_00520815, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437888+fill_005204d0+event_005207a9+evbuf_00520735+cache_0053832b+slot_00520815")

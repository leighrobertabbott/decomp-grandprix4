#include <gp4/gp4.h>

// FUNCTION: GP4 0x00417f6b
// pushal ... popal routine. ebx = record whose position dwords +0x1e0 / +0x1e4 (coordinates << 11)
// are shifted down into 0x00623e30 / 0x00623e38; 0x004061ed then locates the segment (edi on entry, the
// result edx becomes edi), the raw position is saved to 0x00967988 / 0x0096798c, 0x0041e156
// (ebx = body, esi = offset pair) moves the point, and the moved point is saved to 0x00967990 /
// 0x00967994 before 0x0051f299 (a pushal/popal wrapper of 0x0050c980) runs. All registers are
// restored; the flags left are those 0x0051f299 leaves.
void __cdecl FUN_00417f6b(gp4::Regs* r) {
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));

    const int32_t x = GP4_FIELD(int32_t, body, 0x1e0) >> 11;
    GP4_GLOBAL(int32_t, 0x00623e30) = x;
    const int32_t z = GP4_FIELD(int32_t, body, 0x1e4) >> 11;
    GP4_GLOBAL(int32_t, 0x00623e38) = z;

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;
    s.eax = static_cast<uint32_t>(z);
    gp4::call_regs(0x004061ed, s);

    s.edi = s.edx;
    GP4_GLOBAL(uint32_t, 0x00967988) = GP4_FIELD(uint32_t, body, 0x1e0);
    s.eax = GP4_FIELD(uint32_t, body, 0x1e4);
    GP4_GLOBAL(uint32_t, 0x0096798c) = s.eax;
    gp4::call_regs(0x0041e156, s);

    s.eax = GP4_GLOBAL(uint32_t, 0x00623e30);
    GP4_GLOBAL(uint32_t, 0x00967990) = s.eax;
    s.eax = GP4_GLOBAL(uint32_t, 0x00623e38);
    GP4_GLOBAL(uint32_t, 0x00967994) = s.eax;
    gp4::call_regs(0x0051f299, s);

    r->eflags = (r->eflags & ~0x8d5u) | (s.eflags & 0x8d5u);
}
GP4_IMPL(0x00417f6b, FUN_00417f6b, "regs(ebx:ptr[0x1ec]:layout=rec_00417f6b, esi:ptr[8]:bytes, edi:rec[0x8943a0,0x160,0..11]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_00405a12")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Event handler keyed by edx. 2: pushal wrapper over the path scan 0x0052e86f; when the scan counted
// anything (ecx != 0) it cuts the path string at ebx (the last '\') and sets the byte 0x82fd5f to 0xff.
// 5 or 6: clears the dword 0x82fd88. Every other value does nothing. All registers come back unchanged
// (popal does not restore flags): the flags are those of `or ecx,ecx` on the 2 path and of the last
// compare (`cmp edx,5` for 5, else `cmp edx,6`) on the others.
// FUNCTION: GP4 0x0052f2df
void __cdecl FUN_0052f2df(gp4::Regs* r) {
    const uint32_t key = r->edx;
    uint32_t flags;
    if (key == 2u) {
        gp4::Regs c = *r;
        gp4::call_regs(0x0052e86f, c);
        flags = gp4::flags::logic32(c.ecx);
        if (c.ecx != 0u) {
            GP4_FIELD(uint8_t, c.ebx, 0) = 0u;
            GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
        }
    } else if (key == 5u) {
        flags = gp4::flags::sub32(key, 5u);
        GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
    } else {
        flags = gp4::flags::sub32(key, 6u);
        if (key == 6u) {
            GP4_GLOBAL(uint32_t, 0x0082fd88) = 0u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052f2df, FUN_0052f2df, "regs(edx:u32{=2,5,6,0,1,3,7}) -> (cf, pf, af, zf, sf, of); globals=layout=kind_0052f44d")

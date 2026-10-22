#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per call site so the compiler keeps the distinct calls.
static __declspec(noinline) void CallFrameCell(gp4::Regs& c) {
    gp4::call_regs(0x00527f81, c);
}
static __declspec(noinline) void CallLastCase(gp4::Regs& c) {
    gp4::call_regs(0x0054fd4d, c);
}
static __declspec(noinline) void CallStepHandler(gp4::Regs& c) {
    gp4::call_regs(0x0054fc6f, c);
}
static __declspec(noinline) void CallStepCase(gp4::Regs& c) {
    gp4::call_regs(0x0054fd4d, c);
}
static __declspec(noinline) void CallLastHandler(gp4::Regs& c) {
    gp4::call_regs(0x0054fc6f, c);
}
static __declspec(noinline) void CallPost(gp4::Regs& c) {
    gp4::call_regs(0x00549d12, c);
}

static inline uint32_t setLow8(uint32_t reg, uint32_t value) {
    return (reg & 0xffffff00u) | (value & 0xffu);
}

// dec cl: all arithmetic flags of cl - 1 except CF, which is kept.
static inline uint32_t dec8(uint32_t value, uint32_t inFlags) {
    return (gp4::flags::sub8(value, 1u) & ~gp4::flags::CF) | (inFlags & gp4::flags::CF);
}

// Rebuilds the screen after a state change: runs the frame-cell hand-off 0x527f81 (byte [0x81e4f8]
// is restored after it), sets [0x82fd5e] = 0xff, then walks the [0x83e55c] records of 0x28 bytes at
// 0x83e55e: the last record's kind byte goes to the case table 0x54fd4d (dl = 3); every earlier record
// goes, inside a pushal/popal, to the handler dispatcher 0x54fc6f (kind byte in al, handler operand
// dword +4 in esi, dl = 2) and the case table 0x54fd4d (dl = 2); the last record then goes to 0x54fc6f
// with dl = 3. 0x549d12 finishes (it posts the redraw message). With no records only the frame-cell
// hand-off and the final routine run. Registers and flags are those the final routine leaves.
// FUNCTION: GP4 0x0054fbfb
void __cdecl FUN_0054fbfb(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t saved = setLow8(c.eax, GP4_GLOBAL(uint8_t, 0x0081e4f8));
    c.eax = saved;
    CallFrameCell(c);
    c.eax = saved;
    GP4_GLOBAL(uint8_t, 0x0081e4f8) = static_cast<uint8_t>(saved);
    GP4_GLOBAL(uint8_t, 0x0082fd5e) = 0xffu;

    const uint32_t count = GP4_GLOBAL(uint8_t, 0x0083e55c);
    c.eax = count;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(count));
    if (count != 0u) {
        const uint32_t last = (count - 1u) * 0x28u + 0x0083e55eu;
        c.eax = last;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32((count - 1u) * 0x28u, 0x0083e55eu));
        c.edi = last;
        c.eax = setLow8(c.eax, GP4_FIELD(uint8_t, c.edi, 0));
        c.edx = setLow8(c.edx, 3u);
        CallLastCase(c);

        c.edi = 0x0083e55eu;
        uint32_t left = GP4_GLOBAL(uint8_t, 0x0083e55c);
        c.ecx = setLow8(c.ecx, left);
        c.eflags = gp4::flags::merge(c.eflags, dec8(left, c.eflags));
        left = (left - 1u) & 0xffu;
        c.ecx = setLow8(c.ecx, left);
        if (left != 0u) {
            do {
                gp4::Regs p = c;
                p.eax = setLow8(p.eax, GP4_FIELD(uint8_t, p.edi, 0));
                p.esi = GP4_FIELD(uint32_t, p.edi, 4);
                p.edx = setLow8(p.edx, 2u);
                CallStepHandler(p);
                p.edx = setLow8(p.edx, 2u);
                CallStepCase(p);
                c.eflags = p.eflags;
                c.edi += 0x28u;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.edi - 0x28u, 0x28u));
                c.eflags = gp4::flags::merge(c.eflags, dec8(left, c.eflags));
                left = (left - 1u) & 0xffu;
                c.ecx = setLow8(c.ecx, left);
            } while (left != 0u);
        }
        c.eax = setLow8(c.eax, GP4_FIELD(uint8_t, c.edi, 0));
        c.esi = GP4_FIELD(uint32_t, c.edi, 4);
        c.edx = setLow8(c.edx, 3u);
        CallLastHandler(c);
    }
    CallPost(c);
    *r = c;
}
GP4_IMPL(0x0054fbfb, FUN_0054fbfb, "regs(esi:ptr[0x10]:bytes, edi:ptr[8]:bytes) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70+case_0054fd4d+list_00549db9+tbl_0054fbfb; wall=3600")

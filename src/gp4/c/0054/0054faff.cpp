#include <gp4/gp4.h>
#include <gp4/flags.h>

// Pushes (al, esi) onto the 0x28-byte-stride record stack at 0x0083e55e (count byte [0x0083e55c]). With
// records present it first re-plays them: the top record's kind al goes through the dispatcher 0x0054fd4d
// with dl = 1, the cell hand-off 0x00527f81 runs with [0x0081e4f8] saved and restored around it, and every
// record (al = byte +0, esi = dword +4, dl = 2) is dispatched in turn inside a pushal/popal pair. Then the
// caller's al / esi are stored in the next record, the count is incremented and 0x0054fc6f (dl = 0) and
// 0x00549d12 run on the new record (edi = its address).
static inline uint32_t Dec8KeepCf(uint32_t value, uint32_t in_flags) {
    return (gp4::flags::sub8(value, 1u) & ~gp4::flags::CF) | (in_flags & gp4::flags::CF);
}

// FUNCTION: GP4 0x0054faff
void __cdecl FUN_0054faff(gp4::Regs* r) {
    gp4::Regs c = *r;
    const uint32_t saved_eax = r->eax;
    const uint32_t saved_esi = r->esi;

    const uint32_t count0 = GP4_GLOBAL(uint8_t, 0x0083e55c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub8(count0, 0u));
    if (count0 != 0u) {
        const uint32_t offset = (count0 - 1u) * 0x28u;
        const uint32_t top = offset + 0x0083e55eu;
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(offset, 0x0083e55eu));
        c.edi = top;
        c.eax = (top & 0xffffff00u) | GP4_GLOBAL(uint8_t, top);
        c.edx = (c.edx & 0xffffff00u) | 1u;
        gp4::call_regs(0x0054fd4d, c);

        c.eax = (c.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0081e4f8);
        const uint32_t pushed = c.eax;
        gp4::call_regs(0x00527f81, c);
        c.eax = pushed;
        GP4_GLOBAL(uint8_t, 0x0081e4f8) = static_cast<uint8_t>(c.eax);

        c.edi = 0x0083e55eu;
        c.ecx = (c.ecx & 0xffffff00u) | GP4_GLOBAL(uint8_t, 0x0083e55c);
        c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic8(c.ecx & 0xffu));
        if ((c.ecx & 0xffu) != 0u) {
            uint32_t cl;
            do {
                gp4::Regs w = c;
                w.eax = (w.eax & 0xffffff00u) | GP4_GLOBAL(uint8_t, c.edi);
                w.esi = GP4_GLOBAL(uint32_t, c.edi + 4u);
                w.edx = (w.edx & 0xffffff00u) | 2u;
                gp4::call_regs(0x0054fd4d, w);
                c.eflags = w.eflags;
                c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(c.edi, 0x28u));
                c.edi += 0x28u;
                cl = c.ecx & 0xffu;
                c.eflags = gp4::flags::merge(c.eflags, Dec8KeepCf(cl, c.eflags));
                cl = (cl - 1u) & 0xffu;
                c.ecx = (c.ecx & 0xffffff00u) | cl;
            } while (cl != 0u);
        }
    }

    const uint32_t count1 = GP4_GLOBAL(uint8_t, 0x0083e55c);
    const uint32_t slot = count1 * 0x28u + 0x0083e55eu;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(count1 * 0x28u, 0x0083e55eu));
    c.eax = slot;
    c.edi = slot;
    c.esi = saved_esi;
    c.eax = saved_eax;
    c.eflags = gp4::flags::merge(c.eflags, (gp4::flags::add8(count1, 1u) & ~gp4::flags::CF) | (c.eflags & gp4::flags::CF));
    GP4_GLOBAL(uint8_t, 0x0083e55c) = static_cast<uint8_t>(count1 + 1u);
    GP4_GLOBAL(uint8_t, c.edi) = static_cast<uint8_t>(c.eax);
    GP4_GLOBAL(uint32_t, c.edi + 4u) = c.esi;
    c.edx = c.edx & 0xffffff00u;
    gp4::call_regs(0x0054fc6f, c);
    gp4::call_regs(0x00549d12, c);
    *r = c;
}
GP4_IMPL(0x0054faff, FUN_0054faff, "regs(eax:u32{=4,6,7,0x0c,0xfe,0xff,0x134}, ebx:u32, ecx:u32, edx:u32, esi:u32{=0x0054fd8c,0x0054fd98,0x0054e232,0x00527d03,0x00531741}, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=case_0054fd4d+anim_0052822a+slots_005388ed+cells_00527eae+slot8_005388ba+surfcopy_00549d55+frame_0082fd70; wall=3600")

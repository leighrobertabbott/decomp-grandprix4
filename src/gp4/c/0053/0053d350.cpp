#include <gp4/gp4.h>
#include <gp4/flags.h>

// Front-end screen loop (hand-written, register-threaded): after the four set-up routines it
// optionally runs the 0x54ac8b/0x54a2c3 pre-loop until [0x82fd6e] = 0xff, then repeats
// { per-frame input path (0x54032a + 0x5016c0 [+ 0x53c4f0] when [0x7aafd3] != 0xff, else
// 0x54ac83|0x54a894 + 0x54a2c3 + 0x5016c0), 0x500080 under pushal/popal, 0x53d4f9, the
// screen's +0x24 handler when [0x82fd5f] = 0xff } until [0x7ab020] is 3/4/5 (selection 0)
// or [0x81e4fa] = 0xff. It then walks [0x82fd34] to entry [0x81e4f8] + 1; when that entry has
// bit 1 of +0x1e it runs 0x53de6f and, on carry, 0x53c8c8 (a nonzero result restarts the loop),
// then 0x53df39. Finally [0x82fd3c] = [0x82fd38] and 0x53ee17 runs with eax = [0x81e4f8].
// Registers flow between the callees exactly as in the original; the outputs are those
// of the tail call 0x53ee17.
// FUNCTION: GP4 0x0053d350
static inline void set_arith(gp4::Regs& c, uint32_t arith) { c.eflags = gp4::flags::merge(c.eflags, arith); }
static inline void set_lo8(uint32_t& reg, uint32_t v) { reg = (reg & 0xffffff00u) | (v & 0xffu); }
// Kept out of line: MSVC otherwise merges the two calls of the if/else below into one call with
// a selected address, hiding both callees from the shape audit.
static __declspec(noinline) void run_0054ac83(gp4::Regs& c) { gp4::call_regs(0x0054ac83, c); }

void __cdecl FUN_0053d350(gp4::Regs* r) {
    using namespace gp4::flags;
    gp4::Regs c = *r;

    gp4::call_regs(0x0053edec, c);
    gp4::call_regs(0x0054b056, c);
    gp4::call_regs(0x0053e0e2, c);
    gp4::call_regs(0x0053c374, c);

    set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafd3), 0xffu));
    if (GP4_GLOBAL(uint8_t, 0x007aafd3) == 0xffu) {
        set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafe0), 0xffu));
        if (GP4_GLOBAL(uint8_t, 0x007aafe0) != 0xffu) {
            set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafda), 0u));
            if (GP4_GLOBAL(uint8_t, 0x007aafda) == 0u) {
                GP4_GLOBAL(uint8_t, 0x0082fd6e) = 0;
                for (;;) {
                    set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafd3), 0xffu));
                    if (GP4_GLOBAL(uint8_t, 0x007aafd3) < 0xffu) break;
                    set_lo8(c.eax, 4u);
                    gp4::call_regs(0x0054ac8b, c);
                    gp4::call_regs(0x0054a2c3, c);
                    set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x0082fd6e), 0xffu));
                    if (GP4_GLOBAL(uint8_t, 0x0082fd6e) == 0xffu) {
                        gp4::call_regs(0x0054a7da, c);
                        break;
                    }
                }
            }
        }
    }

frame:
    gp4::cdecl_<0x005ae990, void>();
    set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafd3), 0xffu));
    if (GP4_GLOBAL(uint8_t, 0x007aafd3) != 0xffu) {
        gp4::call_regs(0x0054032a, c);
        c.eax = gp4::cdecl_<0x005016c0, uint32_t>();   // ebx..edi pushed/popped around it
        set_arith(c, logic32(c.eax));
        if (c.eax != 0u) {
            gp4::call_regs(0x0053c4f0, c);
        }
    } else {
        set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007aafd9), 0xffu));
        if (GP4_GLOBAL(uint8_t, 0x007aafd9) != 0xffu) {
            run_0054ac83(c);
        } else {
            gp4::call_regs(0x0054a894, c);
        }
        gp4::call_regs(0x0054a2c3, c);
        c.eax = gp4::cdecl_<0x005016c0, uint32_t>();
        set_arith(c, logic32(c.eax));
    }
    {
        // pushal / call 0x500080 / popal: registers kept, flags left by the callee.
        gp4::Regs t = c;
        gp4::call_regs(0x00500080, t);
        c.eflags = t.eflags;
    }
    gp4::call_regs(0x0053d4f9, c);

    set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x0082fd5f), 0xffu));
    if (GP4_GLOBAL(uint8_t, 0x0082fd5f) == 0xffu) {
        GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0;
        c.esi = GP4_GLOBAL(uint32_t, 0x0082fd34);
        const uint32_t handler = GP4_FIELD(uint32_t, (uint8_t*)(uintptr_t)c.esi, 0x24);
        set_arith(c, sub32(handler, 0u));
        if (handler != 0u) {
            gp4::call_regs(handler, c);
        }
    }

    {
        bool leave = false;
        set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007ab020), 3u));
        if (GP4_GLOBAL(uint8_t, 0x007ab020) == 3u) {
            leave = true;
        } else {
            set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007ab020), 4u));
            if (GP4_GLOBAL(uint8_t, 0x007ab020) == 4u) {
                leave = true;
            } else {
                set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x007ab020), 5u));
                leave = GP4_GLOBAL(uint8_t, 0x007ab020) == 5u;
            }
        }
        if (leave) {
            GP4_GLOBAL(uint8_t, 0x0081e4f8) = 0;
        } else {
            set_arith(c, sub8(GP4_GLOBAL(uint8_t, 0x0081e4fa), 0xffu));
            if (GP4_GLOBAL(uint8_t, 0x0081e4fa) != 0xffu) goto frame;
        }
    }

    // Walk the screen's entry list to entry [0x81e4f8] + 1 (cl counts down).
    c.ebx = GP4_GLOBAL(uint32_t, 0x0082fd34);
    set_lo8(c.ecx, GP4_GLOBAL(uint8_t, 0x0081e4f8));
    {
        const uint32_t cl = c.ecx & 0xffu;
        set_arith(c, (add(cl, 1u, 8) & ~CF) | (c.eflags & CF));
        set_lo8(c.ecx, cl + 1u);
    }
    for (;;) {
        c.ebx = *(const uint32_t*)(uintptr_t)c.ebx;
        set_arith(c, logic32(c.ebx));
        if (c.ebx == 0u) {
            // pushfd/pushal/pushal; esi = inline message 0x53d47d; call 0x54e232; popal/popal/popfd.
            // 0x54e232 is a bare `ret` (dead debug hook): esi is ignored and every register and
            // flag is restored, so the message pointer is not modelled.
            gp4::Regs t = c;
            gp4::call_regs(0x0054e232, t);
            goto finish;
        }
        const uint32_t cl = c.ecx & 0xffu;
        set_arith(c, (sub(cl, 1u, 8) & ~CF) | (c.eflags & CF));
        set_lo8(c.ecx, cl - 1u);
        if (((cl - 1u) & 0xffu) != 0u) continue;
        break;
    }
    {
        const uint32_t bits = GP4_FIELD(uint8_t, (uint8_t*)(uintptr_t)c.ebx, 0x1e) & 2u;
        set_arith(c, logic8(bits));
        if (bits == 0u) goto finish;
    }
    gp4::call_regs(0x0053de6f, c);
    if (c.eflags & CF) {
        c.ebx = GP4_GLOBAL(uint32_t, 0x0082fd34);
        c.eax = GP4_FIELD(uint32_t, (uint8_t*)(uintptr_t)c.ebx, 0x14);
        c.esi = 0x0082fda8u;
        GP4_FIELD(uint32_t, GP4_ARRAY(uint8_t, 0x0082fda8), 0xc) = c.eax;
        set_lo8(c.edx, GP4_GLOBAL(uint8_t, 0x0081e4f8));
        const uint32_t saved_edx = c.edx;               // push edx / pop edx
        gp4::call_regs(0x0053c8c8, c);
        c.edx = saved_edx;
        set_arith(c, logic32(c.eax));
        if (c.eax != 0u) goto frame;
        GP4_GLOBAL(uint8_t, 0x0081e4f8) = (uint8_t)saved_edx;
    }
    gp4::call_regs(0x0053df39, c);

finish:
    GP4_GLOBAL(uint32_t, 0x0082fd3c) = GP4_GLOBAL(uint32_t, 0x0082fd38);
    c.eax = GP4_GLOBAL(uint8_t, 0x0081e4f8);
    gp4::call_regs(0x0053ee17, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0053d350, FUN_0053d350, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")

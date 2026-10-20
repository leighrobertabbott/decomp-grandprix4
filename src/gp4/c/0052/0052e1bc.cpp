#include <gp4/gp4.h>
#include <gp4/flags.h>

// Kept out of line so the stores through the fixed node 0x845192 stay `[base + off]` instead of
// folding into absolute addresses the original never names.
static __declspec(noinline) void StoreDword(uint32_t base, uint32_t off, uint32_t value) {
    GP4_FIELD(uint32_t, base, off) = value;
}
static __declspec(noinline) void StoreByte(uint32_t base, uint32_t off, uint8_t value) {
    GP4_FIELD(uint8_t, base, off) = value;
}

static inline void SetLow8(uint32_t& reg, uint32_t value) { reg = (reg & 0xffffff00u) | (value & 0xffu); }
static inline void SetArith(gp4::Regs& s, uint32_t arith) { s.eflags = gp4::flags::merge(s.eflags, arith); }
static inline uint32_t NextNode(uint32_t node) { return GP4_FIELD(uint32_t, node, 0); }

// Register routine (esi = the file record: +0x34 caption, +0x4c type mask, +0x51 drive/mode byte,
// +0x58 selected types). Registers flow between the callees exactly as in the original. It saves
// the record pointer (0x826d11) and [0x7ad894] (0x826cce), allocates 0x754e pool bytes
// (0x5389b9; on failure only the clean-up runs), clears the header fields (0x52e976), sets up the
// menu at 0x845192 (caption, mode byte 0x826cd6, mask 0x826cd2 -> 0x503360): the items after the
// fifth link get one entry per set mask bit (text 0x257 + bit, 0x26a -> 0xb0c, +0x26 = the bit;
// at most six, none when fewer than two), the rest of the six are hidden, then two more items are
// hidden or shown by the +0x51 mode. It builds the name (0x52e42f), the drive count (0x52f429),
// resets the directory (0x5367a8), lists the files (0x52e589), reads the header (0x52e9a2) and
// runs the menu (0x53c8af). An accepted name (0x52e892 clear) is copied back into the record with
// its type mask; an empty or '.' name sets the cancel byte 0x81e4f8. The pool block is released
// (0x538a24) and CF = cancel (also stored at 0x824566). The final pushal block restores the
// directory and [0x7ad894] and keeps the mode byte when +0x51 is 0xff.
// FUNCTION: GP4 0x0052e1bc
void __cdecl FUN_0052e1bc(gp4::Regs* r) {
    using namespace gp4::flags;
    gp4::Regs s = *r;

    {
        gp4::Regs t = s;
        t.eax = 0x2f3u;
        gp4::call_regs(0x00527f80, t);
        s.eflags = t.eflags;
    }
    GP4_GLOBAL(uint32_t, 0x00826d11) = s.esi;
    s.eax = GP4_GLOBAL(uint32_t, 0x007ad894);
    GP4_GLOBAL(uint32_t, 0x00826cce) = s.eax;
    s.eax = 0x754eu;
    gp4::call_regs(0x005389b9, s);

    bool skipToEnd = false;   // jb 0x52e3f5 (name rejected)
    if ((s.eflags & CF) == 0u) {
        GP4_GLOBAL(uint32_t, 0x00826b6c) = s.eax;
        SetArith(s, add32(s.eax, 0x754eu));
        s.eax += 0x754eu;
        GP4_GLOBAL(uint32_t, 0x00826b70) = s.eax;
        gp4::call_regs(0x0052e976, s);

        const uint32_t record = s.esi;
        s.edi = 0x00845192u;
        s.eax = GP4_FIELD(uint32_t, record, 0x34);
        StoreDword(s.edi, 0x14u, s.eax);

        const uint8_t mode = GP4_FIELD(uint8_t, record, 0x51);
        SetLow8(s.eax, mode);
        SetArith(s, sub8(mode, 0xffu));
        if (mode != 0xffu) {
            SetLow8(s.eax, static_cast<uint8_t>(~mode));
        } else {
            SetLow8(s.eax, GP4_GLOBAL(uint8_t, 0x00826cd7));
        }
        GP4_GLOBAL(uint8_t, 0x00826cd6) = static_cast<uint8_t>(s.eax);

        s.edx = GP4_FIELD(uint32_t, record, 0x58) & GP4_FIELD(uint32_t, record, 0x4c);
        SetArith(s, logic32(s.edx));
        if (s.edx == 0u) {
            s.edx = GP4_FIELD(uint32_t, record, 0x4c);
        }
        GP4_GLOBAL(uint32_t, 0x00826cd2) = s.edx;

        {
            gp4::Regs t = s;
            const uint32_t args[1] = {GP4_GLOBAL(uint32_t, 0x00826cd2)};
            gp4::call_regs_stdcall(0x00503360, t, args);
            s.eflags = t.eflags;
        }

        const uint32_t mask = GP4_FIELD(uint32_t, record, 0x4c);
        s.edx = mask;
        for (int i = 0; i < 5; ++i) {
            s.edi = NextNode(s.edi);
        }
        s.ecx = 0u;

        // ror edx,1 / adc eax,0 over 32 bits: the population count; edx comes back unchanged.
        uint32_t count = 0u;
        uint32_t lastCarry = 0u;
        for (int bit = 0; bit < 32; ++bit) {
            const uint32_t carry = (mask >> bit) & 1u;
            lastCarry = add32(count, carry);
            count += carry;
        }
        (void)lastCarry;
        SetArith(s, sub32(count, 6u));
        if (count > 6u) {
            count = 6u;
        }
        s.eax = count;
        s.edx = mask;
        SetArith(s, sub32(count, 1u));
        if (static_cast<int32_t>(count) < 1) {
            gp4::Regs t = s;
            t.esi = 0x0052e272u;
            gp4::call_regs(0x0054e232, t);
        }
        SetArith(s, sub32(count, 2u));
        if (static_cast<int32_t>(count) < 2) {
            count = 0u;
        }
        s.eax = count;

        if (count != 0u) {
            uint32_t bit = 1u;
            uint32_t text = 0x257u;
            uint32_t node = s.edi;
            for (uint32_t left = count; left != 0u; --left) {
                while ((bit & mask) == 0u) {
                    bit <<= 1;
                    text += 1u;
                }
                GP4_FIELD(uint8_t, node, 0x1f) &= 0xefu;
                if (text == 0x26au) {
                    GP4_FIELD(uint32_t, node, 0x12) = 0xb0cu;
                } else {
                    GP4_FIELD(uint32_t, node, 0x12) = text;
                }
                GP4_FIELD(uint32_t, node, 0x26) = bit;
                bit <<= 1;
                text += 1u;
                node = NextNode(node);
            }
            s.ebx = bit;
            s.ebp = text;
            s.edi = node;
            s.ecx = 0u;
        }

        SetArith(s, sub32(6u, count));
        s.ecx = 6u - count;
        if (s.ecx != 0u) {
            uint32_t node = s.edi;
            for (uint32_t left = s.ecx; left != 0u; --left) {
                const uint8_t flags1f = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) | 0x10u);
                GP4_FIELD(uint8_t, node, 0x1f) = flags1f;
                SetArith(s, logic8(flags1f));
                GP4_FIELD(uint32_t, node, 0x12) = 0u;
                GP4_FIELD(uint32_t, node, 0x26) = 0u;
                node = NextNode(node);
            }
            s.edi = node;
            s.ecx = 0u;
        }

        const uint8_t mode2 = GP4_FIELD(uint8_t, record, 0x51);
        SetArith(s, sub8(mode2, 0xffu));
        if (mode2 != 0xffu) {
            uint32_t node = s.edi;
            for (int k = 0; k < 2; ++k) {
                const uint8_t flags1f = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) | 0x10u);
                GP4_FIELD(uint8_t, node, 0x1f) = flags1f;
                SetArith(s, logic8(flags1f));
                node = NextNode(node);
            }
            s.edi = node;
            s.ecx = 0u;
        } else {
            uint32_t node = s.edi;
            uint8_t flags1f = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) & 0xefu);
            GP4_FIELD(uint8_t, node, 0x1f) = flags1f;
            node = NextNode(node);
            flags1f = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) & 0xcfu);
            GP4_FIELD(uint8_t, node, 0x1f) = flags1f;
            const uint8_t shown = GP4_GLOBAL(uint8_t, 0x00826cd6);
            SetArith(s, sub8(shown, 0xffu));
            if (shown != 0xffu) {
                flags1f = static_cast<uint8_t>(GP4_FIELD(uint8_t, node, 0x1f) | 0x20u);
                GP4_FIELD(uint8_t, node, 0x1f) = flags1f;
                SetArith(s, logic8(flags1f));
            }
            s.edi = node;
        }

        gp4::call_regs(0x0052e42f, s);
        SetLow8(s.eax, GP4_GLOBAL(uint8_t, 0x0081e7f2));
        gp4::call_regs(0x0052f429, s);
        GP4_GLOBAL(uint32_t, 0x00824712) = s.eax;
        gp4::call_regs(0x005367a8, s);

        s.esi = 0x00845192u;
        SetLow8(s.eax, 2u);
        const uint8_t drives = GP4_GLOBAL(uint8_t, 0x00826d45);
        SetArith(s, sub8(drives, 0xffu));
        if (drives != 0xffu) {
            SetLow8(s.eax, 1u);
        }
        StoreByte(s.esi, 0xdu, static_cast<uint8_t>(s.eax));
        gp4::call_regs(0x0052e589, s);
        gp4::call_regs(0x0052e9a2, s);
        s.esi = 0x00845192u;
        gp4::call_regs(0x0053c8af, s);

        const uint8_t first = GP4_GLOBAL(uint8_t, 0x0082471a);
        bool cancel = false;
        SetArith(s, sub8(first, 0u));
        if (first == 0u) {
            cancel = true;
        } else {
            SetArith(s, sub8(first, 0x2eu));
            if (first == 0x2eu) {
                cancel = true;
            }
        }
        if (cancel) {
            GP4_GLOBAL(uint8_t, 0x0081e4f8) = 1u;
        } else {
            const uint8_t quit = GP4_GLOBAL(uint8_t, 0x0081e4f8);
            SetArith(s, sub8(quit, 0u));
            if (quit == 0u) {
                gp4::call_regs(0x0052e892, s);
                if ((s.eflags & CF) != 0u) {
                    skipToEnd = true;
                } else {
                    s.esi = 0x0082471au;
                    s.edi = GP4_GLOBAL(uint32_t, 0x00826d11);
                    SetArith(s, add32(s.edi, 0u));
                    s.ecx = 0x30u;
                    for (uint32_t i = 0u; i != 0x30u; ++i) {
                        GP4_FIELD(uint8_t, s.edi, i) = GP4_FIELD(uint8_t, s.esi, i);
                    }
                    s.esi += 0x30u;
                    s.edi += 0x30u;
                    s.ecx = 0u;
                    s.esi = GP4_GLOBAL(uint32_t, 0x00826d11);
                    s.eax = GP4_GLOBAL(uint32_t, 0x00826cd2);
                    GP4_FIELD(uint32_t, s.esi, 0x58) = s.eax;
                }
            }
        }
    }

    if (!skipToEnd) {
        const uint32_t block = GP4_GLOBAL(uint32_t, 0x00826b6c);
        SetArith(s, sub32(block, 0u));
        if (block != 0u) {
            s.eax = 0x754eu;
            gp4::call_regs(0x00538a24, s);
            GP4_GLOBAL(uint32_t, 0x00826b6c) = 0u;
        }
        const uint8_t quit = GP4_GLOBAL(uint8_t, 0x0081e4f8);
        s.eax = quit;
        if (quit != 0u) {
            GP4_GLOBAL(uint8_t, 0x00824566) = 1u;
            SetArith(s, sub8(quit, 0u) | CF);
        } else {
            GP4_GLOBAL(uint8_t, 0x00824566) = 0u;
            SetArith(s, sub8(quit, 0u) & ~CF);
        }
    }

    {
        gp4::Regs t = s;
        const uint8_t hooked = GP4_GLOBAL(uint8_t, 0x007ab184);
        SetArith(t, sub8(hooked, 0u));
        if (hooked != 0u) {
            gp4::call_regs(0x005ae990, t);
        }
        gp4::call_regs(0x005367a8, t);
        GP4_GLOBAL(uint32_t, 0x007ad894) = GP4_GLOBAL(uint32_t, 0x00826cce);
        const uint32_t current = GP4_GLOBAL(uint32_t, 0x00826d11);
        if (GP4_FIELD(uint8_t, current, 0x51) == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x00826cd7) = GP4_GLOBAL(uint8_t, 0x00826cd6);
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = s.eflags;
}
GP4_IMPL(0x0052e1bc, FUN_0052e1bc, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x60]:layout=recbuf_0052e42f, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=pool_005389b9+cell_00845192+rec_0052e42f+lower_00833e18+name_0082471a; emu=skip:win32 SetCurrentDirectoryA")

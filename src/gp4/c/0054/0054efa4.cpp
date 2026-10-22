#include <gp4/gp4.h>
#include <gp4/flags.h>

// One helper per register-routine target so every call site stays a real call.
static __declspec(noinline) void CallAlloc(gp4::Regs& c) {
    gp4::call_regs(0x005389b9, c);
}
static __declspec(noinline) void CallStoreWord(gp4::Regs& c) {
    gp4::call_regs(0x00531af4, c);
}
static __declspec(noinline) void CallSaveRect(gp4::Regs& c) {
    gp4::call_regs(0x0054a15b, c);
}
static __declspec(noinline) void CallListInsert(gp4::Regs& c) {
    gp4::call_regs(0x00549db9, c);
}
static __declspec(noinline) void CallColour(gp4::Regs& c) {
    gp4::call_regs(0x0054cc96, c);
}
static __declspec(noinline) void CallStub(gp4::Regs& c) {
    gp4::call_regs(0x0054cb71, c);
}
static __declspec(noinline) void CallCursor(gp4::Regs& c) {
    gp4::call_regs(0x0054cb7e, c);
}
static __declspec(noinline) void CallText(gp4::Regs& c) {
    gp4::call_regs(0x0054d1a2, c);
}

// pushal wrapper. Runs only when the mode byte [0x838e3e] is 2 and the hovered node [0x838e48] is
// set. Without a saved-pixel buffer [0x838e88] it allocates [0x838e60] * [0x838e64] * 2 bytes
// (size kept in [0x838e8c]) with 0x5389b9 and gives up when that fails (CF). It then copies the
// rectangle [0x838e50..5c] (x0, y0, x1, y1) into [0x838e78..84], saves the surface pixels under
// it into the buffer (0x54a15b), queues it in the sorted pool list (0x549db9) and marks it pending
// ([0x838e90] = 0xff). Finally it sets the colour words (0x531af4 / 0x54cc96 with [0x838e68] /
// [0x838e6c]), the text cursor (x0 + 4, y0 + 4) with 0x54cb7e and draws the node's script string
// (+0xc) with 0x54d1a2. popal restores every register; the flags are those of the last writer
// (the mode compare, `or esi, esi`, the failed allocation or `cmp [0x838e70], 1`).
// FUNCTION: GP4 0x0054efa4
void __cdecl FUN_0054efa4(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x00838e3e);
    if (mode != 2u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 2u));
        return;
    }
    const uint32_t node = GP4_GLOBAL(uint32_t, 0x00838e48);
    if (node == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
        return;
    }

    gp4::Regs c = *r;
    c.esi = node;
    if (GP4_GLOBAL(uint32_t, 0x00838e88) == 0u) {
        const uint32_t size = (GP4_GLOBAL(uint32_t, 0x00838e60) * GP4_GLOBAL(uint32_t, 0x00838e64)) << 1;
        GP4_GLOBAL(uint32_t, 0x00838e8c) = size;
        c.eax = size;
        CallAlloc(c);
        if ((c.eflags & gp4::flags::CF) != 0u) {
            r->eflags = gp4::flags::merge(r->eflags, c.eflags);
            return;
        }
        GP4_GLOBAL(uint32_t, 0x00838e88) = c.eax;
    }

    c.eax = 0u;
    CallStoreWord(c);

    const uint32_t x0 = GP4_GLOBAL(uint32_t, 0x00838e50);
    const uint32_t y0 = GP4_GLOBAL(uint32_t, 0x00838e54);
    const uint32_t x1 = GP4_GLOBAL(uint32_t, 0x00838e58);
    const uint32_t y1 = GP4_GLOBAL(uint32_t, 0x00838e5c);
    GP4_GLOBAL(uint32_t, 0x00838e78) = x0;
    GP4_GLOBAL(uint32_t, 0x00838e7c) = y0;
    GP4_GLOBAL(uint32_t, 0x00838e80) = x1;
    GP4_GLOBAL(uint32_t, 0x00838e84) = y1;
    c.ebx = x0;
    c.eax = y0;
    c.ebp = x1;
    c.ecx = y1;
    c.edi = GP4_GLOBAL(uint32_t, 0x00838e88);
    CallSaveRect(c);
    CallListInsert(c);
    GP4_GLOBAL(uint8_t, 0x00838e90) = 0xffu;

    {
        // pushal; add ebx, 8; add eax, 4; call 0x531b2f; mov eax, [0x838e68]; call 0x531af4; popal
        gp4::Regs s = c;
        s.ebx += 8u;
        s.eflags = gp4::flags::merge(s.eflags, gp4::flags::add32(s.eax, 4u));
        s.eax += 4u;
        gp4::cdecl_<0x00531b2f, void>();
        s.eax = GP4_GLOBAL(uint32_t, 0x00838e68);
        CallStoreWord(s);
        c.eflags = s.eflags;
    }

    c.ebp -= 8u;
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(c.ecx, 4u));
    c.ecx -= 4u;
    gp4::cdecl_<0x00531b2e, void>();
    {
        const uint32_t saved = c.eax;
        c.eax = GP4_GLOBAL(uint32_t, 0x00838e6c);
        CallStoreWord(c);
        c.eax = saved;
    }
    gp4::cdecl_<0x00531b2b, void>();
    c.eax = GP4_GLOBAL(uint32_t, 0x00838e6c);
    CallColour(c);
    c.eax = GP4_GLOBAL(uint32_t, 0x00838e70);
    CallStub(c);

    c.ebx = GP4_GLOBAL(uint32_t, 0x00838e50) + 4u;
    const uint32_t top = GP4_GLOBAL(uint32_t, 0x00838e54);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::add32(top, 4u));
    c.eax = top + 4u;
    CallCursor(c);

    c.esi = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00838e48) + 0xcu));
    CallText(c);

    const uint32_t lines = GP4_GLOBAL(uint32_t, 0x00838e70);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(lines, 1u));
    if (lines != 1u) {
        c.eax = 1u;
        CallStub(c);
    }
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054efa4, FUN_0054efa4, "regs(edx:u32) -> (cf, pf, af, zf, sf, of); globals=layout=tip_0054ee4f+pend_0054ef46+pool_005389b9+list_00549db9+script_all_00835846+textout_0054d211+box_0054efa4")

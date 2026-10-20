#include <gp4/gp4.h>
#include <gp4/flags.h>

// Snapshots a 0x300-stride indexed block into the buffer at edi: writes the four header dwords
// [0x83082c], [0x830830] (byte count), [0x830834] (block index), [0x830838] at edi+0..0xc, then
// copies the count bytes found at (base + index * 0x300) behind them, where base comes from
// 0x005418cd (eax = 0 selects its default pointer). Direction flag clear, as the game runs.
// Leaves eax = [0x830838], ecx = 0, esi = source end, edi = destination end, and the flags of
// `add edi, 0x10`.

// FUNCTION: GP4 0x00541914
void __cdecl FUN_00541914(gp4::Regs* r) {
    uint8_t* dest = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));

    const uint32_t head = GP4_GLOBAL(uint32_t, 0x0083082c);
    *reinterpret_cast<uint32_t*>(dest) = head;

    gp4::Regs c = *r;
    c.eax = head;
    gp4::call_regs(0x005418cd, c);
    uint32_t source = c.eax;

    const uint32_t count = GP4_GLOBAL(uint32_t, 0x00830830);
    *reinterpret_cast<uint32_t*>(dest + 4) = count;
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00830834);
    *reinterpret_cast<uint32_t*>(dest + 8) = index;
    source += index * 0x300u;
    const uint32_t tail = GP4_GLOBAL(uint32_t, 0x00830838);
    *reinterpret_cast<uint32_t*>(dest + 0xc) = tail;

    const uint32_t flags = gp4::flags::add32(r->edi, 0x10u);
    uint8_t* out = dest + 0x10;
    const uint8_t* in = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(source));
    for (uint32_t n = count; n != 0; --n) {
        *out++ = *in++;
    }

    r->eax = tail;
    r->ecx = 0;
    r->esi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(in));
    r->edi = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out));
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00541914, FUN_00541914, "regs(edi:ptr[0x400]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00541914")

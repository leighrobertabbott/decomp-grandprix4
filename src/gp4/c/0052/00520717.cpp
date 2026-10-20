#include <gp4/gp4.h>
#include <gp4/flags.h>

// Event-stream step for the selected car record: when the mode byte [0x007c2e72] is 0xff it runs
// the keyframe reader 0x00520815. Otherwise it advances the stream offset [0x0066d834] by 8, runs
// the ring-buffer reader 0x00520735 and clears CF (clc) while keeping the other flags that routine
// left. In the 0xff case registers and flags are those of 0x00520815.
// FUNCTION: GP4 0x00520717
void __cdecl FUN_00520717(gp4::Regs* r) {
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2e72);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(mode, 0xffu));
    if (mode == 0xffu) {
        gp4::call_regs(0x00520815, *r);
        return;
    }
    const uint32_t offset = GP4_GLOBAL(uint32_t, 0x0066d834);
    GP4_GLOBAL(uint32_t, 0x0066d834) = offset + 8u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 8u));
    gp4::call_regs(0x00520735, *r);
    r->eflags &= ~static_cast<uint32_t>(gp4::flags::CF);
}
GP4_IMPL(0x00520717, FUN_00520717, "regs(ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_00437888+fill_005204d0+event_005207a9+evbuf_00520735+cache_0053832b+slot_00520815")

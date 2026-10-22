#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper around one outgoing-packet update: the stream cursor [0x0062d19c] is pointed at the write
// cursor [0x0062d1a0] (both are restored at the end), then 0x0051f0c7, 0x00445483, 0x00444c3c, 0x004453d1 and the
// receive dispatcher 0x004452a0 run. When the dispatcher leaves CF set (link down / nothing received)
// [0x008914e0] is cleared and, if the link byte [0x007aafd8] is 0xff, set to 0x20 with bit 4 or-ed into
// [0x007c2ead]. popal restores the registers; the flags are those of the last flag writer on the path taken.
// FUNCTION: GP4 0x0044535c
void __cdecl FUN_0044535c(gp4::Regs* r) {
    const uint32_t savedWrite = GP4_GLOBAL(uint32_t, 0x0062d1a0);
    const uint32_t savedRead = GP4_GLOBAL(uint32_t, 0x0062d19c);
    GP4_GLOBAL(uint32_t, 0x0062d19c) = GP4_GLOBAL(uint32_t, 0x0062d1a0);

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0051f0c7, c);
    gp4::call_regs(0x00445483, c);
    gp4::call_regs(0x00444c3c, c);
    gp4::call_regs(0x004453d1, c);
    gp4::call_regs(0x004452a0, c);

    uint32_t flags = c.eflags;
    if ((c.eflags & gp4::flags::CF) != 0u) {
        GP4_GLOBAL(uint8_t, 0x008914e0) = 0;
        const uint8_t link = GP4_GLOBAL(uint8_t, 0x007aafd8);
        flags = gp4::flags::sub8(link, 0xffu);
        if (link == 0xffu) {
            GP4_GLOBAL(uint8_t, 0x008914e0) |= 0x20u;
            const uint8_t mode = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x007c2ead) | 0x10u);
            GP4_GLOBAL(uint8_t, 0x007c2ead) = mode;
            flags = gp4::flags::logic8(mode);
        }
    }

    GP4_GLOBAL(uint32_t, 0x0062d19c) = savedRead;
    GP4_GLOBAL(uint32_t, 0x0062d1a0) = savedWrite;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
// Spec: recv_00445423 (last) adds the link-session byte [0x7aafd3] = 0xff cases in which 0x00445423 enters the
// DirectPlay receive (external): the clc return of 0x004452a0 and its CF=1 return with the link still up
// (length 0) exist only there.
GP4_IMPL(0x0044535c, FUN_0044535c, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+stream_0062d1a0+state_0044535c+recv_00445423")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the key al up in the six slot bytes at 0x831cfd (`repne scasb`, direction flag clear). Found at
// index k: eax = the dword at 0x831d1c + k * 4 and the flags are those of `add eax, 0x831d1c`. Not found:
// eax = 0x6318 and the flags are those of the last `cmp al, [0x831d02]`. ecx and edi are saved and restored.
// FUNCTION: GP4 0x0054972f
void __cdecl FUN_0054972f(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    const uint8_t* slots = GP4_ARRAY(const uint8_t, 0x00831cfd);

    uint32_t index = 0;
    while (index != 6u && slots[index] != key) {
        ++index;
    }

    if (index == 6u) {
        r->eax = 0x6318u;
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(key, slots[5]));
        return;
    }

    const uint32_t offset = index << 2;
    r->eax = *reinterpret_cast<const uint32_t*>(offset + 0x00831d1cu);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, 0x00831d1cu));
}
GP4_IMPL(0x0054972f, FUN_0054972f, "regs(eax:u32{=1,2,3,4,5,0xff}) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cur_00549849+tabs_00831d1c")

#include <gp4/gp4.h>
#include <gp4/flags.h>

// Expands a 2-bits-per-pixel mask into a block of bytes: [0x00833f7c] rows of [0x00833f78] pixels.
// esi is the mask stream (16 pixels per dword, the pixel's bit 1 is tested with `shr ebx, 2`), edi
// the destination; a pixel whose bit is set stores the colour byte [0x00833cd5], every pixel
// advances edi, and each row ends by moving edi on to the next row (edi - width + [0x00822244]).
// ebx keeps the unconsumed mask bits, dh the pixels left in the current mask dword (it is 0x10
// after a reload), ecx is 0 on exit, al is the colour; the flags are those of the last `dec ecx`
// (CF from the last `add edi, [0x822244]`) or of `or ecx, ecx` when there are no rows. Then the
// empty routine 0x0054d6f0 is called.
// FUNCTION: GP4 0x0054d4ee
void __cdecl FUN_0054d4ee(gp4::Regs* r) {
    uint32_t esi = r->esi;
    uint32_t edi = r->edi;
    uint32_t ebx = r->ebx;
    uint32_t flags = gp4::flags::logic32(0u);  // `or ecx, ecx` with ecx == 0

    const uint32_t rows = GP4_GLOBAL(uint32_t, 0x00833f7c);
    if (rows != 0u) {
        ebx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
        esi += 4u;
        uint32_t remaining = 0x10u;
        const uint8_t colour = GP4_GLOBAL(uint8_t, 0x00833cd5);
        r->eax = (r->eax & 0xffffff00u) | colour;

        uint32_t rowsLeft = rows;
        do {
            uint32_t pixels = GP4_GLOBAL(uint32_t, 0x00833f78);
            do {
                const bool set = ((ebx >> 1) & 1u) != 0u;
                ebx >>= 2;
                if (set) {
                    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(edi)) = colour;
                }
                edi += 1u;
                remaining -= 1u;
                if (remaining == 0u) {
                    ebx = *reinterpret_cast<const uint32_t*>(static_cast<uintptr_t>(esi));
                    esi += 4u;
                    remaining = 0x10u;
                }
                pixels -= 1u;
            } while (pixels != 0u);
            edi -= GP4_GLOBAL(uint32_t, 0x00833f78);
            const uint32_t pitch = GP4_GLOBAL(uint32_t, 0x00822244);
            const uint32_t addFlags = gp4::flags::add32(edi, pitch);
            edi += pitch;
            rowsLeft -= 1u;
            flags = gp4::flags::dec32(1u, addFlags);  // dec ecx: CF kept from the add
        } while (rowsLeft != 0u);
        r->edx = (r->edx & 0xffff00ffu) | (remaining << 8);
    }

    gp4::Regs c = {};
    gp4::call_regs(0x0054d6f0, c);

    r->ebx = ebx;
    r->ecx = 0u;
    r->esi = esi;
    r->edi = edi;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054d4ee, FUN_0054d4ee, "regs(eax:u32, edx:u32, esi:ptr[0x100]:bytes, edi:ptr[0x400]:bytes) -> (eax, ebx, ecx, edx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=blit_0054d4ee")

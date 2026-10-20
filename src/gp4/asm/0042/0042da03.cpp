#include <gp4/gp4.h>

struct Unknown_0042da03_Body {
    uint8_t Unknown00[0xfe];
    uint8_t UnknownFE;
};

// Picks the steering-style byte for the record in ebx: bit 0x40 of [ebx+0xfe] set forces a saturated
// value (0xff when bit 0x10 is clear, 0 when set); otherwise FUN_0042ac9c computes it into [0x00604154].
// The byte ends up in dh and in both [0x00604153] and [0x00604154].
// FUNCTION: GP4 0x0042da03
void __cdecl FUN_0042da03(gp4::Regs* r) {
    const Unknown_0042da03_Body* body = reinterpret_cast<const Unknown_0042da03_Body*>(static_cast<uintptr_t>(r->ebx));
    const uint8_t flagsByte = body->UnknownFE;
    uint8_t dh;

    if ((flagsByte & 0x40u) == 0u) {
        gp4::Regs call = *r;
        gp4::call_regs(0x0042ac9c, call);
        r->eax = call.eax;
        r->ebx = call.ebx;
        r->ecx = call.ecx;
        r->edx = call.edx;
        r->esi = call.esi;
        r->edi = call.edi;
        r->ebp = call.ebp;
        r->eflags = call.eflags;
        dh = GP4_GLOBAL(uint8_t, 0x00604154);
    } else {
        // flags of `test byte ptr [ebx + 0xfe], 0x10`: CF = OF = AF = SF = 0, ZF/PF from the masked byte
        const uint32_t masked = flagsByte & 0x10u;
        const uint32_t flags = masked == 0u ? 0x44u : 0u;
        r->eflags = (r->eflags & ~0x8d5u) | flags;
        dh = masked == 0u ? 0xffu : 0u;
    }

    r->edx = (r->edx & 0xffff00ffu) | (static_cast<uint32_t>(dh) << 8);
    GP4_GLOBAL(uint8_t, 0x00604153) = dh;
    GP4_GLOBAL(uint8_t, 0x00604154) = dh;
}
GP4_IMPL(0x0042da03, FUN_0042da03, "regs(edx:u32, ebx:ptr[0x128]:bytes, esi:ptr[0x98]:bytes) -> (edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")

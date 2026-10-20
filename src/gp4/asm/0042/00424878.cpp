#include <gp4/gp4.h>

// FUNCTION: GP4 0x00424878
// Refreshes the object record in ebx: stores eax at +0x4c, then runs the update steps in the
// original order. 0x444837 (pushal wrapper), 0x42453d and 0x424553 (with esi = the record), the
// density scaling 0x420e1d for max([ebx+0x48], [0x608250]) - its result << 5 goes to +0x5c - and
// finally the position/orientation update 0x424a21. Every step keeps the registers it does not
// produce; the registers and flags that leave are those of the last call.
void __cdecl FUN_00424878(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    GP4_FIELD(uint32_t, record, 0x4c) = r->eax;

    gp4::Regs state = *r;
    state.st_in = 0;
    state.st_out = 0;
    gp4::call_regs(0x00444837, state);

    // push esi / mov esi, ebx ... pop esi
    gp4::Regs withRecord = state;
    withRecord.esi = r->ebx;
    gp4::call_regs(0x0042453d, withRecord);
    gp4::call_regs(0x00424553, withRecord);

    int32_t amount = GP4_FIELD(int32_t, record, 0x48);
    if (amount < GP4_GLOBAL(int32_t, 0x00608250)) {
        amount = GP4_GLOBAL(int32_t, 0x00608250);
    }
    state.eax = static_cast<uint32_t>(amount);
    gp4::call_regs(0x00420e1d, state);

    state.eax <<= 5;
    GP4_FIELD(uint32_t, record, 0x5c) = state.eax;
    gp4::call_regs(0x00424a21, state);

    r->eax = state.eax;
    r->ebx = state.ebx;
    r->ecx = state.ecx;
    r->edx = state.edx;
    r->esi = state.esi;
    r->edi = state.edi;
    r->ebp = state.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (state.eflags & 0x8d5u);
}
GP4_IMPL(0x00424878, FUN_00424878, "regs(eax:u32[1..0x7fffff], ebx:ptr[0x80]:layout=rec_0042453d+rec_00424878, esi:ptr[0x41c]:layout=car_0041e500+rec_00420e1d+car_00424878) -> (eax, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042458b+state_00420e1d+blk_00424553+state_0066d8c0+state_0041bce9+state_00444517+state_00424a21")

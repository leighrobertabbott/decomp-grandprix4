#include <gp4/gp4.h>

// Keeps the compiler from folding state-relative offsets into absolute globals.
static __declspec(noinline) uint8_t* StateBase() { return GP4_ARRAY(uint8_t, 0x006380d8); }

// Sum of the dword at `off` in the third and fourth 0x300-byte records of 0x006380d8
// (base + off and base + off + 0x300).
static inline uint32_t pairedField(uint8_t* state, uint32_t off) {
    return GP4_FIELD(uint32_t, state, off) + GP4_FIELD(uint32_t, state, off + 0x300u);
}

// The three body updates the original repeats after every change of esi+0x3c.
static inline void updateBodies(gp4::Regs& s) {
    gp4::call_regs(0x004168b6, s);
    gp4::call_regs(0x00416b52, s);
    gp4::call_regs(0x0041746d, s);
}

// FUNCTION: GP4 0x004193fa
void __cdecl FUN_004193fa(gp4::Regs* r) {
    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint8_t* const car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(s.esi));
    uint8_t* const state = StateBase();
    const uint32_t stateAddr = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(state));

    gp4::call_regs(0x00413397, s);
    gp4::call_regs(0x004132e5, s);

    s.edx = GP4_GLOBAL(uint32_t, 0x00603688);
    GP4_GLOBAL(uint32_t, 0x006036bc) = s.edx;
    s.ebx = stateAddr;
    s.eax = pairedField(state, 0x630) << 15;
    GP4_FIELD(uint32_t, car, 0x3c) += s.eax;
    const uint32_t saved = GP4_FIELD(uint32_t, car, 0x3c);

    // step = [0x6034b0] >> 8 with its magnitude raised to at least [0x603550] (signed compares)
    int32_t step = GP4_GLOBAL(int32_t, 0x006034b0) >> 8;
    const int32_t minStep = GP4_GLOBAL(int32_t, 0x00603550);
    if (step < 0) {
        int32_t magnitude = static_cast<int32_t>(0u - static_cast<uint32_t>(step));
        if (magnitude < minStep) magnitude = minStep;
        step = static_cast<int32_t>(0u - static_cast<uint32_t>(magnitude));
    } else if (step < minStep) {
        step = minStep;
    }
    GP4_GLOBAL(int32_t, 0x006036b4) = step;
    s.eax = static_cast<uint32_t>(step) << 16;
    GP4_FIELD(uint32_t, car, 0x3c) += s.eax;
    updateBodies(s);

    s.ebx = stateAddr;
    s.eax = pairedField(state, 0x634);
    GP4_GLOBAL(uint32_t, 0x006036ac) = s.eax;
    GP4_FIELD(uint32_t, car, 0x3c) = saved;
    updateBodies(s);

    s.ecx = GP4_GLOBAL(uint32_t, 0x006036ac);
    s.ebx = stateAddr;
    s.eax = pairedField(state, 0x634);
    GP4_GLOBAL(uint32_t, 0x006036ac) = s.eax;
    s.ecx -= s.eax;
    GP4_GLOBAL(uint32_t, 0x006036b0) = s.ecx;
    s.eax = GP4_GLOBAL(uint32_t, 0x00603684);
    GP4_GLOBAL(uint32_t, 0x0060368c) = s.eax;

    for (;;) {
        gp4::call_regs(0x004192a8, s);
        if ((s.eflags & gp4::CF) != 0u) break;

        GP4_GLOBAL(uint32_t, 0x006036b4) = s.eax;
        s.eax <<= 16;
        GP4_FIELD(uint32_t, car, 0x3c) += s.eax;
        updateBodies(s);

        s.ebx = stateAddr;
        s.ecx = pairedField(state, 0x634);
        s.eax = GP4_GLOBAL(uint32_t, 0x006036ac);
        GP4_GLOBAL(uint32_t, 0x006036ac) = s.ecx;
        s.ecx -= s.eax;
        GP4_GLOBAL(uint32_t, 0x006036b0) = s.ecx;

        if ((GP4_FIELD(uint8_t, state, 0x62e) & 0x80u) != 0u || (GP4_FIELD(uint8_t, state, 0x92e) & 0x80u) != 0u) {
            gp4::call_regs(0x00419220, s);
            s.eax = GP4_GLOBAL(uint32_t, 0x0060351c);
            if (static_cast<int32_t>(s.eax) > GP4_GLOBAL(int32_t, 0x0060150c)) {
                s.eax = GP4_GLOBAL(uint32_t, 0x006036ac);
                s.edx = GP4_GLOBAL(uint32_t, 0x006036a8);
                s.ebp = s.eax ^ s.edx;
                if (static_cast<int32_t>(s.ebp) >= 0) {
                    if (static_cast<int32_t>(s.eax) < 0) s.eax = 0u - s.eax;
                    if (static_cast<int32_t>(s.edx) < 0) s.edx = 0u - s.edx;
                    s.eax -= s.edx;
                    if (static_cast<int32_t>(s.eax) <= 0 || static_cast<int32_t>(s.eax) <= GP4_GLOBAL(int32_t, 0x006034c4)) {
                        gp4::call_regs(0x0041931d, s);
                        const uint32_t old = GP4_FIELD(uint32_t, car, 0x3c);
                        const int64_t product = static_cast<int64_t>(static_cast<int32_t>(old))
                            * static_cast<int64_t>(GP4_GLOBAL(int32_t, 0x00603518));
                        s.edx = old;
                        s.eax = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14) - old;
                        GP4_FIELD(uint32_t, car, 0x3c) += s.eax;
                        gp4::call_regs(0x00419333, s);
                        updateBodies(s);
                        goto done;
                    }
                }
            }
        }

        s.eax = GP4_GLOBAL(uint32_t, 0x00603680);
        GP4_GLOBAL(uint32_t, 0x0060368c) = s.eax;
        GP4_GLOBAL(uint32_t, 0x00624244) += 1u;
        GP4_GLOBAL(uint32_t, 0x00624248) += 1u;
        if (--GP4_GLOBAL(uint32_t, 0x006036bc) == 0u) break;
    }

    gp4::call_regs(0x00419333, s);
    if ((s.eflags & gp4::CF) != 0u) updateBodies(s);

done:
    r->eax = s.eax;
    r->ebx = stateAddr;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = s.eflags;
}
GP4_IMPL(0x004193fa, FUN_004193fa, "regs(esi:ptr[0x300]:layout=rec_004132e5+rec_0041325b+rec_00412b88+rec_00419220) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=recs_006380d8+state_00416dec+state_0041a2ec+state_0041416e+state_00412b88+state_00419220+state_0041931d+tyre_00601510+floor_00603498+state_00413397+wheel_004193fa")

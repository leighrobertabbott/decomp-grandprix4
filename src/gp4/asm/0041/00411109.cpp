#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF PF AF ZF SF OF) of `left - right` at the given width (8 or 16 bits).
static inline uint32_t subFlags(uint32_t left, uint32_t right, uint32_t bits) {
    const uint32_t mask = (1u << bits) - 1u;
    const uint32_t sign = 1u << (bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result) & sign) != 0u) ? 0x800u : 0u);
}

// Flags of a logic result (test / or / and) at the given width: CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result, uint32_t bits) {
    const uint32_t mask = bits == 32u ? 0xffffffffu : ((1u << bits) - 1u);
    const uint32_t sign = 1u << (bits - 1u);
    result &= mask;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u);
}

// Calls an original register routine that takes esi (and ecx / eax / edx) and returns flags.
static inline uint32_t callFlags(const gp4::Regs* r, uint32_t addr, uint32_t ecx, uint32_t eax, uint32_t edx) {
    gp4::Regs call = *r;
    call.ecx = ecx;
    call.eax = eax;
    call.edx = edx;
    call.st_in = 0;
    call.st_out = 0;
    gp4::call_regs(addr, call);
    return call.eflags & 0x8d5u;
}

struct Unknown_00411109 {
    uint8_t Unknown00[0x0e];
    int16_t Word0E;
    uint8_t Unknown10[0x1c - 0x10];
    uint32_t Ptr1C;            // points at two words: [0] and [0x18]
    uint8_t Unknown20[0x40 - 0x20];
    uint32_t State40;
    uint8_t Unknown44[0x70 - 0x44];
    uint8_t Byte70;
    uint8_t Unknown71[0x84 - 0x71];
    uint8_t Byte84;
    uint8_t Unknown85[0x8d - 0x85];
    uint8_t Flags8D;
    uint8_t Unknown8E[0x90 - 0x8e];
    int16_t Word90;
    uint8_t Unknown92[0x9a - 0x92];
    int16_t Word9A;
    uint8_t Unknown9C[0xb0 - 0x9c];
    int16_t WordB0;
    uint8_t UnknownB2[0xcd - 0xb2];
    uint8_t FlagsCD;
    uint8_t UnknownCE[0xee - 0xce];
    int16_t WordEE;
};

// FUNCTION: GP4 0x00411109
// Per-record check on the record at esi, the result of which is only the flag state and the flag
// bit 1 of +0xcd (set once a sound/event request has been queued through 0x00418fd0, cleared again
// when the pending tag was removed). eax (low word / byte as the original loads it) and edx
// (= [esi+0x1c] on the last path) are changed; ecx is preserved around each callee.
void __cdecl FUN_00411109(gp4::Regs* r) {
    Unknown_00411109* rec = reinterpret_cast<Unknown_00411109*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t edx = r->edx;
    uint32_t flags = 0;

    bool done = false;
    bool stageE = false;   // 0x0041117e
    bool stageD = false;   // 0x004111d9

    if ((rec->FlagsCD & 2u) != 0u) {
        flags = callFlags(r, 0x0041902e, 2u, eax, edx);
        if ((flags & 1u) == 0u) {
            done = true;
        } else {
            flags = callFlags(r, 0x004110dd, r->ecx, eax, edx);
            if ((flags & 1u) == 0u) {
                done = true;
            } else {
                flags = callFlags(r, 0x0041904e, 2u, eax, edx);
                rec->FlagsCD = static_cast<uint8_t>(rec->FlagsCD & 0xfdu);
                flags = logicFlags(rec->FlagsCD, 8u);
                stageE = true;
            }
        }
    } else {
        flags = logicFlags(rec->Flags8D & 2u, 8u);
        if ((rec->Flags8D & 2u) != 0u) {
            done = true;
        } else {
            const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
            flags = subFlags(mode, 0x80u, 8u);
            if (mode == 0x80u) {
                eax = rec->Byte70;
                const int16_t limit = GP4_GLOBAL(int16_t, 0x007c2e96);
                flags = subFlags(eax, static_cast<uint16_t>(limit), 16u);
                if (static_cast<int16_t>(eax) >= limit) {
                    done = true;
                }
            }
            if (!done) {
                flags = logicFlags(rec->State40, 32u);
                if (rec->State40 != 0u) {
                    flags = callFlags(r, 0x004110dd, r->ecx, eax, edx);
                    if ((flags & 1u) != 0u) {
                        stageE = true;
                    } else {
                        stageD = true;
                    }
                } else {
                    flags = logicFlags(rec->Flags8D & 0x10u, 8u);
                    if ((rec->Flags8D & 0x10u) == 0u) {
                        done = true;
                    } else {
                        eax = (eax & 0xffffff00u) | rec->Byte84;
                        flags = subFlags(rec->Byte84, 4u, 8u);
                        if (rec->Byte84 == 4u) {
                            done = true;
                        } else {
                            flags = subFlags(rec->Byte84, 3u, 8u);
                            if (rec->Byte84 == 3u) {
                                done = true;
                            } else {
                                flags = subFlags(rec->Byte84, 7u, 8u);
                                if (rec->Byte84 == 7u) {
                                    done = true;
                                } else {
                                    stageD = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (stageE && !done) {
        const int16_t value = rec->Word9A;
        eax = (eax & 0xffff0000u) | static_cast<uint16_t>(value);
        flags = subFlags(static_cast<uint16_t>(value), 0x1d5u, 16u);
        if (value >= 0x1d5) {
            done = true;
        } else {
            const uint8_t quiet = GP4_GLOBAL(uint8_t, 0x007c2e75);
            flags = subFlags(quiet, 0u, 8u);
            if (quiet == 0u) {
                flags = callFlags(r, 0x00418fd0, 0x12u, 0x3e8u, 0x3e8u);
            }
            rec->FlagsCD = static_cast<uint8_t>(rec->FlagsCD | 2u);
            flags = logicFlags(rec->FlagsCD, 8u);
            done = true;
        }
    } else if (stageD && !done) {
        uint16_t ax = static_cast<uint16_t>(rec->Word90);
        eax = (eax & 0xffff0000u) | ax;
        flags = logicFlags(ax, 16u);
        if (static_cast<int16_t>(ax) < 0) {
            flags = subFlags(0u, ax, 16u);
            ax = static_cast<uint16_t>(0u - ax);
            eax = (eax & 0xffff0000u) | ax;
        }
        flags = subFlags(ax, static_cast<uint16_t>(rec->WordB0), 16u);
        if (static_cast<int16_t>(ax) > rec->WordB0) {
            done = true;
        } else {
            edx = rec->Ptr1C;
            const int16_t* words = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(edx));
            ax = static_cast<uint16_t>(rec->Word0E);
            ax = static_cast<uint16_t>(ax - static_cast<uint16_t>(words[0]));
            ax = static_cast<uint16_t>(ax - static_cast<uint16_t>(words[0x18 / 2]));
            eax = (eax & 0xffff0000u) | ax;
            flags = logicFlags(ax, 16u);
            if (static_cast<int16_t>(ax) < 0) {
                flags = subFlags(0u, ax, 16u);
                ax = static_cast<uint16_t>(0u - ax);
                eax = (eax & 0xffff0000u) | ax;
            }
            flags = subFlags(ax, 0x2000u, 16u);
            if (static_cast<int16_t>(ax) >= 0x2000) {
                done = true;
            } else {
                ax = static_cast<uint16_t>(rec->WordEE);
                eax = (eax & 0xffff0000u) | ax;
                flags = subFlags(ax, 0x3000u, 16u);
                if (static_cast<int16_t>(ax) < 0x3000) {
                    done = true;
                } else {
                    const uint8_t quiet = GP4_GLOBAL(uint8_t, 0x007c2e75);
                    flags = subFlags(quiet, 0u, 8u);
                    if (quiet == 0u) {
                        flags = callFlags(r, 0x00418fd0, 2u, 0x1f40u, 0x1388u);
                    }
                    rec->FlagsCD = static_cast<uint8_t>(rec->FlagsCD | 2u);
                    flags = logicFlags(rec->FlagsCD, 8u);
                    done = true;
                }
            }
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00411109, FUN_00411109, "regs(eax:u32, edx:u32, esi:ptr[0x2c4]:layout=car_0041e500+car_00411109) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=state_0066d8c0+state_0040e84e+state_0041e500+state_00411109; cases=600")

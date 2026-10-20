#include <gp4/gp4.h>

enum : uint32_t { kArithFlags = 0x8d5u };

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x4u : 0u;
}

// Flags of a byte TEST / AND / OR: CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of `cmp left, right` with operand mask / sign bit.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right, uint32_t sign, uint32_t mask) {
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & sign) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & sign) != 0u ? 0x800u : 0u);
}

static inline void setFlags(gp4::Regs& s, uint32_t flags) {
    s.eflags = (s.eflags & ~kArithFlags) | flags;
}

// esi = car record, edi = its current segment record ([esi+0x1c], loaded by the caller 0x004362a9).
// Dispatches on the segment flag bits +0x12 (bit 0, bit 1) and +0x64 (bits 0, 1) and the car flag
// bytes, running the original car-event routines. The register file is threaded through every
// callee; the flags at the end are those of the last flag writer (here or in the last callee).
// FUNCTION: GP4 0x0040f1d1
void __cdecl FUN_0040f1d1(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t* seg = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->edi));

    gp4::Regs s = *r;
    s.st_in = 0;
    s.st_out = 0;

    uint32_t t;
    t = seg[0x12] & 1u;
    setFlags(s, logicFlags8(t));
    if (t == 0u) {
        t = car[0x21d] & 0x40u;
        setFlags(s, logicFlags8(t));
        if (t == 0u) {
            t = seg[0x12] & 2u;
            setFlags(s, logicFlags8(t));
            if (t != 0u) {
                const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
                setFlags(s, logicFlags8(mode));
                if ((mode & 0x80u) != 0u) {
                    t = car[0x8d] & 4u;
                    setFlags(s, logicFlags8(t));
                    if (t == 0u) {
                        const uint32_t level = car[0x70];
                        s.eax = (s.eax & ~0xffu) | level;
                        const uint32_t limit = GP4_GLOBAL(uint8_t, 0x007c2e96);
                        setFlags(s, cmpFlags(level, limit, 0x80u, 0xffu));
                        if (level == limit) {
                            const uint32_t done = GP4_GLOBAL(uint8_t, 0x0066d87f);
                            setFlags(s, logicFlags8(done));
                            if (done == 0u) {
                                gp4::call_regs(0x00441a78, s);
                                gp4::call_regs(0x00441c92, s);
                                GP4_GLOBAL(uint8_t, 0x0066d87f) = 0x80u;
                            }
                        }
                    }
                }
            } else {
                t = seg[0x64] & 1u;
                setFlags(s, logicFlags8(t));
                if (t != 0u) {
                    t = car[0x161] & 0x20u;
                    setFlags(s, logicFlags8(t));
                    if (t == 0u) {
                        car[0x161] = static_cast<uint8_t>(car[0x161] | 0x20u);
                        setFlags(s, logicFlags8(car[0x161]));
                        car[0x74] = 0u;
                        gp4::call_regs(0x0040f02e, s);
                    }
                } else {
                    t = seg[0x64] & 2u;
                    setFlags(s, logicFlags8(t));
                    if (t != 0u) {
                        t = car[0x161] & 0x10u;
                        setFlags(s, logicFlags8(t));
                        if (t == 0u) {
                            car[0x161] = static_cast<uint8_t>(car[0x161] | 0x10u);
                            setFlags(s, logicFlags8(car[0x161]));
                            car[0x74] = 1u;
                            gp4::call_regs(0x0040f02e, s);
                        }
                    }
                }
            }
        }
    } else {
        t = seg[0x12] & 2u;
        setFlags(s, logicFlags8(t));
        if (t != 0u) {
            bool go = true;
            t = car[0x8d] & 2u;
            setFlags(s, logicFlags8(t));
            if (t != 0u) {
                t = car[0x71] & 2u;
                setFlags(s, logicFlags8(t));
                go = (t == 0u);
            }
            if (go) {
                gp4::call_regs(0x0040f19d, s);
                if ((s.eflags & 1u) == 0u) {
                    car[0x21d] = static_cast<uint8_t>(car[0x21d] | 0x40u);
                    setFlags(s, logicFlags8(car[0x21d]));
                } else {
                    car[0x21d] = static_cast<uint8_t>(car[0x21d] & 0xbfu);
                    setFlags(s, logicFlags8(car[0x21d]));
                    t = car[0x8d] & 4u;
                    setFlags(s, logicFlags8(t));
                    if (t == 0u) {
                        GP4_GLOBAL(uint8_t, 0x00852d6e) = 0x80u;
                        t = car[0x71] & 2u;
                        setFlags(s, logicFlags8(t));
                        if (t == 0u) {
                            car[0x74] = 2u;
                            car[0x71] = static_cast<uint8_t>(car[0x71] | 2u);
                            car[0x161] = static_cast<uint8_t>(car[0x161] & 0xdfu);
                            car[0x161] = static_cast<uint8_t>(car[0x161] & 0xefu);
                            setFlags(s, logicFlags8(car[0x161]));
                            gp4::call_regs(0x0040f0dd, s);
                            gp4::call_regs(0x0040f112, s);
                            t = car[0x8d] & 2u;
                            setFlags(s, logicFlags8(t));
                            if (t == 0u) {
                                const uint32_t player = GP4_GLOBAL(uint32_t, 0x0066d820);
                                setFlags(s, cmpFlags(s.esi, player, 0x80000000u, 0xffffffffu));
                                if (s.esi == player) {
                                    GP4_GLOBAL(uint8_t, 0x006f2f67) = 0xffu;
                                }
                                gp4::call_regs(0x0040efa2, s);
                                gp4::call_regs(0x0040ed29, s);
                                gp4::call_regs(0x0040eb43, s);
                                gp4::call_regs(0x0040eb6b, s);
                                const uint32_t quiet = GP4_GLOBAL(uint8_t, 0x007c2e75);
                                setFlags(s, cmpFlags(quiet, 0u, 0x80u, 0xffu));
                                if (quiet == 0u) {
                                    gp4::call_regs(0x0040f175, s);
                                }
                            } else {
                                GP4_GLOBAL(uint8_t, 0x00852d6e) = 0x80u;
                                GP4_FIELD(uint32_t, car, 0xc0) = 0xc0000000u;
                                GP4_FIELD(uint32_t, car, 0x28) = 0x10000000u;
                                gp4::call_regs(0x0040ed29, s);
                                const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
                                setFlags(s, logicFlags8(mode));
                                if ((mode & 0x80u) == 0u) {
                                    gp4::call_regs(0x0040e97d, s);
                                    car[0x89] = static_cast<uint8_t>(car[0x89] | 4u);
                                } else {
                                    gp4::call_regs(0x0040e949, s);
                                    car[0x83] = static_cast<uint8_t>(car[0x83] | 8u);
                                }
                                car[0x88] = static_cast<uint8_t>(car[0x88] | 0x40u);
                                setFlags(s, logicFlags8(car[0x88]));
                            }
                        }
                    }
                }
            }
        } else {
            t = car[0x21d] & 0x40u;
            setFlags(s, logicFlags8(t));
            if (t == 0u) {
                car[0x71] = static_cast<uint8_t>(car[0x71] & 0xfdu);
                car[0x231] = static_cast<uint8_t>(car[0x231] & 0xfbu);
                t = car[0x8d] & 4u;
                setFlags(s, logicFlags8(t));
                if (t == 0u) {
                    car[0x9c] = static_cast<uint8_t>(car[0x9c] | 0x20u);
                    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
                    setFlags(s, logicFlags8(mode));
                    if ((mode & 0x80u) != 0u) {
                        const uint32_t next = (GP4_GLOBAL(uint8_t, 0x0066d8c8) + 1u) & 0xffu;
                        s.eax = (s.eax & ~0xffu) | next;
                        const uint32_t limit = GP4_GLOBAL(uint8_t, 0x007c2e96);
                        setFlags(s, cmpFlags(next, limit, 0x80u, 0xffu));
                        if (static_cast<int8_t>(next) >= static_cast<int8_t>(limit)) {
                            t = car[0xfe] & 4u;
                            setFlags(s, logicFlags8(t));
                            if (t != 0u) {
                                t = car[0x8d] & 2u;
                                setFlags(s, logicFlags8(t));
                                if (t == 0u) {
                                    t = car[0xe5] & 4u;
                                    setFlags(s, logicFlags8(t));
                                    if (t != 0u) {
                                        car[0xfe] = static_cast<uint8_t>(car[0xfe] & 0xfbu);
                                        car[0x71] = static_cast<uint8_t>(car[0x71] & 0x7fu);
                                        setFlags(s, logicFlags8(car[0x71]));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    r->eax = s.eax;
    r->ebx = s.ebx;
    r->ecx = s.ecx;
    r->edx = s.edx;
    r->esi = s.esi;
    r->edi = s.edi;
    r->ebp = s.ebp;
    r->eflags = (r->eflags & ~kArithFlags) | (s.eflags & kArithFlags);
}
GP4_IMPL(0x0040f1d1, FUN_0040f1d1, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:ptr[0x420]:layout=car_004129fa+rec_0040e84e+rec_0040e9ac+rec_0040ee7b+rec_0040f112+rec_0040efa2+body_link_0040f19d, edi:ptr[0x6c]:bytes) -> (eax, ebx, ecx, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040e9ac+state_0040edc3+state_0040ee7b+carno_0066ff6c+order_00675958+state_0040f06c+state_0040efa2+state_0040e949+state_0040e84e+state_004327cd+state_0040eb6b+state_0040ed29+state_0040f112+counter_0040f0dd+ring_008943a0+words_008943a0+ptr_0086850c+geom_008943a0")

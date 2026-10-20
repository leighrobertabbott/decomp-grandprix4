#include <gp4/gp4.h>
#include <gp4/x87.h>

using gp4::x87::Ext;

static inline bool extIsZero(const Ext& e) {
    for (int i = 0; i < 8; ++i) {
        if (e.b[i] != 0) return false;
    }
    return e.b[8] == 0 && (e.b[9] & 0x7f) == 0;
}

static inline bool extIsNaN(const Ext& e) {
    if (e.b[8] != 0xff || (e.b[9] & 0x7f) != 0x7f) return false;
    if ((e.b[7] & 0x7f) != 0) return true;
    for (int i = 0; i < 7; ++i) {
        if (e.b[i] != 0) return true;
    }
    return false;
}

static inline bool extSign(const Ext& e) { return (e.b[9] & 0x80) != 0; }

// C3/C2/C0 of an FUCOM, as SAHF loads them into ZF/PF/CF
static inline uint32_t fucomFlags(bool unordered, bool less, bool equal) {
    if (unordered) return 0x45u;
    if (less) return 0x01u;
    if (equal) return 0x40u;
    return 0u;
}

// FUNCTION: GP4 0x00401ccc
// st0 = limit, st1 = divisor, st2 = dividend on entry; one value on exit:
// dividend / divisor when its magnitude is below the limit, else the limit
// carrying the sign of the ratio (of the dividend when the divisor is zero).
void __cdecl FUN_00401ccc(gp4::Regs* r) {
    const Ext limit = gp4::x87::in_st(r, 0);
    const Ext divisor = gp4::x87::in_st(r, 1);
    const Ext dividend = gp4::x87::in_st(r, 2);
    Ext result;
    uint32_t flags;

    if (extIsZero(divisor) || extIsNaN(divisor)) {
        // ftst: zero (or unordered) divisor, the ratio is not formed
        const bool unordered = extIsNaN(dividend);
        const bool zero = extIsZero(dividend);
        const bool less = !unordered && !zero && extSign(dividend);
        flags = fucomFlags(unordered, less, zero);
        result = limit * gp4::x87::ext((unordered || less) ? -1.0 : 1.0);
    } else {
        const Ext q = dividend / divisor;
        Ext absq = q;
        absq.b[9] &= 0x7f;
        const bool unordered = extIsNaN(q) || extIsNaN(limit);
        bool less = false;
        if (!unordered) {
            const Ext diff = absq - limit;
            less = !extIsNaN(diff) && extSign(diff);
        }
        if (unordered || less) {
            // |q| < limit (or unordered): the ratio itself comes back
            flags = fucomFlags(unordered, true, false);
            result = q;
        } else {
            const bool zero = extIsZero(q);
            const bool neg = extSign(q) && !zero;
            flags = fucomFlags(false, neg, zero);
            result = limit * gp4::x87::ext(neg ? -1.0 : 1.0);
        }
    }

    // FNSTSW/SAHF with three values entered: TOP is 6 or 7, so AF (TOP bit 1) = 1; SF (B) = 0
    flags |= 0x10u;
    gp4::x87::set_out_st(r, 0, result);
    r->eflags = (r->eflags & ~0xd5u) | flags;
}
GP4_IMPL(0x00401ccc, FUN_00401ccc, "regs(st0:f64, st1:f64, st2:f64) -> (st0, cf, pf, af, zf, sf)")

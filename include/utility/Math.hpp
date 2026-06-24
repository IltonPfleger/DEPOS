#ifndef __QUARK_MATH__
#define __QUARK_MATH__

namespace QUARK::Math {

constexpr double abs(double x) { return x < 0 ? -x : x; }

constexpr double sqrt(double x) {
    if (x <= 0.0) return 0.0;

    double r = x > 1.0 ? x : 1.0;

    while (true) {
        double next = 0.5 * (r + x / r);

        double error = abs(next - r);

        if (error < 1e-12) return next;

        r = next;
    }
}

} // namespace QUARK::Math

#endif

/*
 * Copyright Nick Thompson, 2020
 * Use, modification and distribution are subject to the
 * Boost Software License, Version 1.0. (See accompanying file
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "math_unit_test.hpp"
#include <numeric>
#include <utility>
#include <vector>
#include <array>
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/math/interpolators/quintic_hermite.hpp>
#include <boost/circular_buffer.hpp>
#ifdef BOOST_HAS_FLOAT128
#include <boost/multiprecision/float128.hpp>
using boost::multiprecision::float128;
#endif


using boost::math::interpolators::quintic_hermite;
using boost::math::interpolators::cardinal_quintic_hermite;
using boost::math::interpolators::cardinal_quintic_hermite_aos;

template<typename Real>
void test_constant()
{
    std::vector<Real> x{0,1,2,3, 9, 22, 81};
    std::vector<Real> y(x.size());
    std::vector<Real> dydx(x.size(), 0);
    std::vector<Real> d2ydx2(x.size(), 0);
    for (auto & t : y)
    {
        t = 7;
    }

    auto qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));
    for (Real t = 0; t <= 81; t += 0.25)
    {
        CHECK_ULP_CLOSE(Real(7), qh(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh.prime(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh.double_prime(t), 24);
    }
}


template<typename Real>
void test_linear()
{
    std::vector<Real> x{0,1,2,3, 4,5,6,7,8,9};
    std::vector<Real> y = x;
    std::vector<Real> dydx(x.size(), 1);
    std::vector<Real> d2ydx2(x.size(), 0);

    auto qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = 0; t <= 9; t += 0.25)
    {
        CHECK_ULP_CLOSE(Real(t), qh(t), 2);
        CHECK_ULP_CLOSE(Real(1), qh.prime(t), 2);
        CHECK_ULP_CLOSE(Real(0), qh.double_prime(t), 2);
    }

    boost::random::mt19937 rng;
    boost::random::uniform_real_distribution<Real> dis(0.5,1);
    x.resize(512);
    x[0] = dis(rng);
    Real xmin = x[0];
    for (size_t i = 1; i < x.size(); ++i)
    {
        x[i] = x[i-1] + dis(rng);
    }
    Real xmax = x.back();

    y = x;
    dydx.resize(x.size(), 1);
    d2ydx2.resize(x.size(), 0);

    qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = xmin; t <= xmax; t += 0.125)
    {
        CHECK_ULP_CLOSE(t, qh(t), 2);
        CHECK_ULP_CLOSE(Real(1), qh.prime(t), 100);
        CHECK_MOLLIFIED_CLOSE(Real(0), qh.double_prime(t), 200*std::numeric_limits<Real>::epsilon());
    }
}

template<typename Real>
void test_quadratic()
{

    std::vector<Real> x{0,1,2,3, 4,5,6,7,8,9};
    std::vector<Real> y(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = x[i]*x[i]/2;
    }

    std::vector<Real> dydx(x.size());
    for (size_t i = 0; i < y.size(); ++i) {
        dydx[i] = x[i];
    }

    std::vector<Real> d2ydx2(x.size(), 1);

    auto qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = 0; t <= 9; t += 0.0078125)
    {
        CHECK_ULP_CLOSE(Real(t*t)/2, qh(t), 2);
        CHECK_ULP_CLOSE(t, qh.prime(t), 12);
        CHECK_ULP_CLOSE(Real(1), qh.double_prime(t), 32);
    }

    boost::random::mt19937 rng;
    boost::random::uniform_real_distribution<Real> dis(0.5,1);
    x.resize(8);
    x[0] = dis(rng);
    Real xmin = x[0];
    for (size_t i = 1; i < x.size(); ++i)
    {
        x[i] = x[i-1] + dis(rng);
    }
    Real xmax = x.back();

    y.resize(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = x[i]*x[i]/2;
    }

    dydx.resize(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        dydx[i] = x[i];
    }

    d2ydx2.resize(x.size(), 1);

    qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = xmin; t <= xmax; t += 0.125)
    {
        CHECK_ULP_CLOSE(Real(t*t)/2, qh(t), 4);
        CHECK_ULP_CLOSE(t, qh.prime(t), 53);
        CHECK_ULP_CLOSE(Real(1), qh.double_prime(t), 700);
    }
}

template<typename Real>
void test_cubic()
{

    std::vector<Real> x{0,1,2,3, 4,5,6,7,8,9};
    std::vector<Real> y(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = x[i]*x[i]*x[i];
    }

    std::vector<Real> dydx(x.size());
    for (size_t i = 0; i < y.size(); ++i) {
        dydx[i] = 3*x[i]*x[i];
    }

    std::vector<Real> d2ydx2(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        d2ydx2[i] = 6*x[i];
    }

    auto qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = 0; t <= 9; t += 0.0078125)
    {
        CHECK_ULP_CLOSE(t*t*t, qh(t), 10);
        CHECK_ULP_CLOSE(3*t*t, qh.prime(t), 15);
        CHECK_ULP_CLOSE(6*t, qh.double_prime(t), 20);
    }
}

template<typename Real>
void test_quartic()
{

    std::vector<Real> x{0,1,2,3, 4,5,6,7,8,9, 10, 11};
    std::vector<Real> y(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = x[i]*x[i]*x[i]*x[i];
    }

    std::vector<Real> dydx(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        dydx[i] = 4*x[i]*x[i]*x[i];
    }

    std::vector<Real> d2ydx2(x.size());
    for (size_t i = 0; i < y.size(); ++i)
    {
        d2ydx2[i] = 12*x[i]*x[i];
    }

    auto qh = quintic_hermite(std::move(x), std::move(y), std::move(dydx), std::move(d2ydx2));

    for (Real t = 1; t <= 11; t += 0.0078125)
    {
        CHECK_ULP_CLOSE(t*t*t*t, qh(t), 100);
        CHECK_ULP_CLOSE(4*t*t*t, qh.prime(t), 100);
        CHECK_ULP_CLOSE(12*t*t, qh.double_prime(t), 100);
    }
}


template<typename Real>
void test_interpolation_condition()
{
    for (size_t n = 4; n < 50; ++n) {
        std::vector<Real> x(n);
        std::vector<Real> y(n);
        std::vector<Real> dydx(n);
        std::vector<Real> d2ydx2(n);
        boost::random::mt19937 rd; 
        boost::random::uniform_real_distribution<Real> dis(0,1);
        Real x0 = dis(rd);
        x[0] = x0;
        y[0] = dis(rd);
        for (size_t i = 1; i < n; ++i) {
            x[i] = x[i-1] + dis(rd);
            y[i] = dis(rd);
            dydx[i] = dis(rd);
            d2ydx2[i] = dis(rd);
        }

        auto x_copy = x;
        auto y_copy = y;
        auto dydx_copy = dydx;
        auto d2ydx2_copy = d2ydx2;
        auto s = quintic_hermite(std::move(x_copy), std::move(y_copy), std::move(dydx_copy), std::move(d2ydx2_copy));
        //std::cout << "s = " << s << "\n";
        for (size_t i = 0; i < x.size(); ++i) {
            CHECK_ULP_CLOSE(y[i], s(x[i]), 2);
            CHECK_ULP_CLOSE(dydx[i], s.prime(x[i]), 2);
            CHECK_ULP_CLOSE(d2ydx2[i], s.double_prime(x[i]), 2);
        }
    }
}

template<typename Real>
void test_cardinal_constant()
{

    std::vector<Real> y(25);
    std::vector<Real> dydx(y.size(), 0);
    std::vector<Real> d2ydx2(y.size(), 0);
    for (auto & t : y) {
        t = 7;
    }
    Real x0 = 4;
    Real dx = Real(1)/Real(8);

    auto qh = cardinal_quintic_hermite(std::move(y), std::move(dydx), std::move(d2ydx2), x0, dx);

    for (Real t = x0; t <= x0 + 24*dx; t += 0.25)
    {
        CHECK_ULP_CLOSE(Real(7), qh(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh.prime(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh.double_prime(t), 24);
    }

    std::vector<std::array<Real, 3>> data(25);
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i][0] = 7;
        data[i][1] = 0;
        data[i][2] = 0;
    }

    auto qh_aos = cardinal_quintic_hermite_aos(std::move(data), x0, dx);
    for (Real t = x0; t <= x0 + 24*dx; t += 0.25)
    {
        CHECK_ULP_CLOSE(Real(7), qh_aos(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh_aos.prime(t), 24);
        CHECK_ULP_CLOSE(Real(0), qh_aos.double_prime(t), 24);
    }
}


template<typename Real>
void test_cardinal_linear()
{
    std::vector<Real> y{0,1,2,3,4,5,6,7,8,9};
    Real x0 = 0;
    Real dx = 1;
    std::vector<Real> dydx(y.size(), 1);
    std::vector<Real> d2ydx2(y.size(), 0);

    auto qh = cardinal_quintic_hermite(std::move(y), std::move(dydx), std::move(d2ydx2), x0, dx);

    for (Real t = 0; t <= 9; t += 0.25) {
        CHECK_ULP_CLOSE(Real(t), qh(t), 2);
        CHECK_ULP_CLOSE(Real(1), qh.prime(t), 2);
        CHECK_ULP_CLOSE(Real(0), qh.double_prime(t), 2);
    }

    std::vector<std::array<Real, 3>> data(10);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i][0] = i;
        data[i][1] = 1;
        data[i][2] = 0;
    }

    auto qh_aos = cardinal_quintic_hermite_aos(std::move(data), x0, dx);

    for (Real t = 0; t <= 9; t += 0.25) {
        CHECK_ULP_CLOSE(Real(t), qh_aos(t), 2);
        CHECK_ULP_CLOSE(Real(1), qh_aos.prime(t), 2);
        CHECK_ULP_CLOSE(Real(0), qh_aos.double_prime(t), 2);
    }

}

template<typename Real>
void test_cardinal_quadratic()
{
    Real x0 = 0;
    Real dx = 1;
    std::vector<Real> y(10);
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = i*i/Real(2);
    }

    std::vector<Real> dydx(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        dydx[i] = i;
    }

    std::vector<Real> d2ydx2(y.size(), 1);

    auto qh = cardinal_quintic_hermite(std::move(y), std::move(dydx), std::move(d2ydx2), x0, dx);

    for (Real t = 0; t <= 9; t += 0.0078125) {
        Real computed = qh(t);
        CHECK_ULP_CLOSE(Real(t*t)/2, computed, 2);
        CHECK_ULP_CLOSE(t, qh.prime(t), 15);
        CHECK_ULP_CLOSE(Real(1), qh.double_prime(t), 32);
    }

    std::vector<std::array<Real, 3>> data(10);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i][0] = i*i/Real(2);
        data[i][1] = i;
        data[i][2] = 1;
    }
    auto qh_aos = cardinal_quintic_hermite_aos(std::move(data), x0, dx);

    for (Real t = 0; t <= 9; t += 0.0078125)
    {
        Real computed = qh_aos(t);
        CHECK_ULP_CLOSE(Real(t*t)/2, computed, 2);
        CHECK_ULP_CLOSE(t, qh_aos.prime(t), 12);
        CHECK_ULP_CLOSE(Real(1), qh_aos.double_prime(t), 64);
    }
}

template<typename Real>
void test_cardinal_cubic()
{
    Real x0 = 0;
    Real dx = 1;
    std::vector<Real> y(10);
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = i*i*i;
    }

    std::vector<Real> dydx(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        dydx[i] = 3*i*i;
    }

    std::vector<Real> d2ydx2(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        d2ydx2[i] = 6*i;
    }

    auto qh = cardinal_quintic_hermite(std::move(y), std::move(dydx), std::move(d2ydx2), x0, dx);

    for (Real t = 0; t <= 9; t += 0.0078125)
    {
        Real computed = qh(t);
        CHECK_ULP_CLOSE(t*t*t, computed, 10);
        CHECK_ULP_CLOSE(3*t*t, qh.prime(t), 15);
        CHECK_ULP_CLOSE(6*t, qh.double_prime(t), 39);
    }

    std::vector<std::array<Real, 3>> data(10);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i][0] = i*i*i;
        data[i][1] = 3*i*i;
        data[i][2] = 6*i;
    }

    auto qh_aos = cardinal_quintic_hermite_aos(std::move(data), x0, dx);
    for (Real t = 0; t <= 9; t += 0.0078125)
    {
        Real computed = qh_aos(t);
        CHECK_ULP_CLOSE(t*t*t, computed, 10);
        CHECK_ULP_CLOSE(3*t*t, qh_aos.prime(t), 15);
        CHECK_ULP_CLOSE(6*t, qh_aos.double_prime(t), 30);
    }
}

template<typename Real>
void test_cardinal_quartic()
{
    Real x0 = 0;
    Real dx = 1;
    std::vector<Real> y(7);
    for (size_t i = 0; i < y.size(); ++i)
    {
        y[i] = i*i*i*i;
    }

    std::vector<Real> dydx(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        dydx[i] = 4*i*i*i;
    }

    std::vector<Real> d2ydx2(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        d2ydx2[i] = 12*i*i;
    }

    auto qh = cardinal_quintic_hermite(std::move(y), std::move(dydx), std::move(d2ydx2), x0, dx);

    for (Real t = 0; t <= 6; t += 0.0078125)
    {
        CHECK_ULP_CLOSE(Real(t*t*t*t), qh(t), 250);
        CHECK_ULP_CLOSE(4*t*t*t, qh.prime(t), 250);
        CHECK_ULP_CLOSE(12*t*t, qh.double_prime(t), 250);
    }

    std::vector<std::array<Real, 3>> data(7);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i][0] = i*i*i*i;
        data[i][1] = 4*i*i*i;
        data[i][2] = 12*i*i;
    }

    auto qh_aos = cardinal_quintic_hermite_aos(std::move(data), x0, dx);
    for (Real t = 0; t <= 6; t += 0.0078125)
    {
        Real computed = qh_aos(t);
        CHECK_ULP_CLOSE(t*t*t*t, computed, 10);
        CHECK_ULP_CLOSE(4*t*t*t, qh_aos.prime(t), 64);
        CHECK_ULP_CLOSE(12*t*t, qh_aos.double_prime(t), 128);
    }
}


int main()
{
    test_constant<float>();
    test_linear<float>();
    test_quadratic<float>();
    test_cubic<float>();
    test_quartic<float>();
    test_interpolation_condition<float>();

    test_cardinal_constant<float>();
    test_cardinal_linear<float>();
    test_cardinal_quadratic<float>();
    test_cardinal_cubic<float>();
    test_cardinal_quartic<float>();

    test_constant<double>();
    test_linear<double>();
    test_quadratic<double>();
    test_cubic<double>();
    test_quartic<double>();
    test_interpolation_condition<double>();

    test_cardinal_constant<double>();
    test_cardinal_linear<double>();
    test_cardinal_quadratic<double>();
    test_cardinal_cubic<double>();
    test_cardinal_quartic<double>();

    test_constant<long double>();
    test_linear<long double>();
    test_quadratic<long double>();
    test_cubic<long double>();
    test_quartic<long double>();
    test_interpolation_condition<long double>();

    test_cardinal_constant<long double>();
    test_cardinal_linear<long double>();
    test_cardinal_quadratic<long double>();
    test_cardinal_cubic<long double>();
    test_cardinal_quartic<long double>();

#ifdef BOOST_HAS_FLOAT128
    test_constant<float128>();
    //test_linear<float128>();
    test_quadratic<float128>();
    test_cubic<float128>();
    test_quartic<float128>();
    test_interpolation_condition<float128>();
    test_cardinal_constant<float128>();
    test_cardinal_linear<float128>();
    test_cardinal_quadratic<float128>();
    test_cardinal_cubic<float128>();
    test_cardinal_quartic<float128>();
#endif

    return boost::math::test::report_errors();
}

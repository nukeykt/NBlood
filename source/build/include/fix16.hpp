#ifndef __libfixmath_fix16_hpp__
#define __libfixmath_fix16_hpp__

#include "fix16.h"

class Fix16 {
    public:
        fix16_t m_value;

        Fix16() { m_value = 0; }
        Fix16(const Fix16 & inValue) { m_value = inValue.m_value; }
        Fix16(const fix16_t inValue) { m_value = inValue; }
        Fix16(const float   inValue) { m_value = fix16_from_float(inValue); }
        Fix16(const double  inValue) { m_value = fix16_from_dbl(inValue); }
        Fix16(const int16_t inValue) { m_value = fix16_from_int(inValue); }

        operator fix16_t() const { return m_value; }
        operator double()  const { return fix16_to_dbl(m_value); }
        operator float()   const { return fix16_to_float(m_value); }
        operator int16_t() const { return fix16_to_int(m_value); }

        Fix16 &operator=(const Fix16 & rhs) { m_value = rhs.m_value; return *this; }
        Fix16 &operator=(const fix16_t rhs) { m_value = rhs; return *this; }
        Fix16 &operator=(const double  rhs) { m_value = fix16_from_dbl(rhs); return *this; }
        Fix16 &operator=(const float   rhs) { m_value = fix16_from_float(rhs); return *this; }
        Fix16 &operator=(const int16_t rhs) { m_value = fix16_from_int(rhs); return *this; }

        Fix16 &operator+=(const Fix16 & rhs) { m_value += rhs.m_value; return *this; }
        Fix16 &operator+=(const fix16_t rhs) { m_value += rhs; return *this; }
        Fix16 &operator+=(const double  rhs) { m_value += fix16_from_dbl(rhs); return *this; }
        Fix16 &operator+=(const float   rhs) { m_value += fix16_from_float(rhs); return *this; }
        Fix16 &operator+=(const int16_t rhs) { m_value += fix16_from_int(rhs); return *this; }

        Fix16 &operator-=(const Fix16 & rhs) { m_value -= rhs.m_value; return *this; }
        Fix16 &operator-=(const fix16_t rhs) { m_value -= rhs; return *this; }
        Fix16 &operator-=(const double  rhs) { m_value -= fix16_from_dbl(rhs); return *this; }
        Fix16 &operator-=(const float   rhs) { m_value -= fix16_from_float(rhs); return *this; }
        Fix16 &operator-=(const int16_t rhs) { m_value -= fix16_from_int(rhs); return *this; }

        Fix16 &operator*=(const Fix16 & rhs) { m_value = fix16_mul(m_value, rhs.m_value); return *this; }
        Fix16 &operator*=(const fix16_t rhs) { m_value = fix16_mul(m_value, rhs); return *this; }
        Fix16 &operator*=(const double  rhs) { m_value = fix16_mul(m_value, fix16_from_dbl(rhs)); return *this; }
        Fix16 &operator*=(const float   rhs) { m_value = fix16_mul(m_value, fix16_from_float(rhs)); return *this; }
        Fix16 &operator*=(const int16_t rhs) { m_value *= rhs; return *this; }

        Fix16 &operator/=(const Fix16 & rhs) { m_value = fix16_div(m_value, rhs.m_value); return *this; }
        Fix16 &operator/=(const fix16_t rhs) { m_value = fix16_div(m_value, rhs); return *this; }
        Fix16 &operator/=(const double  rhs) { m_value = fix16_div(m_value, fix16_from_dbl(rhs)); return *this; }
        Fix16 &operator/=(const float   rhs) { m_value = fix16_div(m_value, fix16_from_float(rhs)); return *this; }
        Fix16 &operator/=(const int16_t rhs) { m_value = tabledivide32(m_value, rhs); return *this; }

        Fix16 &operator%=(const Fix16 & rhs) { m_value = fix16_mod(m_value, rhs.m_value); return *this; }
        Fix16 &operator%=(const fix16_t rhs) { m_value = fix16_mod(m_value, rhs); return *this; }
        Fix16 &operator%=(const double  rhs) { m_value = fix16_mod(m_value, fix16_from_dbl(rhs)); return *this; }
        Fix16 &operator%=(const float   rhs) { m_value = fix16_mod(m_value, fix16_from_float(rhs)); return *this; }
        Fix16 &operator%=(const int16_t rhs) { m_value %= rhs; return *this; }

        Fix16 &operator&=(const Fix16 & rhs) { m_value = fix16_from_int(fix16_to_int(m_value) & fix16_to_int(rhs.m_value)); return *this; }
        Fix16 &operator&=(const fix16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) & fix16_to_int(rhs)); return *this; }
        Fix16 &operator&=(const int16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) & rhs); return *this; }

        Fix16 &operator^=(const Fix16 & rhs) { m_value = fix16_from_int(fix16_to_int(m_value) ^ fix16_to_int(rhs.m_value)); return *this; }
        Fix16 &operator^=(const fix16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) ^ fix16_to_int(rhs)); return *this; }
        Fix16 &operator^=(const int16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) ^ rhs); return *this; }

        Fix16 &operator|=(const Fix16 & rhs) { m_value = fix16_from_int(fix16_to_int(m_value) | fix16_to_int(rhs.m_value)); return *this; }
        Fix16 &operator|=(const fix16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) | fix16_to_int(rhs)); return *this; }
        Fix16 &operator|=(const int16_t rhs) { m_value = fix16_from_int(fix16_to_int(m_value) | rhs); return *this; }

        Fix16 &operator<<=(const Fix16 & rhs) { m_value <<= rhs.m_value; return *this; }
        Fix16 &operator<<=(const fix16_t rhs) { m_value <<= rhs; return *this; }
        Fix16 &operator<<=(const int16_t rhs) { m_value <<= fix16_from_int(rhs); return *this; }

        Fix16 &operator>>=(const Fix16 & rhs) { m_value >>= rhs.m_value; return *this; }
        Fix16 &operator>>=(const fix16_t rhs) { m_value >>= rhs; return *this; }
        Fix16 &operator>>=(const int16_t rhs) { m_value >>= fix16_from_int(rhs); return *this; }

        const Fix16 operator+(const Fix16 & other) const { return fix16_add(m_value, other.m_value); }
        const Fix16 operator+(const fix16_t other) const { return fix16_add(m_value, other); }
        const Fix16 operator+(const double  other) const { return fix16_add(m_value, fix16_from_dbl(other)); }
        const Fix16 operator+(const float   other) const { return fix16_add(m_value, fix16_from_float(other)); }
        const Fix16 operator+(const int16_t other) const { return fix16_add(m_value, fix16_from_int(other)); }

        const Fix16 sadd(const Fix16 & other) const { return fix16_sadd(m_value, other.m_value); }
        const Fix16 sadd(const fix16_t other) const { return fix16_sadd(m_value, other); }
        const Fix16 sadd(const double  other) const { return fix16_sadd(m_value, fix16_from_dbl(other)); }
        const Fix16 sadd(const float   other) const { return fix16_sadd(m_value, fix16_from_float(other)); }
        const Fix16 sadd(const int16_t other) const { return fix16_sadd(m_value, fix16_from_int(other)); }

        const Fix16 operator-(const Fix16 & other) const { return fix16_add(m_value, -other.m_value); }
        const Fix16 operator-(const fix16_t other) const { return fix16_add(m_value, -other); }
        const Fix16 operator-(const double  other) const { return fix16_add(m_value, -fix16_from_dbl(other)); }
        const Fix16 operator-(const float   other) const { return fix16_add(m_value, -fix16_from_float(other)); }
        const Fix16 operator-(const int16_t other) const { return fix16_add(m_value, -fix16_from_int(other)); }

        const Fix16 ssub(const Fix16 & other) const { return fix16_sadd(m_value, -other.m_value); }
        const Fix16 ssub(const fix16_t other) const { return fix16_sadd(m_value, -other); }
        const Fix16 ssub(const double  other) const { return fix16_sadd(m_value, -fix16_from_dbl(other)); }
        const Fix16 ssub(const float   other) const { return fix16_sadd(m_value, -fix16_from_float(other)); }
        const Fix16 ssub(const int16_t other) const { return fix16_sadd(m_value, -fix16_from_int(other)); }

        const Fix16 operator*(const Fix16 & other) const { return fix16_mul(m_value, other.m_value); }
        const Fix16 operator*(const fix16_t other) const { return fix16_mul(m_value, other); }
        const Fix16 operator*(const double  other) const { return fix16_mul(m_value, fix16_from_dbl(other)); }
        const Fix16 operator*(const float   other) const { return fix16_mul(m_value, fix16_from_float(other)); }
        const Fix16 operator*(const int16_t other) const { return fix16_mul(m_value, fix16_from_int(other)); }

        const Fix16 smul(const Fix16 & other) const { return fix16_smul(m_value, other.m_value); }
        const Fix16 smul(const fix16_t other) const { return fix16_smul(m_value, other); }
        const Fix16 smul(const double  other) const { return fix16_smul(m_value, fix16_from_dbl(other)); }
        const Fix16 smul(const float   other) const { return fix16_smul(m_value, fix16_from_float(other)); }
        const Fix16 smul(const int16_t other) const { return fix16_smul(m_value, fix16_from_int(other)); }

        const Fix16 operator/(const Fix16 & other) const { return fix16_div(m_value, other.m_value); }
        const Fix16 operator/(const fix16_t other) const { return fix16_div(m_value, other); }
        const Fix16 operator/(const double  other) const { return fix16_div(m_value, fix16_from_dbl(other)); }
        const Fix16 operator/(const float   other) const { return fix16_div(m_value, fix16_from_float(other)); }
        const Fix16 operator/(const int16_t other) const { return fix16_div(m_value, fix16_from_int(other)); }

        const Fix16 sdiv(const Fix16 & other) const { return fix16_sdiv(m_value, other.m_value); }
        const Fix16 sdiv(const fix16_t other) const { return fix16_sdiv(m_value, other); }
        const Fix16 sdiv(const double  other) const { return fix16_sdiv(m_value, fix16_from_dbl(other)); }
        const Fix16 sdiv(const float   other) const { return fix16_sdiv(m_value, fix16_from_float(other)); }
        const Fix16 sdiv(const int16_t other) const { return fix16_sdiv(m_value, fix16_from_int(other)); }

        bool operator==(const Fix16 & other) const { return m_value == other.m_value; }
        bool operator==(const fix16_t other) const { return m_value == other; }
        bool operator==(const double  other) const { return m_value == fix16_from_dbl(other); }
        bool operator==(const float   other) const { return m_value == fix16_from_float(other); }
        bool operator==(const int16_t other) const { return m_value == fix16_from_int(other); }

        bool operator!=(const Fix16 & other) const { return m_value != other.m_value; }
        bool operator!=(const fix16_t other) const { return m_value != other; }
        bool operator!=(const double  other) const { return m_value != fix16_from_dbl(other); }
        bool operator!=(const float   other) const { return m_value != fix16_from_float(other); }
        bool operator!=(const int16_t other) const { return m_value != fix16_from_int(other); }

        bool operator<=(const Fix16 & other) const { return m_value <= other.m_value; }
        bool operator<=(const fix16_t other) const { return m_value <= other; }
        bool operator<=(const double  other) const { return m_value <= fix16_from_dbl(other); }
        bool operator<=(const float   other) const { return m_value <= fix16_from_float(other); }
        bool operator<=(const int16_t other) const { return m_value <= fix16_from_int(other); }

        bool operator>=(const Fix16 & other) const { return m_value >= other.m_value; }
        bool operator>=(const fix16_t other) const { return m_value >= other; }
        bool operator>=(const double  other) const { return m_value >= fix16_from_dbl(other); }
        bool operator>=(const float   other) const { return m_value >= fix16_from_float(other); }
        bool operator>=(const int16_t other) const { return m_value >= fix16_from_int(other); }

        bool operator<(const Fix16 & other) const { return m_value < other.m_value; }
        bool operator<(const fix16_t other) const { return m_value < other; }
        bool operator<(const double  other) const { return m_value < fix16_from_dbl(other); }
        bool operator<(const float   other) const { return m_value < fix16_from_float(other); }
        bool operator<(const int16_t other) const { return m_value < fix16_from_int(other); }

        bool operator>(const Fix16 & other) const { return m_value > other.m_value; }
        bool operator>(const fix16_t other) const { return m_value > other; }
        bool operator>(const double  other) const { return m_value > fix16_from_dbl(other); }
        bool operator>(const float   other) const { return m_value > fix16_from_float(other); }
        bool operator>(const int16_t other) const { return m_value > fix16_from_int(other); }
};

#endif

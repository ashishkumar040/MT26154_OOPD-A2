#pragma once
// Money: fixed-point currency amount stored in minor units.

#include <cstdint>
#include <iosfwd>
#include <string>
#include <stdexcept>

namespace bookmgmt {

class Money {
public:
    Money() = default;

    static Money fromMinor(std::int64_t minor,
                           const std::string& currency = "INR");
    static Money of(std::int64_t major, int minor = 0,
                    const std::string& currency = "INR");

    std::int64_t minorUnits() const { return minor_; }
    const std::string& currency() const { return currency_; }
    double toDouble() const { return static_cast<double>(minor_) / 100.0; }
    std::string toString() const;

    bool isZero() const { return minor_ == 0; }
    bool isNegative() const { return minor_ < 0; }

    Money& operator+=(Money o);
    Money& operator-=(Money o);
    Money& operator*=(std::int64_t k);

    friend Money operator+(Money a, Money b) { return a += b; }
    friend Money operator-(Money a, Money b) { return a -= b; }
    friend Money operator*(Money a, std::int64_t k) { return a *= k; }
    friend Money operator*(std::int64_t k, Money a) { return a *= k; }

    friend bool operator==(Money a, Money b);
    friend bool operator!=(Money a, Money b) { return !(a == b); }
    friend bool operator<(Money a, Money b);
    friend bool operator<=(Money a, Money b) { return a < b || a == b; }
    friend bool operator>(Money a, Money b) { return b < a; }
    friend bool operator>=(Money a, Money b) { return b <= a; }

private:
    Money(std::int64_t minor, std::string currency);
    static void checkCurrency(const std::string& currency);
    void ensureSameCurrency(Money other) const;

    std::int64_t minor_ = 0;
    std::string currency_ = "INR";
};

std::ostream& operator<<(std::ostream& os, Money m);

}  // namespace bookmgmt

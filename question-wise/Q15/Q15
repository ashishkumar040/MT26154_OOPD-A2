#include "bookmgmt/Money.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

Money::Money(std::int64_t minor, std::string currency)
    : minor_(minor), currency_(std::move(currency)) {
    checkCurrency(currency_);
}

void Money::checkCurrency(const std::string& currency) {
    if (currency.size() != 3)
        throw std::invalid_argument("currency code must contain 3 characters");
    for (char c : currency) {
        if (c < 'A' || c > 'Z')
            throw std::invalid_argument("currency code must use uppercase letters");
    }
}

void Money::ensureSameCurrency(Money other) const {
    if (currency_ != other.currency_)
        throw std::invalid_argument("cannot operate on different currencies");
}

Money Money::fromMinor(std::int64_t minor, const std::string& currency) {
    return Money(minor, currency);
}

Money Money::of(std::int64_t major, int minor, const std::string& currency) {
    if (minor < 0 || minor > 99)
        throw std::invalid_argument("minor part must be in 0..99");
    const std::int64_t sign = major < 0 ? -1 : 1;
    return Money(major * 100 + sign * minor, currency);
}

Money& Money::operator+=(Money other) {
    ensureSameCurrency(other);
    minor_ += other.minor_;
    return *this;
}

Money& Money::operator-=(Money other) {
    ensureSameCurrency(other);
    minor_ -= other.minor_;
    return *this;
}

Money& Money::operator*=(std::int64_t k) {
    minor_ *= k;
    return *this;
}

bool operator==(Money a, Money b) {
    a.ensureSameCurrency(b);
    return a.minor_ == b.minor_;
}

bool operator<(Money a, Money b) {
    a.ensureSameCurrency(b);
    return a.minor_ < b.minor_;
}

std::string Money::toString() const {
    const std::int64_t absMinor = minor_ < 0 ? -minor_ : minor_;
    std::string s = std::to_string(absMinor / 100);
    const auto frac = absMinor % 100;
    s += '.';
    if (frac < 10) s += '0';
    s += std::to_string(frac);
    return minor_ < 0 ? "-" + s : s;
}

std::ostream& operator<<(std::ostream& os, Money m) {
    return os << m.toString() << ' ' << m.currency();
}

}  // namespace bookmgmt

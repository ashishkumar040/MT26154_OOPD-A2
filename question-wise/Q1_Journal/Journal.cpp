
#include "bookmgmt/Journal.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

Journal::Journal(std::string id, std::string title, std::string issn,
                 int issuesPerYear, std::string publisher, int year,
                 Money unitPrice, int subscriptionYears)
    : Resource(std::move(id), std::move(title), std::move(publisher),
               year, unitPrice),
      issn_(std::move(issn)),
      issuesPerYear_(issuesPerYear),
      subscriptionYears_(subscriptionYears) {
    if (subscriptionYears_ < 1) {
        throw std::invalid_argument(
            "subscription length must be at least one year");
    }
}

Money Journal::costFor(int quantity) const {
    requirePositive(quantity);
    return unitPrice() *
           (static_cast<std::int64_t>(quantity) * subscriptionYears_);
}

void Journal::printDetails(std::ostream& os) const {
    os << "  ISSN: " << issn_ << "\n"
       << "  issues per year: " << issuesPerYear_ << "\n"
       << "  subscription years: " << subscriptionYears_ << "\n";
}

}  // namespace bookmgmt

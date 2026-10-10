#include "bookmgmt/EBook.h"

#include <ostream>
#include <utility>

namespace bookmgmt {

const char* eBookFormatName(EBookFormat format) {
    switch (format) {
        case EBookFormat::PDF:  return "PDF";
        case EBookFormat::EPUB: return "EPUB";
        case EBookFormat::HTML: return "HTML";
    }
    return "Unknown";
}

EBook::EBook(std::string id, std::string title,
             std::vector<std::string> authors, std::string isbn,
             std::string publisher, int year, Money pricePerSeat,
             std::string accessUrl, EBookFormat format, bool drm,
             LicenseModel license, Money platformFee)
    : ElectronicResource(std::move(id), std::move(title),
                         std::move(publisher), year, pricePerSeat,
                         std::move(accessUrl), license, platformFee),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      format_(format),
      drm_(drm) {}

void EBook::printDetails(std::ostream& os) const {
    ElectronicResource::printDetails(os);
    os << "  authors: " << joinAuthors(authors_) << "\n"
       << "  ISBN: " << isbn_ << "\n"
       << "  format: " << eBookFormatName(format_) << "\n"
       << "  DRM: " << (drm_ ? "Enabled" : "Disabled") << "\n";
}

}  // namespace bookmgmt

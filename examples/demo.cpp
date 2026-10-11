
// Demo: builds a small catalog, sets a budget with per-category quotas,
// and runs a batch of purchase requests through the acquisition manager.

#include <iostream>
#include <string>
#include <vector>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

int main() {
    Catalog catalog;

    catalog.emplace<Book>(
        "B001", "Clean Code",
        std::vector<std::string>{"Robert C. Martin"},
        "978-0132350884", "Prentice Hall", 2008, Money::of(450));

    catalog.emplace<Book>(
        "B002", "The C++ Programming Language",
        std::vector<std::string>{"Bjarne Stroustrup"},
        "978-0321563842", "Addison-Wesley", 2013,
        Money::of(1200), 4, Binding::Hardcover);

    catalog.emplace<ElectronicResource>(
        "R001", "IEEE Xplore Digital Library", "IEEE", 2026,
        Money::of(150), "https://ieeexplore.example",
        LicenseModel::AnnualSubscription, Money::of(2000));

    catalog.emplace<ElectronicResource>(
        "R002", "MATLAB Campus Licence", "MathWorks", 2026,
        Money::of(400), "https://licensing.example/matlab",
        LicenseModel::Perpetual);

    catalog.emplace<Journal>(
        "J001", "IEEE Spectrum", "1234-5678", 12,
        "IEEE", 2025, Money::of(120), 3);

    catalog.emplace<EBook>(
        "EB001", "C++ Programming",
        std::vector<std::string>{"Bjarne Stroustrup", "Another Author"},
        "978-1234567890", "Tech Publisher", 2026,
        Money::of(100), "https://example.com/ebook",
        EBookFormat::PDF, true,
        LicenseModel::AnnualSubscription, Money::of(500));

    catalog.emplace<AudioBook>(
        "A001", "Digital Design Audio", "John Smith", 240,
        "Tech Publisher", 2026, Money::of(100),
        "https://example.com/audio",
        LicenseModel::AnnualSubscription, Money::of(500));

    catalog.emplace<Thesis>(
        "T001", "VLSI Design Research",
        "IIIT Delhi", "M.Tech", "Dr. Sharma",
        "IIIT Delhi", 2026);

    std::cout << "=== Catalog ===\n";
    for (const Resource* r : catalog.all()) {
        std::cout << r->summary() << "\n";
    }

    std::cout << "\n=== Details of R001 ===\n" << catalog.get("R001");
    std::cout << "\n=== Details of J001 ===\n" << catalog.get("J001");

    std::cout << "Cost for 2 copies of J001 for 3 years = "
              << catalog.get("J001").costFor(2) << "\n";

    Budget budget(Money::of(20000));
    budget.setQuota(ResourceCategory::Book, {10, Money::of(8000)});
    budget.setQuota(ResourceCategory::ElectronicResource, {40, Money::of(12000)});
    budget.setQuota(ResourceCategory::Journal, {5, Money::of(2000)});
    budget.setQuota(ResourceCategory::EBook, {20, Money::of(5000)});

    AcquisitionManager acq(catalog, budget);

    std::cout << "\n=== Quotes ===\n";
    std::cout << "5 copies of B002 = " << acq.quote("B002", 5) << "\n";
    std::cout << "20 seats of R001 = " << acq.quote("R001", 20)
              << " (incl. platform fee)\n";

    acq.processBatch({
        {"B001", 4},
        {"B002", 5},
        {"B001", 1},
        {"R001", 20},
        {"R002", 25},
        {"R002", 15},
        {"R002", 5},
        {"J001", 2},
        {"EB001", 2},
        {"X999", 1}
    });

    std::cout << "\n=== Acquisition report ===\n";
    acq.printReport(std::cout);

    std::cout << "\n=== Budget ===\n";
    budget.print(std::cout);

    std::cout << "\n=== Holdings ===\n";
    for (const Resource* r : catalog.all()) {
        std::cout << "  " << r->id() << ": " << catalog.holdings(r->id())
                  << (r->isDigital() ? " seats" : " copies") << "\n";
    }

    std::cout << "\n=== Direct purchase that breaks a quota ===\n";
    try {
        acq.purchase("B002", 1);
    } catch (const QuotaExceededError& e) {
        std::cout << "QuotaExceededError: " << e.what() << "\n";
    }

    // Q4: Hardcover books cost 20% more than their listed price.
    Book hardcover(
        "B-HC", "Hardcover Example", {"Author"},
        "ISBN-HC", "Publisher", 2026,
        Money::of(100), 1, Binding::Hardcover);

    std::cout << "\n=== Q4: Hardcover pricing ===\n";
    std::cout << "Listed price: " << hardcover.unitPrice() << "\n";
    std::cout << "Cost for 2 hardcover copies: "
              << hardcover.costFor(2) << "\n";

    // Q5: Bulk discounts for print and electronic resources.
    std::cout << "\n=== Q5: Bulk discount pricing ===\n";

    Book paperback(
        "B-PB", "Paperback Example", {"Author"},
        "ISBN-PB", "Publisher", 2026, Money::of(100));

    std::cout << "10 paperback copies: "
              << paperback.costFor(10) << "\n";

    std::cout << "10 hardcover copies (surcharge + bulk discount): "
              << hardcover.costFor(10) << "\n";

    ElectronicResource bulkElectronic(
        "R-BULK", "Electronic Database", "Publisher", 2026,
        Money::of(10), "https://example.com",
        LicenseModel::AnnualSubscription, Money::of(100));

    std::cout << "60 electronic seats (incl. platform fee): "
              << bulkElectronic.costFor(60) << "\n";


    return 0;
}

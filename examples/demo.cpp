
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


    // Q13: Search by author, ISBN/ISSN, and publication-year range.
    std::cout << "\n=== Q13: Catalog searches ===\n";

    std::cout << "Author search (Bjarne):\n";
    for (const Resource* r : catalog.searchAuthor("Bjarne"))
        std::cout << r->summary() << "\n";

    std::cout << "ISBN/ISSN search (1234-5678):\n";
    for (const Resource* r : catalog.searchISBNISSN("1234-5678"))
        std::cout << r->summary() << "\n";

    std::cout << "Publication years 2013-2025:\n";
    for (const Resource* r : catalog.searchYearRange(2013, 2025))
        std::cout << r->summary() << "\n";

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

    // Q12: Compare vendors and purchase from the cheapest one.
    acq.addVendorOffer("B001", "Vendor A", Money::of(430));
    acq.addVendorOffer("B001", "Vendor B", Money::of(410));
    acq.addVendorOffer("B001", "Vendor C", Money::of(425));

    std::cout << "\n=== Q12: Vendor comparison ===\n";
    std::cout << "Cheapest vendor for B001: "
              << acq.cheapestVendor("B001") << "\n";
    std::cout << "Quote for 1 copy: " << acq.quote("B001", 1) << "\n";

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


    // Q8: Cancel an approved order, refund the budget, and reduce holdings.
    std::cout << "\n=== Q8: Order cancellation ===\n";

    Catalog cancelCatalog;
    cancelCatalog.emplace<Book>(
        "C001", "Cancellation Demo Book", std::vector<std::string>{"Author"},
        "ISBN-C001", "Publisher", 2026, Money::of(100));

    Budget cancelBudget(Money::of(1000));
    cancelBudget.setQuota(ResourceCategory::Book, {10, Money::of(1000), 5});
    AcquisitionManager cancelAcq(cancelCatalog, cancelBudget, TaxRates{10, 20});

    const auto& approvedOrder = cancelAcq.purchase("C001", 2);
    const int orderNo = approvedOrder.orderNo;

    std::cout << "Order #" << orderNo << " approved; cost: "
              << approvedOrder.cost << "\\n";
    std::cout << "Holdings before cancellation: "
              << cancelCatalog.holdings("C001") << "\\n";

    const auto& cancellation = cancelAcq.cancelOrder(orderNo);
    std::cout << "Cancellation record #" << cancellation.orderNo
              << " for order #" << cancellation.cancelsOrderNo << "\\n";
    std::cout << "Budget spent after refund: " << cancelBudget.spent() << "\\n";
    std::cout << "Holdings after cancellation: "
              << cancelCatalog.holdings("C001") << "\\n";
    std::cout << "History records retained: "
              << cancelAcq.history().size() << "\\n";


    // Q9: Each department has an independent budget and quotas.
    std::cout << "\n=== Q9: Department Budgets ===\n";

    Catalog departmentCatalog;
    departmentCatalog.emplace<Book>(
        "D001", "Computer Science Book", std::vector<std::string>{"Author"},
        "ISBN-D001", "Publisher", 2026, Money::of(100));
    departmentCatalog.emplace<Book>(
        "D002", "Physics Book", std::vector<std::string>{"Author"},
        "ISBN-D002", "Publisher", 2026, Money::of(150));

    Budget sharedBudget(Money::of(10000));
    AcquisitionManager departmentAcq(departmentCatalog, sharedBudget);

    departmentAcq.addDepartment("Computer Science", Money::of(300));
    departmentAcq.addDepartment("Physics", Money::of(500));

    departmentAcq.departmentBudget("Computer Science").setQuota(
        ResourceCategory::Book, {3, Money::of(200), 2});
    departmentAcq.departmentBudget("Physics").setQuota(
        ResourceCategory::Book, {5, Money::of(500), 3});

    const int csOrderNo =
        departmentAcq.purchase("D001", 2, "Computer Science").orderNo;
    std::cout << "Computer Science order approved.\\n";

    const auto& physicsOrder =
        departmentAcq.purchase("D002", 2, "Physics");
    std::cout << "Physics order cost: " << physicsOrder.cost << "\n";

    std::cout << "Computer Science budget spent: "
              << departmentAcq.departmentBudget("Computer Science").spent()
              << "\n";
    std::cout << "Physics budget spent: "
              << departmentAcq.departmentBudget("Physics").spent() << "\n";

    const auto& csCancellation =
        departmentAcq.cancelOrder(csOrderNo);
    std::cout << "Cancelled order department: "
              << csCancellation.department << "\n";
    std::cout << "Computer Science budget after refund: "
              << departmentAcq.departmentBudget("Computer Science").spent()
              << "\n";

    // Q10: Carry forward a configurable percentage of the unspent budget.
    std::cout << "\\n=== Q10: Year-End Budget Rollover ===\\n";

    Budget currentYear(Money::of(1000));
    currentYear.commit(ResourceCategory::Book, 2, Money::of(700));

    Budget nextYear = currentYear.rollover(50.0, Money::of(2000));

    std::cout << "Current year's remaining budget: "
              << currentYear.remaining() << "\\n";
    std::cout << "Carry-forward percentage: 50%\\n";
    std::cout << "Next year's fresh allocation: "
              << Money::of(2000) << "\\n";
    std::cout << "Next year's total budget after rollover: "
              << nextYear.total() << "\\n";


    // Q11: All-or-nothing batch processing.
    std::cout << "\n=== Q11: All-or-Nothing Batch ===\n";

    Catalog batchCatalog;
    batchCatalog.emplace<Book>(
        "A001", "Batch Demo Book", std::vector<std::string>{"Author"},
        "ISBN-A001", "Publisher", 2026, Money::of(100));

    Budget batchBudget(Money::of(250));
    AcquisitionManager batchAcq(batchCatalog, batchBudget);

    const auto acceptedBatch =
        batchAcq.processBatch({{"A001", 1}, {"A001", 1}}, true);

    std::cout << "Valid batch: "
              << (acceptedBatch[0].approved && acceptedBatch[1].approved
                      ? "all purchases approved" : "batch rejected")
              << "\n";
    std::cout << "Budget spent after valid batch: "
              << batchBudget.spent() << "\n";

    const auto historyBefore = batchAcq.history().size();
    const auto rejectedBatch =
        batchAcq.processBatch({{"A001", 1}, {"MISSING", 1}}, true);

    std::cout << "Batch containing a missing resource: "
              << (rejectedBatch[0].approved || rejectedBatch[1].approved
                      ? "unexpected approval" : "entire batch rejected")
              << "\n";
    std::cout << "Budget spent after rejected batch: "
              << batchBudget.spent() << "\n";
    std::cout << "History unchanged after rejection: "
              << (batchAcq.history().size() == historyBefore ? "yes" : "no")
              << "\n";


    // Q14: Print lending and electronic-resource sessions.
    std::cout << "\n=== Q14: Lending ===\n";

    Catalog lendingCatalog;
    lendingCatalog.emplace<Book>(
        "L001", "Lending Demo Book", std::vector<std::string>{"Demo Author"},
        "ISBN-L001", "Demo Publisher", 2026, Money::of(100));
    lendingCatalog.emplace<ElectronicResource>(
        "L002", "Lending Demo Database", "Demo Publisher", 2026,
        Money::of(50), "https://example.com");

    lendingCatalog.addHoldings("L001", 2);
    lendingCatalog.addHoldings("L002", 2);

    LendingManager lending(lendingCatalog);
    lending.borrow("L001", "P001");
    std::cout << "Print copies borrowed: "
              << lending.borrowedCount("L001") << "\n";

    lending.returnCopy("L001", "P001");
    std::cout << "Print copies borrowed after return: "
              << lending.borrowedCount("L001") << "\n";

    lending.openSession("L002", "P001");
    lending.openSession("L002", "P002");
    std::cout << "Active electronic sessions: "
              << lending.activeSessions("L002") << "\n";

    lending.closeSession("L002", "P001");
    std::cout << "Active sessions after closing one: "
              << lending.activeSessions("L002") << "\n";

    // Q15: Currency-aware Money operations.
    std::cout << "\\n=== Q15: Money currencies ===\\n";
    Money inr = Money::of(100, 0, "INR");
    Money usd = Money::of(100, 0, "USD");
    std::cout << "INR amount: " << inr << "\\n";
    std::cout << "USD amount: " << usd << "\\n";
    try {
        (void)(inr + usd);
    } catch (const std::invalid_argument& e) {
        std::cout << "Mixed-currency addition rejected: " << e.what() << "\\n";
    }

    return 0;
}

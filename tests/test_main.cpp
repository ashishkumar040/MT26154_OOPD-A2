// Minimal self-contained test runner (no external framework needed).
#include "bookmgmt/EBook.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failures;                                                        \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond \
                      << "\n";                                                   \
        }                                                                        \
    } while (0)

#define CHECK_THROWS(expr, ExType)            \
    do {                                      \
        bool thrown_ = false;                 \
        try {                                 \
            (void)(expr);                     \
        } catch (const ExType&) {             \
            thrown_ = true;                   \
        } catch (...) {                       \
        }                                     \
        CHECK(thrown_ && "expected " #ExType); \
    } while (0)

static void testMoney() {
    CHECK(Money::of(12, 5).toString() == "12.05");
    CHECK(Money::of(-3, 50).toString() == "-3.50");
    CHECK(Money::fromMinor(7).toString() == "0.07");
    CHECK(Money::of(10) + Money::of(0, 50) == Money::fromMinor(1050));
    CHECK(Money::of(3) * 4 == Money::of(12));
    CHECK(Money::of(1) < Money::of(2));
    CHECK_THROWS(Money::of(1, 100), std::invalid_argument);
}

static void testResourcesAndCost() {
    Book b("B1", "T", {"A", "B", "C"}, "isbn", "P", 2020, Money::of(100));
    CHECK(b.category() == ResourceCategory::Book);
    CHECK(!b.isDigital());
    CHECK(b.costFor(3) == Money::of(300));
    // Q4: Hardcover books cost 20% more than listed price.
Book hardcover("B2", "Hardcover Book", {"Author"}, "isbn2",
               "Publisher", 2026, Money::of(100), 1,
               Binding::Hardcover);

CHECK(hardcover.costFor(1) == Money::of(120));
CHECK(hardcover.costFor(2) == Money::of(240));
CHECK(b.costFor(2) == Money::of(200));
// Q5: 10% bulk discount for 10 or more print copies.
CHECK(b.costFor(10) == Money::of(900));

// Q4 + Q5: Hardcover surcharge followed by bulk discount.
CHECK(hardcover.costFor(10) == Money::of(1080));  // Paperback stays at listed price.
CHECK_THROWS(hardcover.costFor(0), std::invalid_argument);
    CHECK_THROWS(b.costFor(0), std::invalid_argument);
    CHECK(joinAuthors(b.authors()) == "A, B and C");

    ElectronicResource e("R1", "DB", "P", 2026, Money::of(10), "url",
                         LicenseModel::AnnualSubscription, Money::of(100));
    CHECK(e.isDigital());
    CHECK(e.costFor(5) == Money::of(150));
    // Q5: First 50 seats full price; remaining seats half price.
CHECK(e.costFor(60) == Money::of(650));

    CHECK(e.category() == ResourceCategory::ElectronicResource);

    // Polymorphism through a base-class reference
    const Resource& r = e;
    CHECK(r.costFor(1) == Money::of(110));
    std::ostringstream os;
    os << r;
    CHECK(os.str().find("platform fee: 100.00") != std::string::npos);

    CHECK_THROWS(Book("", "T", {}, "", "", 2000, Money::of(1)), std::invalid_argument);
    CHECK_THROWS(Book("B", "T", {}, "", "", 2000, Money::fromMinor(-1)),
                 std::invalid_argument);
}
              
static void testJournal() {
    Journal j("J1", "IEEE Spectrum", "1234-5678", 12,
              "IEEE", 2025, Money::of(120), 3);

    CHECK(j.category() == ResourceCategory::Journal);
    CHECK(j.issn() == "1234-5678");
    CHECK(j.issuesPerYear() == 12);
    CHECK(j.subscriptionYears() == 3);

    // Annual price × copies × subscription years
    CHECK(j.costFor(2) == Money::of(720));
    // Q5: Bulk discount applies to 10 or more journal subscriptions.
CHECK(j.costFor(10) == Money::of(3240));
    CHECK_THROWS(j.costFor(0), std::invalid_argument);

    // Subscription duration defaults to one year.
    Journal oneYear("J2", "Science Today", "9876-5432", 6,
                    "Publisher", 2026, Money::of(50));
    CHECK(oneYear.subscriptionYears() == 1);
    CHECK(oneYear.costFor(2) == Money::of(100));

    // Invalid subscription durations must be rejected.
    CHECK_THROWS(
        Journal("J3", "Invalid", "1111-2222", 12, "P", 2026,
                Money::of(100), 0),
        std::invalid_argument);

    CHECK_THROWS(
        Journal("J4", "Invalid", "1111-2222", 12, "P", 2026,
                Money::of(100), -1),
        std::invalid_argument);

    // Printed details must include the Journal-specific fields.
    std::ostringstream os;
    os << j;
    CHECK(os.str().find("ISSN: 1234-5678") != std::string::npos);
    CHECK(os.str().find("issues per year: 12") != std::string::npos);
    CHECK(os.str().find("subscription years: 3") != std::string::npos);
}
   
static void testEBook() {
    EBook ebook("EB1", "C++ Programming",
                {"Bjarne Stroustrup", "Another Author"},
                "978-1234567890", "Tech Publisher", 2026,
                Money::of(100), "https://example.com/ebook",
                EBookFormat::PDF, true,
                LicenseModel::AnnualSubscription, Money::of(500));

    CHECK(ebook.category() == ResourceCategory::EBook);
    CHECK(ebook.isDigital());
    CHECK(ebook.authors().size() == 2);
    CHECK(ebook.isbn() == "978-1234567890");
    CHECK(ebook.format() == EBookFormat::PDF);
    CHECK(ebook.drmEnabled());
    CHECK(ebook.costFor(5) == Money::of(1000));

    EBook epub("EB2", "Digital Systems", {"Author"},
               "978-0000000000", "Publisher", 2025,
               Money::of(80), "https://example.com/epub",
               EBookFormat::EPUB, false);

    CHECK(epub.format() == EBookFormat::EPUB);
    CHECK(!epub.drmEnabled());
    CHECK(epub.costFor(2) == Money::of(160));

    std::ostringstream os;
    os << ebook;
    


    CHECK(os.str().find("authors: Bjarne Stroustrup and Another Author")
      != std::string::npos);
          
    CHECK(os.str().find("ISBN: 978-1234567890") != std::string::npos);
    CHECK(os.str().find("format: PDF") != std::string::npos);
    CHECK(os.str().find("DRM: Enabled") != std::string::npos);
}

static void testAudioBook() {
    AudioBook audio(
        "A1", "Digital Design Audio", "John Smith", 240,
        "Tech Publisher", 2026, Money::of(100),
        "https://example.com/audio",
        LicenseModel::AnnualSubscription, Money::of(500));

    CHECK(audio.category() == ResourceCategory::AudioBook);
    CHECK(audio.isDigital());
    CHECK(audio.narrator() == "John Smith");
    CHECK(audio.durationMinutes() == 240);
    CHECK(audio.costFor(2) == Money::of(700));

    std::ostringstream os;
    os << audio;

    CHECK(os.str().find("narrator: John Smith") != std::string::npos);
    CHECK(os.str().find("duration (minutes): 240") != std::string::npos);
}

static void testThesis() {
    Thesis thesis(
        "T1", "VLSI Design Research",
        "IIIT Delhi", "M.Tech", "Dr. Sharma",
        "IIIT Delhi", 2026);

    CHECK(thesis.category() == ResourceCategory::Thesis);
    CHECK(!thesis.isDigital());
    CHECK(thesis.university() == "IIIT Delhi");
    CHECK(thesis.degree() == "M.Tech");
    CHECK(thesis.supervisor() == "Dr. Sharma");
    CHECK(thesis.costFor(2) == Money::of(0));

    std::ostringstream os;
    os << thesis;

    CHECK(os.str().find("university: IIIT Delhi") != std::string::npos);
    CHECK(os.str().find("degree: M.Tech") != std::string::npos);
    CHECK(os.str().find("supervisor: Dr. Sharma") != std::string::npos);
}





static void testCatalog() {
    Catalog c;
    c.emplace<Book>("B1", "Clean Code", std::vector<std::string>{"M"}, "i", "P", 2008,
                    Money::of(1));
    c.emplace<Book>("B2", "Clean Architecture", std::vector<std::string>{"M"}, "i", "P",
                    2017, Money::of(1));
    c.emplace<ElectronicResource>("R1", "ACM Digital Library", "ACM", 2026, Money::of(1),
                                  "url");

    CHECK(c.size() == 3);
    CHECK(c.contains("B1"));
    CHECK(c.find("nope") == nullptr);
    CHECK_THROWS(c.get("nope"), NotFoundError);
    CHECK_THROWS(c.emplace<Book>("B1", "dup", std::vector<std::string>{}, "", "", 1,
                                 Money::of(1)),
                 DuplicateIdError);

    CHECK(c.searchTitle("clean").size() == 2);
    CHECK(c.byCategory(ResourceCategory::ElectronicResource).size() == 1);
    CHECK(c.where([](const Resource& r) { return r.isDigital(); }).size() == 1);

    CHECK(c.holdings("B1") == 0);
    c.addHoldings("B1", 3);
    CHECK(c.holdings("B1") == 3);
    CHECK_THROWS(c.addHoldings("B1", -5), std::invalid_argument);

    c.remove("R1");
    CHECK(c.size() == 2);
    CHECK_THROWS(c.remove("R1"), NotFoundError);
}

static void testBudget() {
    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {5, Money::of(400)});

    CHECK(b.check(ResourceCategory::Book, 2, Money::of(200)).empty());
    CHECK(!b.check(ResourceCategory::Book, 6, Money::of(10)).empty());   // units
    CHECK(!b.check(ResourceCategory::Book, 1, Money::of(401)).empty());  // spend
    CHECK(!b.check(ResourceCategory::ElectronicResource, 1, Money::of(1001)).empty());  // overall
    CHECK(b.check(ResourceCategory::ElectronicResource, 1, Money::of(900)).empty());    // no quota

    b.commit(ResourceCategory::Book, 4, Money::of(300));
    CHECK(b.spent() == Money::of(300));
    CHECK(*b.unitsRemaining(ResourceCategory::Book) == 1);
    CHECK(*b.spendRemaining(ResourceCategory::Book) == Money::of(100));
    CHECK(!b.unitsRemaining(ResourceCategory::ElectronicResource).has_value());

    CHECK_THROWS(b.commit(ResourceCategory::Book, 2, Money::of(10)), QuotaExceededError);
    CHECK_THROWS(b.commit(ResourceCategory::ElectronicResource, 1, Money::of(800)),
                 BudgetExceededError);
    CHECK_THROWS(b.commit(ResourceCategory::ElectronicResource, 0, Money::of(1)),
                 std::invalid_argument);
    CHECK(b.spent() == Money::of(300));  // failed commits changed nothing
}

static void testAcquisition() {
    Catalog c;
    c.emplace<Book>("B1", "Book", std::vector<std::string>{"A"}, "i", "P", 2020,
                    Money::of(100));
    c.emplace<ElectronicResource>("R1", "DB", "P", 2026, Money::of(10), "url",
                                  LicenseModel::AnnualSubscription, Money::of(50));
    Budget b(Money::of(500));
    b.setQuota(ResourceCategory::Book, {3, Money::of(1000)});
    AcquisitionManager acq(c, b);

    CHECK(acq.quote("R1", 5) == Money::of(100));
    std::string why;
    CHECK(acq.canPurchase("B1", 3, &why) && why.empty());
    CHECK(!acq.canPurchase("B1", 4, &why) && !why.empty());
    CHECK(!acq.canPurchase("nope", 1, &why));

    const auto& rec = acq.purchase("B1", 2);
    CHECK(rec.approved && rec.cost == Money::of(200) && rec.orderNo == 1);
    CHECK(c.holdings("B1") == 2);

    CHECK_THROWS(acq.purchase("B1", 2), QuotaExceededError);
    CHECK_THROWS(acq.purchase("nope", 1), NotFoundError);
    CHECK(acq.history().size() == 1);  // exceptions don't record

    auto res = acq.processBatch({{"R1", 10}, {"R1", 100}, {"B1", 1}, {"zzz", 1}, {"B1", 0}});
    CHECK(res.size() == 5);
    CHECK(res[0].approved && res[0].cost == Money::of(150));
    CHECK(!res[1].approved);  // 1050 > remaining 150
    CHECK(res[2].approved);
    CHECK(!res[3].approved && res[3].reason.find("not found") != std::string::npos);
    CHECK(!res[4].approved);
    CHECK(acq.totalSpent() == Money::of(450));
    CHECK(b.spent() == acq.totalSpent());
    CHECK(c.holdings("R1") == 10 && c.holdings("B1") == 3);
    CHECK(acq.history().size() == 6);
}



static void testAllOrNothingBatch() {
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i", "P", 2021, Money::of(100));

    Budget b(Money::of(250));
    AcquisitionManager acq(c, b);

    // A valid all-or-nothing batch buys every item.
    auto successful = acq.processBatch({{"B1", 1}, {"B2", 1}}, true);
    CHECK(successful.size() == 2);
    CHECK(successful[0].approved && successful[1].approved);
    CHECK(b.spent() == Money::of(200));
    CHECK(c.holdings("B1") == 1 && c.holdings("B2") == 1);
    CHECK(acq.history().size() == 2);

    // If any request fails, nothing in this batch is purchased or recorded.
    const Money spentBefore = b.spent();
    const auto historyBefore = acq.history().size();
    const int b1Before = c.holdings("B1");
    const int b2Before = c.holdings("B2");

    auto rejected = acq.processBatch({{"B1", 1}, {"missing", 1}}, true);
    CHECK(rejected.size() == 2);
    CHECK(!rejected[0].approved && !rejected[1].approved);
    CHECK(rejected[0].reason.find("batch rejected") != std::string::npos);
    CHECK(b.spent() == spentBefore);
    CHECK(c.holdings("B1") == b1Before && c.holdings("B2") == b2Before);
    CHECK(acq.history().size() == historyBefore);

    // Cumulative costs are validated, not just each request independently.
    auto overBudget = acq.processBatch({{"B1", 1}, {"B2", 1}}, true);
    CHECK(overBudget.size() == 2);
    CHECK(!overBudget[0].approved && !overBudget[1].approved);
    CHECK(b.spent() == spentBefore);
    CHECK(c.holdings("B1") == b1Before && c.holdings("B2") == b2Before);
    CHECK(acq.history().size() == historyBefore);
}

static void testTitleQuota() {
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i", "P", 2021, Money::of(100));

    Budget b(Money::of(500));
    b.setQuota(ResourceCategory::Book, {10, Money::of(500), 1});
    AcquisitionManager acq(c, b);

    CHECK(acq.canPurchase("B1", 1));
    acq.purchase("B1", 1);

    // Buying the same title again must not use another title slot.
    CHECK(acq.canPurchase("B1", 1));
    acq.purchase("B1", 1);

    // A different title exceeds the one-title category limit.
    CHECK(!acq.canPurchase("B2", 1));
    CHECK_THROWS(acq.purchase("B2", 1), QuotaExceededError);

    CHECK(c.holdings("B1") == 2);
    CHECK(c.holdings("B2") == 0);
    CHECK(b.spent() == Money::of(200));
}


static void testOrderCancellation() {
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i", "P", 2021, Money::of(100));

    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {10, Money::of(1000), 1});
    AcquisitionManager acq(c, b, TaxRates{10, 20});

    // Two orders for the same title share one distinct-title quota slot.
    const int firstOrder = acq.purchase("B1", 2).orderNo;
    acq.purchase("B1", 1);

    CHECK(c.holdings("B1") == 3);
    CHECK(b.spent() == Money::of(330));
    CHECK(acq.totalSpent() == Money::of(330));

    // Cancelling one order refunds its post-tax cost and units.
    const auto& cancellation = acq.cancelOrder(firstOrder);
    CHECK(cancellation.cancellation);
    CHECK(cancellation.cancelsOrderNo == firstOrder);
    CHECK(c.holdings("B1") == 1);
    CHECK(b.spent() == Money::of(110));
    CHECK(b.usageFor(ResourceCategory::Book).units == 1);
    CHECK(acq.totalSpent() == Money::of(110));
    CHECK(acq.history().size() == 3);
    CHECK(acq.history()[0].approved && !acq.history()[0].cancellation);
    CHECK(acq.history()[0].orderNo == firstOrder);

    // The title slot must remain occupied while another B1 order is active.
    CHECK(!acq.canPurchase("B2", 1));

    // A second cancellation of the same order must fail without side effects.
    CHECK_THROWS(acq.cancelOrder(firstOrder), std::invalid_argument);
    CHECK(c.holdings("B1") == 1);
    CHECK(b.spent() == Money::of(110));

    // Cancelling the last active B1 order releases the title slot.
    const int secondOrder = acq.history()[1].orderNo;
    acq.cancelOrder(secondOrder);
    CHECK(c.holdings("B1") == 0);
    CHECK(b.spent() == Money::of(0));
    CHECK(acq.totalSpent() == Money::of(0));
    CHECK(b.usageFor(ResourceCategory::Book).units == 0);
    CHECK(b.usageFor(ResourceCategory::Book).titles.empty());
    CHECK(acq.canPurchase("B2", 1));

    // Rejected and nonexistent orders cannot be cancelled.
    const auto rejected = acq.processBatch({{"B2", 100}}).front();
    CHECK(!rejected.approved);
    CHECK_THROWS(acq.cancelOrder(rejected.orderNo), std::invalid_argument);
    CHECK_THROWS(acq.cancelOrder(9999), std::invalid_argument);
}


static void testYearEndRollover() {
    Budget thisYear(Money::of(1000));
    thisYear.commit(ResourceCategory::Book, 2, Money::of(700));

    Budget nextYear = thisYear.rollover(50.0, Money::of(2000));
    CHECK(nextYear.total() == Money::of(2150));
    CHECK(nextYear.spent() == Money::of(0));
    CHECK(nextYear.remaining() == Money::of(2150));

    Budget noCarry = thisYear.rollover(0.0, Money::of(2000));
    CHECK(noCarry.total() == Money::of(2000));

    Budget fullCarry = thisYear.rollover(100.0, Money::of(2000));
    CHECK(fullCarry.total() == Money::of(2300));

    Budget fractional = thisYear.rollover(33.0, Money::of(2000));
    CHECK(fractional.total() == Money::of(2099, 0)); // 33% of 300 = 99

    CHECK_THROWS(thisYear.rollover(-1.0), std::invalid_argument);
    CHECK_THROWS(thisYear.rollover(101.0), std::invalid_argument);
    CHECK_THROWS(thisYear.rollover(50.0, Money::of(-1)), std::invalid_argument);

    // Rollover must not modify this year's budget.
    CHECK(thisYear.spent() == Money::of(700));
    CHECK(thisYear.remaining() == Money::of(300));
}

static void testDepartmentBudgets() {
    Catalog c;
    c.emplace<Book>("B1", "Book One", std::vector<std::string>{"A"},
                    "i", "P", 2020, Money::of(100));
    c.emplace<Book>("B2", "Book Two", std::vector<std::string>{"B"},
                    "i", "P", 2021, Money::of(100));

    Budget sharedBudget(Money::of(10000));
    AcquisitionManager acq(c, sharedBudget, TaxRates{});

    acq.addDepartment("Computer Science", Money::of(300));
    acq.addDepartment("Physics", Money::of(500));

    acq.departmentBudget("Computer Science").setQuota(
        ResourceCategory::Book, {3, Money::of(200), 1});
    acq.departmentBudget("Physics").setQuota(
        ResourceCategory::Book, {5, Money::of(500), 3});

    std::string why;
    CHECK(acq.canPurchase("B1", 2, "Computer Science", &why));
    CHECK(why.empty());
    CHECK(!acq.canPurchase("B1", 3, "Computer Science", &why));
    CHECK(!acq.canPurchase("B1", 1, "Unknown Department", &why));
    CHECK(why.find("department not found") != std::string::npos);
    CHECK_THROWS(acq.departmentBudget("Unknown Department"),
                 std::invalid_argument);
    CHECK_THROWS(acq.addDepartment("Physics", Money::of(100)),
                 std::invalid_argument);

    const int csOrder =
        acq.purchase("B1", 2, "Computer Science").orderNo;
    CHECK(acq.history().back().department == "Computer Science");
    CHECK(acq.departmentBudget("Computer Science").spent() == Money::of(200));
    CHECK(acq.departmentBudget("Physics").spent() == Money::of(0));
    CHECK(sharedBudget.spent() == Money::of(0));

    // Physics has its own budget and title quota.
    CHECK(acq.canPurchase("B2", 2, "Physics"));
    acq.purchase("B2", 2, "Physics");
    CHECK(acq.departmentBudget("Physics").spent() == Money::of(200));
    CHECK(c.holdings("B1") == 2 && c.holdings("B2") == 2);

    // A department-specific batch request uses that department's budget.
    const auto batch = acq.processBatch({
        {"B1", 1, "Computer Science"},
        {"B2", 1, "Unknown Department"}
    });
    CHECK(batch.size() == 2);
    CHECK(!batch[0].approved);  // CS has reached its title quota.
    CHECK(!batch[1].approved);
    CHECK(batch[1].department == "Unknown Department");
    CHECK(batch[1].reason.find("department not found") != std::string::npos);

    // Cancelling the CS order refunds the CS budget, not the shared budget.
    const auto& cancellation = acq.cancelOrder(csOrder);
    CHECK(cancellation.cancellation);
    CHECK(cancellation.department == "Computer Science");
    CHECK(acq.departmentBudget("Computer Science").spent() == Money::of(0));
    CHECK(acq.departmentBudget("Physics").spent() == Money::of(200));
    CHECK(sharedBudget.spent() == Money::of(0));
    CHECK(c.holdings("B1") == 0);
    CHECK(acq.canPurchase("B2", 1, "Computer Science"));
}

int main() {
    testMoney();
    testResourcesAndCost();
    testJournal();
    testEBook();
    testCatalog();
    testBudget();
    testTitleQuota();
    testOrderCancellation();
    testYearEndRollover();
    testDepartmentBudgets();
    testAllOrNothingBatch();
    testAcquisition();
    testAudioBook();
    testThesis();

    std::cout << (g_checks - g_failures) << "/" << g_checks << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}


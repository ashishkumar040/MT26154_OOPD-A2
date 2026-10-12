// Minimal self-contained test runner (no external framework needed).
#include "bookmgmt/EBook.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;



template <typename Exception, typename Callable>
static bool throwsAs(Callable&& callable) {
    try {
        callable();
    } catch (const Exception&) {
        return true;
    } catch (...) {
    }
    return false;
}

static void testMoney() {
    assert(Money::of(12, 5).toString() == "12.05");
    assert(Money::of(-3, 50).toString() == "-3.50");
    assert(Money::fromMinor(7).toString() == "0.07");
    assert(Money::of(10) + Money::of(0, 50) == Money::fromMinor(1050));
    assert(Money::of(3) * 4 == Money::of(12));
    assert(Money::of(1) < Money::of(2));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 100)); }));
    assert(Money::of(10, 0, "USD").currency() == "USD");
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 0, "USD") + Money::of(1, 0, "INR")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 0, "USD") - Money::of(1, 0, "INR")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 0, "USD") == Money::of(1, 0, "INR")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 0, "USD") < Money::of(2, 0, "INR")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Money::of(1, 0, "usd")); }));
}

static void testResourcesAndCost() {
    Book b("B1", "T", {"A", "B", "C"}, "isbn", "P", 2020, Money::of(100));
    assert(b.category() == ResourceCategory::Book);
    assert(!b.isDigital());
    assert(b.costFor(3) == Money::of(300));
    // Q4: Hardcover books cost 20% more than listed price.
Book hardcover("B2", "Hardcover Book", {"Author"}, "isbn2",
               "Publisher", 2026, Money::of(100), 1,
               Binding::Hardcover);

assert(hardcover.costFor(1) == Money::of(120));
assert(hardcover.costFor(2) == Money::of(240));
assert(b.costFor(2) == Money::of(200));
// Q5: 10% bulk discount for 10 or more print copies.
assert(b.costFor(10) == Money::of(900));

// Q4 + Q5: Hardcover surcharge followed by bulk discount.
assert(hardcover.costFor(10) == Money::of(1080));  // Paperback stays at listed price.
assert(throwsAs<std::invalid_argument>([&]() { (void)(hardcover.costFor(0)); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(b.costFor(0)); }));
    assert(joinAuthors(b.authors()) == "A, B and C");

    ElectronicResource e("R1", "DB", "P", 2026, Money::of(10), "url",
                         LicenseModel::AnnualSubscription, Money::of(100));
    assert(e.isDigital());
    assert(e.costFor(5) == Money::of(150));
    // Q5: First 50 seats full price; remaining seats half price.
assert(e.costFor(60) == Money::of(650));

    assert(e.category() == ResourceCategory::ElectronicResource);

    // Polymorphism through a base-class reference
    const Resource& r = e;
    assert(r.costFor(1) == Money::of(110));
    std::ostringstream os;
    os << r;
    assert(os.str().find("platform fee: 100.00") != std::string::npos);

    assert(throwsAs<std::invalid_argument>([&]() { (void)(Book("", "T", {}, "", "", 2000, Money::of(1))); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(Book("B", "T", {}, "", "", 2000, Money::fromMinor(-1))); }));
}
              
static void testJournal() {
    Journal j("J1", "IEEE Spectrum", "1234-5678", 12,
              "IEEE", 2025, Money::of(120), 3);

    assert(j.category() == ResourceCategory::Journal);
    assert(j.issn() == "1234-5678");
    assert(j.issuesPerYear() == 12);
    assert(j.subscriptionYears() == 3);

    // Annual price × copies × subscription years
    assert(j.costFor(2) == Money::of(720));
    // Q5: Bulk discount applies to 10 or more journal subscriptions.
assert(j.costFor(10) == Money::of(3240));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(j.costFor(0)); }));

    // Subscription duration defaults to one year.
    Journal oneYear("J2", "Science Today", "9876-5432", 6,
                    "Publisher", 2026, Money::of(50));
    assert(oneYear.subscriptionYears() == 1);
    assert(oneYear.costFor(2) == Money::of(100));

    // Invalid subscription durations must be rejected.
    assert(throwsAs<std::invalid_argument>([&]() { (void)(
        Journal("J3", "Invalid", "1111-2222", 12, "P", 2026,
                Money::of(100), 0)); }));

    assert(throwsAs<std::invalid_argument>([&]() { (void)(
        Journal("J4", "Invalid", "1111-2222", 12, "P", 2026,
                Money::of(100), -1)); }));

    // Printed details must include the Journal-specific fields.
    std::ostringstream os;
    os << j;
    assert(os.str().find("ISSN: 1234-5678") != std::string::npos);
    assert(os.str().find("issues per year: 12") != std::string::npos);
    assert(os.str().find("subscription years: 3") != std::string::npos);
}
   
static void testEBook() {
    EBook ebook("EB1", "C++ Programming",
                {"Bjarne Stroustrup", "Another Author"},
                "978-1234567890", "Tech Publisher", 2026,
                Money::of(100), "https://example.com/ebook",
                EBookFormat::PDF, true,
                LicenseModel::AnnualSubscription, Money::of(500));

    assert(ebook.category() == ResourceCategory::EBook);
    assert(ebook.isDigital());
    assert(ebook.authors().size() == 2);
    assert(ebook.isbn() == "978-1234567890");
    assert(ebook.format() == EBookFormat::PDF);
    assert(ebook.drmEnabled());
    assert(ebook.costFor(5) == Money::of(1000));

    EBook epub("EB2", "Digital Systems", {"Author"},
               "978-0000000000", "Publisher", 2025,
               Money::of(80), "https://example.com/epub",
               EBookFormat::EPUB, false);

    assert(epub.format() == EBookFormat::EPUB);
    assert(!epub.drmEnabled());
    assert(epub.costFor(2) == Money::of(160));

    std::ostringstream os;
    os << ebook;
    


    assert(os.str().find("authors: Bjarne Stroustrup and Another Author")
      != std::string::npos);
          
    assert(os.str().find("ISBN: 978-1234567890") != std::string::npos);
    assert(os.str().find("format: PDF") != std::string::npos);
    assert(os.str().find("DRM: Enabled") != std::string::npos);
}

static void testAudioBook() {
    AudioBook audio(
        "A1", "Digital Design Audio", "John Smith", 240,
        "Tech Publisher", 2026, Money::of(100),
        "https://example.com/audio",
        LicenseModel::AnnualSubscription, Money::of(500));

    assert(audio.category() == ResourceCategory::AudioBook);
    assert(audio.isDigital());
    assert(audio.narrator() == "John Smith");
    assert(audio.durationMinutes() == 240);
    assert(audio.costFor(2) == Money::of(700));

    std::ostringstream os;
    os << audio;

    assert(os.str().find("narrator: John Smith") != std::string::npos);
    assert(os.str().find("duration (minutes): 240") != std::string::npos);
}

static void testThesis() {
    Thesis thesis(
        "T1", "VLSI Design Research",
        "IIIT Delhi", "M.Tech", "Dr. Sharma",
        "IIIT Delhi", 2026);

    assert(thesis.category() == ResourceCategory::Thesis);
    assert(!thesis.isDigital());
    assert(thesis.university() == "IIIT Delhi");
    assert(thesis.degree() == "M.Tech");
    assert(thesis.supervisor() == "Dr. Sharma");
    assert(thesis.costFor(2) == Money::of(0));

    std::ostringstream os;
    os << thesis;

    assert(os.str().find("university: IIIT Delhi") != std::string::npos);
    assert(os.str().find("degree: M.Tech") != std::string::npos);
    assert(os.str().find("supervisor: Dr. Sharma") != std::string::npos);
}





static void testCatalog() {
    Catalog c;
    c.emplace<Book>("B1", "Clean Code", std::vector<std::string>{"M"}, "i", "P", 2008,
                    Money::of(1));
    c.emplace<Book>("B2", "Clean Architecture", std::vector<std::string>{"M"}, "i", "P",
                    2017, Money::of(1));
    c.emplace<ElectronicResource>("R1", "ACM Digital Library", "ACM", 2026, Money::of(1),
                                  "url");

    assert(c.size() == 3);
    assert(c.contains("B1"));
    assert(c.find("nope") == nullptr);
    assert(throwsAs<NotFoundError>([&]() { (void)(c.get("nope")); }));
    assert(throwsAs<DuplicateIdError>([&]() { (void)(c.emplace<Book>("B1", "dup", std::vector<std::string>{}, "", "", 1,
                                 Money::of(1))); }));

    assert(c.searchTitle("clean").size() == 2);

    // Q13: Catalog searches by author, ISBN/ISSN, and publication year.
    c.emplace<Book>("B3", "Effective C++",
                    std::vector<std::string>{"Scott Meyers"},
                    "978-0321334879", "Publisher", 2005, Money::of(1));
    c.emplace<EBook>("E1", "Modern C++",
                     std::vector<std::string>{"Scott Meyers", "Jane Doe"},
                     "978-1111111111", "Publisher", 2020, Money::of(1),
                     "https://example.com", EBookFormat::PDF, false);
    c.emplace<Journal>("J1", "Computing Journal", "9876-5432", 12,
                       "Publisher", 2017, Money::of(1));

    assert(c.searchAuthor("scott").size() == 2);
    assert(c.searchAuthor("JANE").size() == 1);
    assert(c.searchAuthor("unknown").empty());

    assert(c.searchISBNISSN("978-0321334879").size() == 1);
    assert(c.searchISBNISSN("9876-5432").size() == 1);
    assert(c.searchISBNISSN("978-111").size() == 1);
    assert(c.searchISBNISSN("unknown").empty());

    assert(c.searchYearRange(2005, 2005).size() == 1);
    assert(c.searchYearRange(2017, 2020).size() == 3);
    assert(c.searchYearRange(2021, 2025).empty());
    assert(throwsAs<std::invalid_argument>([&]() { (void)(c.searchYearRange(2020, 2010)); }));

    assert(c.byCategory(ResourceCategory::ElectronicResource).size() == 1);
    assert(c.where([](const Resource& r) { return r.isDigital(); }).size() == 2);

    assert(c.holdings("B1") == 0);
    c.addHoldings("B1", 3);
    assert(c.holdings("B1") == 3);
    assert(throwsAs<std::invalid_argument>([&]() { (void)(c.addHoldings("B1", -5)); }));

    c.remove("R1");
    assert(c.size() == 5);
    assert(throwsAs<NotFoundError>([&]() { (void)(c.remove("R1")); }));
}

static void testBudget() {
    Budget b(Money::of(1000));
    b.setQuota(ResourceCategory::Book, {5, Money::of(400)});

    assert(b.check(ResourceCategory::Book, 2, Money::of(200)).empty());
    assert(!b.check(ResourceCategory::Book, 6, Money::of(10)).empty());   // units
    assert(!b.check(ResourceCategory::Book, 1, Money::of(401)).empty());  // spend
    assert(!b.check(ResourceCategory::ElectronicResource, 1, Money::of(1001)).empty());  // overall
    assert(b.check(ResourceCategory::ElectronicResource, 1, Money::of(900)).empty());    // no quota

    b.commit(ResourceCategory::Book, 4, Money::of(300));
    assert(b.spent() == Money::of(300));
    assert(*b.unitsRemaining(ResourceCategory::Book) == 1);
    assert(*b.spendRemaining(ResourceCategory::Book) == Money::of(100));
    assert(!b.unitsRemaining(ResourceCategory::ElectronicResource).has_value());

    assert(throwsAs<QuotaExceededError>([&]() { (void)(b.commit(ResourceCategory::Book, 2, Money::of(10))); }));
    assert(throwsAs<BudgetExceededError>([&]() { (void)(b.commit(ResourceCategory::ElectronicResource, 1, Money::of(800))); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(b.commit(ResourceCategory::ElectronicResource, 0, Money::of(1))); }));
    assert(b.spent() == Money::of(300));  // failed commits changed nothing
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

    assert(acq.quote("R1", 5) == Money::of(100));
    std::string why;
    assert(acq.canPurchase("B1", 3, &why) && why.empty());
    assert(!acq.canPurchase("B1", 4, &why) && !why.empty());
    assert(!acq.canPurchase("nope", 1, &why));

    const auto& rec = acq.purchase("B1", 2);
    assert(rec.approved && rec.cost == Money::of(200) && rec.orderNo == 1);
    assert(c.holdings("B1") == 2);

    assert(throwsAs<QuotaExceededError>([&]() { (void)(acq.purchase("B1", 2)); }));
    assert(throwsAs<NotFoundError>([&]() { (void)(acq.purchase("nope", 1)); }));
    assert(acq.history().size() == 1);  // exceptions don't record

    auto res = acq.processBatch({{"R1", 10}, {"R1", 100}, {"B1", 1}, {"zzz", 1}, {"B1", 0}});
    assert(res.size() == 5);
    assert(res[0].approved && res[0].cost == Money::of(150));
    assert(!res[1].approved);  // 1050 > remaining 150
    assert(res[2].approved);
    assert(!res[3].approved && res[3].reason.find("not found") != std::string::npos);
    assert(!res[4].approved);
    assert(acq.totalSpent() == Money::of(450));
    assert(b.spent() == acq.totalSpent());
    assert(c.holdings("R1") == 10 && c.holdings("B1") == 3);
    assert(acq.history().size() == 6);
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
    assert(successful.size() == 2);
    assert(successful[0].approved && successful[1].approved);
    assert(b.spent() == Money::of(200));
    assert(c.holdings("B1") == 1 && c.holdings("B2") == 1);
    assert(acq.history().size() == 2);

    // If any request fails, nothing in this batch is purchased or recorded.
    const Money spentBefore = b.spent();
    const auto historyBefore = acq.history().size();
    const int b1Before = c.holdings("B1");
    const int b2Before = c.holdings("B2");

    auto rejected = acq.processBatch({{"B1", 1}, {"missing", 1}}, true);
    assert(rejected.size() == 2);
    assert(!rejected[0].approved && !rejected[1].approved);
    assert(rejected[0].reason.find("batch rejected") != std::string::npos);
    assert(b.spent() == spentBefore);
    assert(c.holdings("B1") == b1Before && c.holdings("B2") == b2Before);
    assert(acq.history().size() == historyBefore);

    // Cumulative costs are validated, not just each request independently.
    auto overBudget = acq.processBatch({{"B1", 1}, {"B2", 1}}, true);
    assert(overBudget.size() == 2);
    assert(!overBudget[0].approved && !overBudget[1].approved);
    assert(b.spent() == spentBefore);
    assert(c.holdings("B1") == b1Before && c.holdings("B2") == b2Before);
    assert(acq.history().size() == historyBefore);
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

    assert(acq.canPurchase("B1", 1));
    acq.purchase("B1", 1);

    // Buying the same title again must not use another title slot.
    assert(acq.canPurchase("B1", 1));
    acq.purchase("B1", 1);

    // A different title exceeds the one-title category limit.
    assert(!acq.canPurchase("B2", 1));
    assert(throwsAs<QuotaExceededError>([&]() { (void)(acq.purchase("B2", 1)); }));

    assert(c.holdings("B1") == 2);
    assert(c.holdings("B2") == 0);
    assert(b.spent() == Money::of(200));
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

    assert(c.holdings("B1") == 3);
    assert(b.spent() == Money::of(330));
    assert(acq.totalSpent() == Money::of(330));

    // Cancelling one order refunds its post-tax cost and units.
    const auto& cancellation = acq.cancelOrder(firstOrder);
    assert(cancellation.cancellation);
    assert(cancellation.cancelsOrderNo == firstOrder);
    assert(c.holdings("B1") == 1);
    assert(b.spent() == Money::of(110));
    assert(b.usageFor(ResourceCategory::Book).units == 1);
    assert(acq.totalSpent() == Money::of(110));
    assert(acq.history().size() == 3);
    assert(acq.history()[0].approved && !acq.history()[0].cancellation);
    assert(acq.history()[0].orderNo == firstOrder);

    // The title slot must remain occupied while another B1 order is active.
    assert(!acq.canPurchase("B2", 1));

    // A second cancellation of the same order must fail without side effects.
    assert(throwsAs<std::invalid_argument>([&]() { (void)(acq.cancelOrder(firstOrder)); }));
    assert(c.holdings("B1") == 1);
    assert(b.spent() == Money::of(110));

    // Cancelling the last active B1 order releases the title slot.
    const int secondOrder = acq.history()[1].orderNo;
    acq.cancelOrder(secondOrder);
    assert(c.holdings("B1") == 0);
    assert(b.spent() == Money::of(0));
    assert(acq.totalSpent() == Money::of(0));
    assert(b.usageFor(ResourceCategory::Book).units == 0);
    assert(b.usageFor(ResourceCategory::Book).titles.empty());
    assert(acq.canPurchase("B2", 1));

    // Rejected and nonexistent orders cannot be cancelled.
    const auto rejected = acq.processBatch({{"B2", 100}}).front();
    assert(!rejected.approved);
    assert(throwsAs<std::invalid_argument>([&]() { (void)(acq.cancelOrder(rejected.orderNo)); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(acq.cancelOrder(9999)); }));
}


static void testYearEndRollover() {
    Budget thisYear(Money::of(1000));
    thisYear.commit(ResourceCategory::Book, 2, Money::of(700));

    Budget nextYear = thisYear.rollover(50.0, Money::of(2000));
    assert(nextYear.total() == Money::of(2150));
    assert(nextYear.spent() == Money::of(0));
    assert(nextYear.remaining() == Money::of(2150));

    Budget noCarry = thisYear.rollover(0.0, Money::of(2000));
    assert(noCarry.total() == Money::of(2000));

    Budget fullCarry = thisYear.rollover(100.0, Money::of(2000));
    assert(fullCarry.total() == Money::of(2300));

    Budget fractional = thisYear.rollover(33.0, Money::of(2000));
    assert(fractional.total() == Money::of(2099, 0)); // 33% of 300 = 99

    assert(throwsAs<std::invalid_argument>([&]() { (void)(thisYear.rollover(-1.0)); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(thisYear.rollover(101.0)); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(thisYear.rollover(50.0, Money::of(-1))); }));

    // Rollover must not modify this year's budget.
    assert(thisYear.spent() == Money::of(700));
    assert(thisYear.remaining() == Money::of(300));
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
    assert(acq.canPurchase("B1", 2, "Computer Science", &why));
    assert(why.empty());
    assert(!acq.canPurchase("B1", 3, "Computer Science", &why));
    assert(!acq.canPurchase("B1", 1, "Unknown Department", &why));
    assert(why.find("department not found") != std::string::npos);
    assert(throwsAs<std::invalid_argument>([&]() { (void)(acq.departmentBudget("Unknown Department")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(acq.addDepartment("Physics", Money::of(100))); }));

    const int csOrder =
        acq.purchase("B1", 2, "Computer Science").orderNo;
    assert(acq.history().back().department == "Computer Science");
    assert(acq.departmentBudget("Computer Science").spent() == Money::of(200));
    assert(acq.departmentBudget("Physics").spent() == Money::of(0));
    assert(sharedBudget.spent() == Money::of(0));

    // Physics has its own budget and title quota.
    assert(acq.canPurchase("B2", 2, "Physics"));
    acq.purchase("B2", 2, "Physics");
    assert(acq.departmentBudget("Physics").spent() == Money::of(200));
    assert(c.holdings("B1") == 2 && c.holdings("B2") == 2);

    // A department-specific batch request uses that department's budget.
    const auto batch = acq.processBatch({
        {"B1", 1, "Computer Science"},
        {"B2", 1, "Unknown Department"}
    });
    assert(batch.size() == 2);
    assert(!batch[0].approved);  // CS has reached its title quota.
    assert(!batch[1].approved);
    assert(batch[1].department == "Unknown Department");
    assert(batch[1].reason.find("department not found") != std::string::npos);

    // Cancelling the CS order refunds the CS budget, not the shared budget.
    const auto& cancellation = acq.cancelOrder(csOrder);
    assert(cancellation.cancellation);
    assert(cancellation.department == "Computer Science");
    assert(acq.departmentBudget("Computer Science").spent() == Money::of(0));
    assert(acq.departmentBudget("Physics").spent() == Money::of(200));
    assert(sharedBudget.spent() == Money::of(0));
    assert(c.holdings("B1") == 0);
    assert(acq.canPurchase("B2", 1, "Computer Science"));
}


static void testVendorPricing() {
    Catalog c;
    Book& book = c.emplace<Book>(
        "B12", "Vendor Test Book", std::vector<std::string>{"Author"},
        "ISBN12", "Publisher", 2026, Money::of(100));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(c, budget);

    acq.addVendorOffer("B12", "Vendor A", Money::of(90));
    acq.addVendorOffer("B12", "Vendor B", Money::of(70));
    acq.addVendorOffer("B12", "Vendor C", Money::of(80));

    assert(acq.cheapestVendor("B12") == "Vendor B");
    assert(acq.quotePreTax("B12", 2) == Money::of(140));
    assert(acq.quote("B12", 2) == Money::of(140));

    const auto& order = acq.purchase("B12", 2);
    assert(order.approved);
    assert(order.vendor == "Vendor B");
    assert(order.vendorUnitPrice == Money::of(70));
    assert(order.preTaxCost == Money::of(140));
    assert(order.cost == Money::of(140));

    // Vendor pricing must not permanently change the catalog price.
    assert(book.unitPrice() == Money::of(100));

    // Resources without vendor offers keep the existing behavior.
    Catalog c2;
    c2.emplace<Book>(
        "B13", "Regular Book", std::vector<std::string>{"Author"},
        "ISBN13", "Publisher", 2026, Money::of(50));
    Budget budget2(Money::of(500));
    AcquisitionManager acq2(c2, budget2);

    const auto& regularOrder = acq2.purchase("B13", 2);
    assert(regularOrder.vendor.empty());
    assert(regularOrder.vendorUnitPrice == Money::of(50));
    assert(regularOrder.cost == Money::of(100));

    assert(throwsAs<std::invalid_argument>([&]() { (void)(
        acq.addVendorOffer("B12", "Invalid Vendor", Money::fromMinor(-1))); }));

    // Q12 + Q11: batch purchases must use and record the cheapest vendor.
    auto batch = acq.processBatch({{"B12", 1}}, true);
    assert(batch.size() == 1);
    assert(batch[0].approved);
    assert(batch[0].vendor == "Vendor B");
    assert(batch[0].vendorUnitPrice == Money::of(70));
    assert(batch[0].preTaxCost == Money::of(70));
}


static void testLending() {
    Catalog c;
    c.emplace<Book>("B1", "Print Book", std::vector<std::string>{"Author"},
                    "ISBN1", "Publisher", 2020, Money::of(10));
    c.emplace<ElectronicResource>("R1", "Digital Resource", "Publisher", 2020,
                                  Money::of(10), "url");

    c.addHoldings("B1", 2);
    c.addHoldings("R1", 2);

    LendingManager lending(c);

    // Print borrowing respects the number of copies held.
    lending.borrow("B1", "P1");
    lending.borrow("B1", "P2");
    assert(lending.borrowedCount("B1") == 2);
    assert(throwsAs<std::runtime_error>([&]() { (void)(lending.borrow("B1", "P3")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.borrow("B1", "P1")); }));

    // Returning frees a copy; returning twice is invalid.
    lending.returnCopy("B1", "P1");
    assert(lending.borrowedCount("B1") == 1);
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.returnCopy("B1", "P1")); }));
    lending.borrow("B1", "P3");
    assert(lending.borrowedCount("B1") == 2);

    // Print resources cannot be opened as electronic sessions.
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.openSession("B1", "P1")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.returnCopy("B1", "UNKNOWN")); }));

    // Electronic sessions respect licensed-seat limits.
    lending.openSession("R1", "P1");
    lending.openSession("R1", "P2");
    assert(lending.activeSessions("R1") == 2);
    assert(throwsAs<std::runtime_error>([&]() { (void)(lending.openSession("R1", "P3")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.openSession("R1", "P1")); }));

    // Closing a session frees a seat.
    lending.closeSession("R1", "P1");
    assert(lending.activeSessions("R1") == 1);
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.closeSession("R1", "P1")); }));
    lending.openSession("R1", "P3");
    assert(lending.activeSessions("R1") == 2);

    // Electronic resources cannot be borrowed as print copies.
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.borrow("R1", "P4")); }));
    assert(throwsAs<std::invalid_argument>([&]() { (void)(lending.closeSession("B1", "P1")); }));

    // Lending does not change the total copies or licensed seats.
    assert(c.holdings("B1") == 2);
    assert(c.holdings("R1") == 2);

    // Unknown resource IDs are reported.
    assert(throwsAs<NotFoundError>([&]() { (void)(lending.borrow("MISSING", "P1")); }));
    assert(throwsAs<NotFoundError>([&]() { (void)(lending.openSession("MISSING", "P1")); }));
}

int main() {
    testMoney();
    testResourcesAndCost();
    testJournal();
    testEBook();
    testCatalog();
    testLending();
    testBudget();
    testTitleQuota();
    testOrderCancellation();
    testYearEndRollover();
    testDepartmentBudgets();
    testVendorPricing();
    testAllOrNothingBatch();
    testAcquisition();
    testAudioBook();
    testThesis();

    std::cout << "All assertions passed\n";
    return 0;
}


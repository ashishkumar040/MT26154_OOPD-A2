#include "bookmgmt/Acquisition.h"

#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

namespace {
Money costAtUnitPrice(Resource& resource, int quantity, Money unitPrice);
}

AcquisitionManager::AcquisitionManager(Catalog& catalog, Budget& budget,
                                       TaxRates taxRates)
    : catalog_(catalog), budget_(budget), taxRates_(taxRates) {}

void AcquisitionManager::addDepartment(const std::string& name,
                                      Money totalBudget) {
    if (name.empty())
        throw std::invalid_argument("department name cannot be empty");

    if (departments_.find(name) != departments_.end())
        throw std::invalid_argument("department already exists: " + name);

    departments_.emplace(name, Department(name, totalBudget));
}

Budget& AcquisitionManager::departmentBudget(const std::string& name) {
    auto it = departments_.find(name);
    if (it == departments_.end())
        throw std::invalid_argument("department not found: " + name);
    return it->second.budget;
}

const Budget& AcquisitionManager::departmentBudget(
    const std::string& name) const {
    auto it = departments_.find(name);
    if (it == departments_.end())
        throw std::invalid_argument("department not found: " + name);
    return it->second.budget;
}

Money AcquisitionManager::quotePreTax(const std::string& id, int quantity) const {
    const Resource& r = catalog_.get(id);
    auto offers = vendorOffers_.find(id);
    if (offers == vendorOffers_.end() || offers->second.empty())
        return r.costFor(quantity);

    auto best = offers->second.begin();
    for (auto it = std::next(best); it != offers->second.end(); ++it) {
        if (it->second < best->second)
            best = it;
    }
    return costAtUnitPrice(const_cast<Resource&>(r), quantity, best->second);
}

Money AcquisitionManager::taxFor(const std::string& id, int quantity) const {
    const Resource& r = catalog_.get(id);
    const Money preTax = quotePreTax(id, quantity);

    const int rate = r.isDigital()
        ? taxRates_.electronicPercent
        : taxRates_.printPercent;

    return Money::fromMinor(preTax.minorUnits() * rate / 100);
}

Money AcquisitionManager::quote(const std::string& id, int quantity) const {
    return quotePreTax(id, quantity) + taxFor(id, quantity);
}

bool AcquisitionManager::canPurchase(const std::string& id, int quantity,
                                     std::string* reason) const {
    std::string why;
    if (const Resource* r = catalog_.find(id)) {
        if (quantity <= 0)
            why = "quantity must be positive";
        else
            why = budget_.check(r->category(), quantity, quote(id, quantity), id);
    } else {
        why = "resource not found: " + id;
    }
    if (reason) *reason = why;
    return why.empty();
}

PurchaseRecord& AcquisitionManager::record(const Resource* r, const std::string& id,
                                           int qty, Money preTaxCost, Money cost,
                                           bool approved, std::string reason) {
    history_.push_back(PurchaseRecord{
        nextOrderNo_++, id, r ? r->title() : std::string("(unknown)"),
        r ? r->category() : ResourceCategory::Book, qty,
        preTaxCost, cost, approved, std::move(reason)});
    return history_.back();
}

bool AcquisitionManager::canPurchase(const std::string& id, int quantity,
                                     const std::string& department,
                                     std::string* reason) const {
    std::string why;
    auto it = departments_.find(department);

    if (it == departments_.end()) {
        why = "department not found: " + department;
    } else if (quantity <= 0) {
        why = "quantity must be positive";
    } else if (const Resource* r = catalog_.find(id)) {
        const Money cost = quote(id, quantity);
        why = it->second.budget.check(
            r->category(), quantity, cost, id);
    } else {
        why = "resource not found: " + id;
    }

    if (reason)
        *reason = why;
    return why.empty();
}

namespace {
Money costAtUnitPrice(Resource& resource, int quantity, Money unitPrice) {
    const Money originalPrice = resource.unitPrice();
    struct RestorePrice {
        Resource& resource;
        Money originalPrice;
        ~RestorePrice() { resource.setUnitPrice(originalPrice); }
    } restore{resource, originalPrice};

    resource.setUnitPrice(unitPrice);
    return resource.costFor(quantity);
}
}

void AcquisitionManager::addVendorOffer(
    const std::string& resourceId, const std::string& vendor, Money unitPrice) {
    if (!catalog_.contains(resourceId))
        throw NotFoundError(resourceId);
    if (vendor.empty())
        throw std::invalid_argument("vendor name cannot be empty");
    if (unitPrice.isNegative())
        throw std::invalid_argument("vendor price must not be negative");
    vendorOffers_[resourceId][vendor] = unitPrice;
}

std::string AcquisitionManager::cheapestVendor(
    const std::string& resourceId) const {
    auto resourceIt = vendorOffers_.find(resourceId);
    if (resourceIt == vendorOffers_.end() || resourceIt->second.empty())
        return {};
    auto best = resourceIt->second.begin();
    for (auto it = std::next(best); it != resourceIt->second.end(); ++it) {
        if (it->second < best->second)
            best = it;
    }
    return best->first;
}

const PurchaseRecord& AcquisitionManager::purchase(const std::string& id, int quantity) {
    const Resource& r = catalog_.get(id);
    const Money preTaxCost = quotePreTax(id, quantity);
    const Money cost = preTaxCost + taxFor(id, quantity);
    const std::string vendor = cheapestVendor(id);
    const auto offers = vendorOffers_.find(id);
    const Money vendorPrice = vendor.empty()
        ? r.unitPrice() : offers->second.at(vendor);

    budget_.commit(r.category(), quantity, cost, id);
    catalog_.addHoldings(id, quantity);

    history_.push_back(PurchaseRecord{
        nextOrderNo_++, id, r.title(), r.category(), quantity,
        preTaxCost, cost, true, {}, false, 0, {},
        vendor, vendorPrice
    });
    return history_.back();
}

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& id, int quantity, const std::string& department) {
    auto it = departments_.find(department);
    if (it == departments_.end())
        throw std::invalid_argument("department not found: " + department);
    if (quantity <= 0)
        throw std::invalid_argument("quantity must be positive");

    const Resource& r = catalog_.get(id);
    const Money preTaxCost = quotePreTax(id, quantity);
    const Money cost = preTaxCost + taxFor(id, quantity);
    const std::string vendor = cheapestVendor(id);
    const auto offers = vendorOffers_.find(id);
    const Money vendorPrice = vendor.empty()
        ? r.unitPrice() : offers->second.at(vendor);

    it->second.budget.commit(r.category(), quantity, cost, id);
    catalog_.addHoldings(id, quantity);

    history_.push_back(PurchaseRecord{
        nextOrderNo_++, id, r.title(), r.category(), quantity,
        preTaxCost, cost, true, {}, false, 0, department,
        vendor, vendorPrice
    });
    return history_.back();
}

const PurchaseRecord& AcquisitionManager::cancelOrder(int orderNo) {
    auto original = history_.end();

    for (auto it = history_.begin(); it != history_.end(); ++it) {
        if (it->orderNo == orderNo && !it->cancellation) {
            original = it;
            break;
        }
    }

    if (original == history_.end())
        throw std::invalid_argument("order not found");
    if (!original->approved)
        throw std::invalid_argument("cannot cancel a rejected order");

    for (const auto& rec : history_) {
        if (rec.cancellation && rec.cancelsOrderNo == orderNo)
            throw std::invalid_argument("order already cancelled");
    }

    const PurchaseRecord cancelled = *original;
    if (catalog_.holdings(cancelled.resourceId) < cancelled.quantity)
        throw std::logic_error("holdings are insufficient to cancel this order");

    // Keep the title quota occupied if another active order uses this title.
    bool anotherActivePurchase = false;
    for (const auto& rec : history_) {
        if (!rec.approved || rec.cancellation ||
            rec.resourceId != cancelled.resourceId || rec.orderNo == orderNo)
            continue;

        bool alreadyCancelled = false;
        for (const auto& entry : history_) {
            if (entry.cancellation && entry.cancelsOrderNo == rec.orderNo) {
                alreadyCancelled = true;
                break;
            }
        }
        if (!alreadyCancelled) {
            anotherActivePurchase = true;
            break;
        }
    }

    Budget& refundBudget = cancelled.department.empty()
        ? budget_
        : departmentBudget(cancelled.department);

    refundBudget.refund(cancelled.category, cancelled.quantity, cancelled.cost,
                        cancelled.resourceId, !anotherActivePurchase);
    catalog_.addHoldings(cancelled.resourceId, -cancelled.quantity);

    history_.push_back(PurchaseRecord{
        nextOrderNo_++, cancelled.resourceId, cancelled.title,
        cancelled.category, cancelled.quantity, cancelled.preTaxCost,
        cancelled.cost, true, "Cancellation of order #" +
        std::to_string(orderNo), true, orderNo, cancelled.department
    });

    return history_.back();
}

std::vector<PurchaseRecord> AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs, bool allOrNothing) {
    if (allOrNothing) {
        Budget stagedGeneral = budget_;
        std::map<std::string, Budget> stagedDepartments;
        for (const auto& entry : departments_)
            stagedDepartments.emplace(entry.first, entry.second.budget);

        std::vector<PurchaseRecord> results;
        results.reserve(reqs.size());
        std::string batchFailure;

        for (const auto& req : reqs) {
            const Resource* r = catalog_.find(req.resourceId);
            Money preTaxCost;
            Money cost;
            std::string why;

            if (!r) {
                why = "resource not found: " + req.resourceId;
            } else if (req.quantity <= 0) {
                why = "quantity must be positive";
            } else {
                preTaxCost = quotePreTax(req.resourceId, req.quantity);
                cost = preTaxCost + taxFor(req.resourceId, req.quantity);

                Budget* stagedBudget = &stagedGeneral;
                if (!req.department.empty()) {
                    auto it = stagedDepartments.find(req.department);
                    if (it == stagedDepartments.end()) {
                        why = "department not found: " + req.department;
                    } else {
                        stagedBudget = &it->second;
                    }
                }

                if (why.empty()) {
                    why = stagedBudget->check(
                        r->category(), req.quantity, cost, req.resourceId);
                    if (why.empty()) {
                        try {
                            stagedBudget->commit(
                                r->category(), req.quantity, cost, req.resourceId);
                        } catch (const std::exception& e) {
                            why = e.what();
                        }
                    }
                }
            }

            results.push_back(PurchaseRecord{
                0, req.resourceId,
                r ? r->title() : std::string("(unknown)"),
                r ? r->category() : ResourceCategory::Book,
                req.quantity, preTaxCost, cost, why.empty(), why,
                false, 0, req.department
            });

            if (!why.empty() && batchFailure.empty())
                batchFailure = why;
        }

        if (!batchFailure.empty()) {
            for (auto& result : results) {
                result.approved = false;
                result.reason = "batch rejected: " + batchFailure;
            }
            return results;
        }

        results.clear();
        for (const auto& req : reqs) {
            if (req.department.empty())
                results.push_back(purchase(req.resourceId, req.quantity));
            else
                results.push_back(
                    purchase(req.resourceId, req.quantity, req.department));
        }
        return results;
    }

    std::vector<PurchaseRecord> results;
    results.reserve(reqs.size());

    for (const auto& req : reqs) {
        const Resource* r = catalog_.find(req.resourceId);
        Money preTaxCost;
        Money cost;
        std::string why;

        if (!r) {
            why = "resource not found: " + req.resourceId;
        } else if (req.quantity <= 0) {
            why = "quantity must be positive";
        } else {
            preTaxCost = quotePreTax(req.resourceId, req.quantity);
            cost = preTaxCost + taxFor(req.resourceId, req.quantity);

            if (req.department.empty()) {
                why = budget_.check(r->category(), req.quantity, cost,
                                    req.resourceId);
            } else {
                auto it = departments_.find(req.department);
                if (it == departments_.end()) {
                    why = "department not found: " + req.department;
                } else {
                    why = it->second.budget.check(
                        r->category(), req.quantity, cost, req.resourceId);
                }
            }
        }

        if (why.empty()) {
            if (req.department.empty()) {
                results.push_back(purchase(req.resourceId, req.quantity));
            } else {
                results.push_back(
                    purchase(req.resourceId, req.quantity, req.department));
            }
        } else {
            results.push_back(record(r, req.resourceId, req.quantity,
                                     preTaxCost, cost, false, why));
            results.back().department = req.department;
            history_.back().department = req.department;
        }
    }
    return results;
}

Money AcquisitionManager::totalSpent() const {
    Money sum;
    for (const auto& rec : history_) {
        if (!rec.approved || rec.cancellation)
            continue;

        bool cancelled = false;
        for (const auto& entry : history_) {
            if (entry.cancellation && entry.cancelsOrderNo == rec.orderNo) {
                cancelled = true;
                break;
            }
        }

        if (!cancelled)
            sum += rec.cost;
    }
    return sum;
}

void AcquisitionManager::printReport(std::ostream& os) const {
    os << "Order history (" << history_.size() << " orders)\\n";
    for (const auto& rec : history_) {
        os << "  #" << std::setw(3) << std::left << rec.orderNo << " "
           << (rec.cancellation ? "CANCELLED" : (rec.approved ? "APPROVED" : "REJECTED")) << "  " << std::setw(6)
           << rec.resourceId << " x" << std::setw(3) << rec.quantity
           << "  pre-tax " << rec.preTaxCost
           << "  post-tax " << rec.cost << "  " << rec.title;
        if (!rec.vendor.empty())
            os << "  vendor: " << rec.vendor
               << " @ " << rec.vendorUnitPrice << " per unit";
        if (!rec.approved) os << "\\n        reason: " << rec.reason;
        os << "\\n";
    }
    os << "Pre-tax total: ";
    Money preTaxTotal;
    for (const auto& rec : history_) {
        if (!rec.approved || rec.cancellation)
            continue;

        bool cancelled = false;
        for (const auto& entry : history_) {
            if (entry.cancellation && entry.cancelsOrderNo == rec.orderNo) {
                cancelled = true;
                break;
            }
        }

        if (!cancelled)
            preTaxTotal += rec.preTaxCost;
    }
    os << preTaxTotal << "\\n";
    os << "Post-tax total: " << totalSpent() << "\\n";
}

}  // namespace bookmgmt

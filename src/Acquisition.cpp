#include "bookmgmt/Acquisition.h"

#include <iomanip>
#include <ostream>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

AcquisitionManager::AcquisitionManager(Catalog& catalog, Budget& budget,
                                       TaxRates taxRates)
    : catalog_(catalog), budget_(budget), taxRates_(taxRates) {}
Money AcquisitionManager::quotePreTax(const std::string& id, int quantity) const {
    return catalog_.get(id).costFor(quantity);
}

Money AcquisitionManager::taxFor(const std::string& id, int quantity) const {
    const Resource& r = catalog_.get(id);
    const Money preTax = r.costFor(quantity);

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

const PurchaseRecord& AcquisitionManager::purchase(const std::string& id, int quantity) {
    const Resource& r = catalog_.get(id);
    const Money preTaxCost = r.costFor(quantity);
    const Money cost = preTaxCost + taxFor(id, quantity);

    budget_.commit(r.category(), quantity, cost, id);
    catalog_.addHoldings(id, quantity);

    history_.push_back(PurchaseRecord{
        nextOrderNo_++, id, r.title(), r.category(), quantity,
        preTaxCost, cost, true, {}
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

    budget_.refund(cancelled.category, cancelled.quantity, cancelled.cost,
                   cancelled.resourceId, !anotherActivePurchase);
    catalog_.addHoldings(cancelled.resourceId, -cancelled.quantity);

    history_.push_back(PurchaseRecord{
        nextOrderNo_++, cancelled.resourceId, cancelled.title,
        cancelled.category, cancelled.quantity, cancelled.preTaxCost,
        cancelled.cost, true, "Cancellation of order #" +
        std::to_string(orderNo), true, orderNo
    });

    return history_.back();
}

std::vector<PurchaseRecord> AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs) {
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
            preTaxCost = r->costFor(req.quantity);
            cost = preTaxCost + taxFor(req.resourceId, req.quantity);
            why = budget_.check(r->category(), req.quantity, cost, req.resourceId);
        }

        if (why.empty()) {
            results.push_back(purchase(req.resourceId, req.quantity));
        } else {
            results.push_back(record(r, req.resourceId, req.quantity,
                                     preTaxCost, cost, false, why));
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

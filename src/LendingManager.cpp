#include "bookmgmt/LendingManager.h"

#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

void LendingManager::borrow(const std::string& resourceId,
                            const std::string& patronId) {
    const Resource& resource = catalog_.get(resourceId);

    if (resource.isDigital()) {
        throw std::invalid_argument("electronic resources use openSession()");
    }
    if (patronId.empty()) {
        throw std::invalid_argument("patron ID cannot be empty");
    }

    const PatronResource key{resourceId, patronId};
    if (borrowed_.count(key) != 0) {
        throw std::invalid_argument("patron already borrowed this resource");
    }

    if (borrowedCount(resourceId) >= catalog_.holdings(resourceId)) {
        throw std::runtime_error("no print copies available");
    }

    borrowed_[key] = 1;
}

void LendingManager::returnCopy(const std::string& resourceId,
                                const std::string& patronId) {
    const Resource& resource = catalog_.get(resourceId);

    if (resource.isDigital()) {
        throw std::invalid_argument("electronic resources use closeSession()");
    }

    const PatronResource key{resourceId, patronId};
    const auto it = borrowed_.find(key);
    if (it == borrowed_.end()) {
        throw std::invalid_argument("patron has not borrowed this resource");
    }

    borrowed_.erase(it);
}

void LendingManager::openSession(const std::string& resourceId,
                                 const std::string& patronId) {
    const Resource& resource = catalog_.get(resourceId);

    if (!resource.isDigital()) {
        throw std::invalid_argument("print resources cannot open sessions");
    }
    if (patronId.empty()) {
        throw std::invalid_argument("patron ID cannot be empty");
    }

    const PatronResource key{resourceId, patronId};
    if (sessions_.count(key) != 0) {
        throw std::invalid_argument("patron already has an open session");
    }

    if (activeSessions(resourceId) >= catalog_.holdings(resourceId)) {
        throw std::runtime_error("no licensed seats available");
    }

    sessions_[key] = true;
}

void LendingManager::closeSession(const std::string& resourceId,
                                  const std::string& patronId) {
    const Resource& resource = catalog_.get(resourceId);

    if (!resource.isDigital()) {
        throw std::invalid_argument("print resources cannot close sessions");
    }

    const PatronResource key{resourceId, patronId};
    const auto it = sessions_.find(key);
    if (it == sessions_.end()) {
        throw std::invalid_argument("patron has no open session");
    }

    sessions_.erase(it);
}

int LendingManager::borrowedCount(const std::string& resourceId) const {
    (void)catalog_.get(resourceId);

    int count = 0;
    for (const auto& [key, value] : borrowed_) {
        if (key.first == resourceId) {
            count += value;
        }
    }
    return count;
}

int LendingManager::activeSessions(const std::string& resourceId) const {
    (void)catalog_.get(resourceId);

    int count = 0;
    for (const auto& [key, open] : sessions_) {
        if (key.first == resourceId && open) {
            ++count;
        }
    }
    return count;
}

}  // namespace bookmgmt

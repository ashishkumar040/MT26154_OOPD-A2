#pragma once

#include <map>
#include <string>
#include <utility>

#include "bookmgmt/Catalog.h"

namespace bookmgmt {

class LendingManager {
public:
    explicit LendingManager(Catalog& catalog) : catalog_(catalog) {}

    void borrow(const std::string& resourceId, const std::string& patronId);
    void returnCopy(const std::string& resourceId, const std::string& patronId);

    void openSession(const std::string& resourceId, const std::string& patronId);
    void closeSession(const std::string& resourceId, const std::string& patronId);

    int borrowedCount(const std::string& resourceId) const;
    int activeSessions(const std::string& resourceId) const;

private:
    using PatronResource = std::pair<std::string, std::string>;

    Catalog& catalog_;
    std::map<PatronResource, int> borrowed_;
    std::map<PatronResource, bool> sessions_;
};

}  // namespace bookmgmt

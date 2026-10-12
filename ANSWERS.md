# Assignment 2 — Answers

## Questions Attempted

Questions 1–16 have been implemented in the project.

## Design Decisions

- **Q1 — Journal:** Added a print-journal resource with ISSN, issues per year, subscription length, and subscription-based pricing.
- **Q2 — EBook:** Derived EBook from ElectronicResource to reuse electronic pricing and licensing behavior.
- **Q3 — AudioBook and Thesis:** Used suitable existing resource base classes to reuse common resource behavior while adding type-specific details.
- **Q4 — Hardcover pricing:** Applied the hardcover price premium through the pricing implementation.
- **Q5 — Bulk discounts:** Implemented the specified discounts for print copies and electronic seats.
- **Q6 — Taxes:** Added configurable tax handling and used post-tax costs for budget and quota checks.
- **Q7 — Title quotas:** Added limits on distinct titles by resource category.
- **Q8 — Cancellation:** Added cancellation handling that restores budget and quota usage and updates holdings while preserving history.
- **Q9 — Department budgets:** Added department-specific budgets and quotas.
- **Q10 — Year-end rollover:** Added configurable carry-forward of the unspent budget.
- **Q11 — All-or-nothing batches:** Added batch processing that avoids partial purchases when all-or-nothing mode is enabled.
- **Q12 — Vendors:** Added vendor pricing and selection of the cheapest vendor for purchases.
- **Q13 — Catalogue searches:** Added searches by author, ISBN/ISSN, and publication-year range.
- **Q14 — Lending:** Added borrowing and returning of print copies and opening and closing electronic-resource sessions.
- **Q15 — Currency-aware Money:** Added currency codes and exceptions for arithmetic or comparisons involving different currencies.
- **Q16 — Assertions:** Replaced the test macros with standard C++ assertions.

## Written Answers

### Q2 — EBook and Book duplication

Book and EBook can duplicate fields and behavior related to authors, ISBN, and book-specific metadata. A shared intermediate class for common book metadata could reduce this duplication while allowing print books and electronic books to retain their distinct resource behavior.

### Q3 — Base-class choices

AudioBook is represented using the existing book/resource hierarchy to reuse common resource information and pricing behavior while adding narrator and duration. Thesis uses the resource hierarchy to represent thesis-specific metadata and support a usually zero price.

### Q15 — Currency handling

Money stores a currency code alongside its fixed-point amount. Arithmetic and comparisons between different currencies throw an exception rather than silently combining incompatible amounts.

### Q16 — Assertions

The test runner uses `assert` for checks and a helper to verify expected exception types. The tests are run through the project's test executable.

## Verification

The project was built using CMake, and the test executable reported:

`All assertions passed`

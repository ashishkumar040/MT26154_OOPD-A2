# BookManagement — OOPD Assignment 2

## Assignment Overview

BookManagement is a C++17 library-acquisition system. It maintains a catalogue of resources, calculates acquisition costs, and checks category quotas and the overall budget before approving purchases. Assignment 2 extends the starter project with additional resource types, pricing rules, budget and quota management, acquisition features, catalogue searches, lending, currency-aware money, and cleaner tests.

## Requirements and Build Instructions

Requirements:
- A C++17-compatible compiler
- CMake
- Git

From the project root, configure and build the project:

```bash
cmake -S . -B build
cmake --build build
```

Run the tests:

```bash
./build/bookmgmt_tests
```

Run the demo (the assignment's supplied CMake configuration names this executable `demo`):

```bash
./build/demo
```

The assignment requires the project to compile without warnings using `-Wall -Wextra`, all existing tests to continue passing, tests for each new question to be added to `tests/test_main.cpp`, and the demo to be extended in `examples/demo.cpp`.

## Question-wise Implementation

### Q1 — Journal

Adds `Journal`, derived from `Resource`, for print journals bought as subscriptions. It stores the ISSN, issues per year, and subscription length. Subscription length defaults to one year and must be at least one; an invalid value throws `std::invalid_argument`. The annual unit price is multiplied by the number of copies and subscription years. The category is included in the category-name and all-categories support, and `printDetails()` reports the journal-specific fields.

### Q2 — EBook

Adds `EBook`, derived from `ElectronicResource`, with authors, ISBN, file format (PDF, EPUB, or HTML), and DRM-protection information. It retains the parent electronic-resource pricing and has its own resource category and quota. Its details include inherited electronic-resource information followed by ebook-specific information. The design question notes that `Book` and `EBook` both store author and ISBN information; a shared book-information abstraction could reduce that duplication.

### Q3 — AudioBook and Thesis

Adds `AudioBook` with narrator and duration in minutes, and `Thesis` with university, degree, and supervisor. Their base classes should reflect their resource behaviour: an audiobook is an electronic resource when licensed like other electronic resources, while a thesis can use the resource base directly when modelled as an academic item that is usually free. The code comment should justify the selected bases.

### Q4 — Hardcover Pricing

Hardcover books cost 20% more than their listed unit price.

### Q5 — Bulk Discounts

Print items receive a 10% discount when at least 10 copies are purchased in one order. For electronic resources, each seat after the 50th costs half price.

### Q6 — Taxes

Print and electronic items use different configurable tax rates. Reports show pre-tax and post-tax totals, and quota checks use the post-tax cost.

### Q7 — Distinct-Title Quotas

Adds a limit on the number of different titles that may be purchased in each category, alongside the existing quota and budget checks.

### Q8 — Cancellation

Allows an approved order to be cancelled. The cancellation refunds budget and quota usage and reduces holdings. The original order remains in history, and a separate cancellation record is added.

### Q9 — Department Budgets

Gives each department its own budget and quotas. A purchase request identifies the department to which the cost is charged.

### Q10 — Year-End Rollover

Creates the next year's budget from the current year's budget by carrying forward a configurable percentage of the unspent amount.

### Q11 — All-or-Nothing Batch Processing

Adds an all-or-nothing option to `processBatch`. If any request in the batch would be rejected, no request in that batch is purchased.

### Q12 — Vendors

Records multiple vendors and their prices for the same title, purchases from the cheapest vendor, and records the selected vendor in the order history.

### Q13 — Catalogue Searches

Adds searches by author, ISBN/ISSN, and a range of publication years.

### Q14 — Lending

Adds lending support for print copies, limited by copies held, and electronic sessions, limited by the number of licensed seats. Patrons can borrow and return print copies and open and close electronic sessions.

### Q15 — Currency-Aware Money

Adds a currency code to `Money`. Adding, subtracting, or comparing amounts with different currencies must throw an exception.

### Q16 — Assertions

Replaces the test macros with standard C++ `assert` checks without changing the tests' meanings.

## Testing

Run the build and test commands above after making changes. A successful test run in this project reports:

```text
All assertions passed
```

## Submission Checklist

- Include the complete project directory in one `.zip` archive.
- Include the Git repository and its `.git` directory in the archive.
- Include `ANSWERS.md` listing the questions attempted, design decisions, and written answers.
- Include the source code, tests, demo, and documentation.
- Submit a `.zip` file, not a `.rar` file.

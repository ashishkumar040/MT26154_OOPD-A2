# Q4 - Hardcover Pricing

## Requirement
Hardcover books cost 20% more than their listed unit price.

## Implementation
- Added a `costFor(int quantity)` override in `Book`.
- Paperback books retain their listed unit price.
- Hardcover books apply a 20% surcharge before multiplying by quantity.
- Invalid quantities are rejected.

## Testing
Added tests for:
- Hardcover pricing for one copy.
- Hardcover pricing for multiple copies.
- Unchanged paperback pricing.
- Invalid quantity handling.

## Verification
- Build: `cmake --build build`
- Tests: `./build/bookmgmt_tests`
- Result: 111/111 checks passed.
- Demo: demonstrates the cost of two hardcover copies.

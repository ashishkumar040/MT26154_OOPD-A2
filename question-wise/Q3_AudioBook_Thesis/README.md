# Q3 - AudioBook and Thesis

## AudioBook
- Inherits from ElectronicResource.
- Stores narrator and duration in minutes.
- Reuses digital-resource licensing and pricing.
- Includes a comment explaining the inheritance choice.

## Thesis
- Inherits directly from Resource.
- Stores university, degree, and supervisor.
- Defaults to zero cost.
- Includes a comment explaining the inheritance choice.

## Files
- AudioBook.h
- AudioBook.cpp
- Thesis.h
- Thesis.cpp

## Testing
The complete project test suite passed 107/107 checks.

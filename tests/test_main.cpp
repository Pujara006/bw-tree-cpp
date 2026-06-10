#include "tests.hpp"

int main()
{
    runInsertSearchTests();
    runRangeSearchTests();
    runValidationTests();
    runDeleteTests();
    runConcurrencyTests();
    return 0;
}
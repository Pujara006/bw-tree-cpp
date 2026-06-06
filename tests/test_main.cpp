#include "tests.hpp"

int main()
{
    runInsertSearchTests();
    runRangeSearchTests();
    runValidationTests();
    runDeleteTests();
    return 0;
}
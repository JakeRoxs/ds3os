#include <cassert>

// Forward declarations of the test suites.
void RunHookManagerTests();
void RunInjectorInitTests();

int main()
{
    RunHookManagerTests();
    RunInjectorInitTests();

    return 0;
}

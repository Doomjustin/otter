#include <cstdlib>
#include <iostream>

#include <async/async.h>

auto factorial(int n) -> otter::async::Task<int>
{
    if (n <= 1)
        co_return 1;

    auto tmp = co_await factorial(n - 1);
    co_return (n * tmp);
}

auto hello() -> otter::async::Task<void>
{
    auto result = co_await factorial(5);
    std::cout << "factorial(5) = " << result << "\n";
    co_return;
}

int main()
{
    auto task = hello();
    task();

    return EXIT_SUCCESS;
}
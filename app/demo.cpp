#include <cstdlib>
#include <format>
#include <iostream>
#include <stop_token>

#include <otter/async.h>

using namespace otter::async;

auto factorial(int n) -> otter::async::Task<int>
{
    if (n <= 1) {
        std::cout << std::format("factorial({}) intermediate result = 1\n", n);
        co_return 1;
    }

    auto tmp = co_await factorial(n - 1);
    std::cout << std::format("factorial({}) intermediate result = {}\n", n, tmp);
    co_return (n * tmp);
}

auto hello(std::string num) -> otter::async::Task<>
{
    auto result = co_await factorial(5);
    std::cout << std::format("[{}] factorial(5) = {}\n", num, result);

    spawn(co_await this_coro::context, factorial(1));
    co_return;
}

void async_main(IOContext& ctx)
{
    spawn(ctx, hello("1"));
    ctx.run();
}

int main()
{
    std::stop_source stop_source{};
    otter::async::IOContext ctx{};
    std::thread t{ async_main, std::ref(ctx) };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    stop_source.request_stop();
    std::cout << "Stop requested\n";
    ctx.stop();

    t.join();
    std::cout << "Thread joined\n";

    return EXIT_SUCCESS;
}
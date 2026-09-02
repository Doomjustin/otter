#include <chrono>
#include <cstdlib>
#include <stop_token>

#include <spdlog/spdlog.h>

#include <otter/async.h>

using namespace otter::async;

auto factorial(int n) -> otter::async::Task<int>
{
    if (n <= 1) {
        spdlog::info("factorial({}) intermediate result = 1", n);
        co_return 1;
    }

    auto tmp = co_await factorial(n - 1);
    spdlog::info("factorial({}) intermediate result = {}", n, tmp);
    co_return (n * tmp);
}

auto hello(std::string num) -> otter::async::Task<>
{
    auto result = co_await factorial(5);
    spdlog::info("[{}] factorial(5) = {}", num, result);

    auto task = factorial(1);
    spawn(co_await this_coro::context, std::move(task));

    spdlog::info("--- Starting sleep");
    co_await sleep_for(std::chrono::minutes(1));
    spdlog::info("--- Slept over");
    co_return;
}

void async_main(IOContext& ctx, std::stop_token stop_token)
{
    spawn(ctx, hello("1"), std::move(stop_token));
    ctx.run();
}

int main()
{
    std::stop_source stop_source{};
    otter::async::IOContext ctx{};
    std::thread t{ async_main, std::ref(ctx), stop_source.get_token() };

    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_source.request_stop();
    spdlog::info("Stop requested");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    spdlog::info("Stopping context");
    ctx.stop();
    t.join();
    spdlog::info("Thread joined");

    return EXIT_SUCCESS;
}
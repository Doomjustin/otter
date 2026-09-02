#include <chrono>
#include <cstdlib>
#include <stop_token>

#include <spdlog/spdlog.h>

#include <otter/async.h>

#include "async/io_context.h"

using namespace otter::async;
using namespace std::literals;

auto shutdown_monitor(std::stop_source stop_source) -> Task<>
{
    SignalSet signals{ signals::interrupt, signals::terminate };
    auto res = co_await signals.async_wait();

    if (!res) {
        spdlog::error("signal wait failed: {}", res.error().message());
        co_return;
    }

    spdlog::info("received terminate signal {}", *res);
    stop_source.request_stop();
}

auto hello() -> Task<>
{
    auto res = co_await timeout(sleep_for(5s), 1ms);
    if (!res) {
        spdlog::error("{}", res.error().message());
        co_return;
    }

    spdlog::info("after");
}

void worker(std::stop_token stop_token)
{
    auto ctx = IOContext{};
    spawn(ctx, hello(), std::move(stop_token));
    ctx.run();
}

int main()
{
    signals::block(signals::interrupt, signals::terminate);
    std::stop_source process_stop_source{};
    auto stop_token = process_stop_source.get_token();

    auto thread_count = std::thread::hardware_concurrency();
    std::vector<std::jthread> threads(thread_count - 1);
    for (auto& thread : threads)
        thread = std::jthread(worker, stop_token);

    IOContext ctx{};
    spawn(ctx, shutdown_monitor(process_stop_source));
    spawn(ctx, hello(), stop_token);
    ctx.run();

    return EXIT_SUCCESS;
}
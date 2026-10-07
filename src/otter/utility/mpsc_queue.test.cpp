#include "mpsc_queue.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace {

class TestNode final : public otter::MPSCQueueNode {
public:
    explicit TestNode(int id_value)
      : id(id_value)
    {}

    int id;
};

auto drain_ids(otter::MPSCQueue<TestNode>& queue) -> std::vector<int>
{
    std::vector<int> ids;
    auto* current = queue.pop_all();
    while (current != nullptr) {
        ids.push_back(current->id);
        current = static_cast<TestNode*>(current->mpsc_next.load(std::memory_order_relaxed));
    }

    return ids;
}

} // namespace

TEST_CASE("mpsc_queue returns true only when queue transitions from empty", "[mpsc_queue]")
{
    otter::MPSCQueue<TestNode> queue;
    TestNode first(1);
    TestNode second(2);
    TestNode third(3);

    CHECK(queue.empty());
    CHECK(queue.push(&first));
    CHECK_FALSE(queue.push(&second));
    CHECK_FALSE(queue.push(&third));
    CHECK_FALSE(queue.empty());
}

TEST_CASE("mpsc_queue pop_all returns nodes in producer insertion order", "[mpsc_queue]")
{
    otter::MPSCQueue<TestNode> queue;
    TestNode first(1);
    TestNode second(2);
    TestNode third(3);

    queue.push(&first);
    queue.push(&second);
    queue.push(&third);

    CHECK(drain_ids(queue) == std::vector<int>{ 1, 2, 3 });
    CHECK(queue.empty());
}

TEST_CASE("mpsc_queue can be reused after draining", "[mpsc_queue]")
{
    otter::MPSCQueue<TestNode> queue;
    TestNode first(1);
    TestNode second(2);

    queue.push(&first);
    CHECK(drain_ids(queue) == std::vector<int>{ 1 });
    CHECK(queue.empty());

    CHECK(queue.push(&second));
    CHECK(drain_ids(queue) == std::vector<int>{ 2 });
    CHECK(queue.empty());
}

TEST_CASE("mpsc_queue pop_all on empty queue returns null", "[mpsc_queue]")
{
    otter::MPSCQueue<TestNode> queue;

    CHECK(queue.pop_all() == nullptr);
    CHECK(queue.empty());
}

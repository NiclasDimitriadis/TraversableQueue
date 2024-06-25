#define DOCTEST_CONFIG_IMPLEMENT

#include <algorithm>

#include "TraversableQueue.hpp"
#include "doctest.h"

TEST_CASE("testing basic enqueue and dequeue functionality"){
    using ShortQueueType = TraversableQueue::TraversableQueue<int, 4>;
    ShortQueueType test_queue;
    // dequeueing from empty queue needs to return empty std::optional<int>
    const auto deq_1 = test_queue.dequeue();
    CHECK(!deq_1.has_value());

    // fill queue, needs to return true every time
    const bool enq_1 = test_queue.enqueue(1);
    CHECK(enq_1);
    const bool enq_2 = test_queue.enqueue(2);
    CHECK(enq_1);
    const bool enq_3 = test_queue.enqueue(3);
    CHECK(enq_1);
    const bool enq_4 = test_queue.enqueue(4);
    CHECK(enq_1);

    // return false upon queue overflow
    const bool enq_5 = test_queue.enqueue(5);
    CHECK(!enq_5);

    // empty queue, needs to return std::optional<int> containing all the values previously enqueued
    const auto deq_2 = test_queue.dequeue();
    CHECK(deq_2.value() == 1);
    const auto deq_3 = test_queue.dequeue();
    CHECK(deq_3.value() == 2);
    const auto deq_4 = test_queue.dequeue();
    CHECK(deq_4.value() == 3);
    const auto deq_5 = test_queue.dequeue();
    CHECK(deq_5.value() == 4);

    // queue should now be empty, dequeueing should return an empty std::optional<int> again
    const auto deq_6 = test_queue.dequeue();
    CHECK(!deq_6.has_value());
};

TEST_CASE("testing traversal of queue content"){
    using LongQueueType = TraversableQueue::TraversableQueue<std::uint32_t, 128>;
    constexpr auto dequeue_n = [](std::int64_t state, std::uint32_t& element)->std::tuple<std::int64_t, bool, bool>{
        if (state <= 0){
            return std::make_tuple(0, false, true);
        }
        const bool exceeds_state = element > state;
        const auto memorize_state = state;
        state = std::max((int64_t)0, state - element);
        element -= memorize_state * exceeds_state;
        return std::make_tuple(state, !exceeds_state, state == 0);
    };

    // enqueue 20 four times
    LongQueueType test_queue;
    for(size_t i = 0; i < 4; ++i){
        const bool enq_res = test_queue.enqueue(20);
    };

    // use dequeue_n to dequeue 50
    const std::int64_t traverse_ret = test_queue.traverse_content<dequeue_n>(50);
    CHECK(traverse_ret == 0);

    // should leave one entry of 10
    const auto deq_1 = test_queue.dequeue();
    CHECK(deq_1.value() == 10);

    // and another entry of 20
    const auto deq_2 = test_queue.dequeue();
    CHECK(deq_2.value() == 20);

    // queue should then be empty
    const auto deq_3 = test_queue.dequeue();
    CHECK(!deq_3.has_value());
};

int main() {
  doctest::Context context;
  context.run();
}

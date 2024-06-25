#pragma once

#include <bit>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "function_signature.hpp"
#include "param_pack.hpp"

namespace TraversableQueue{
template<typename ContentType, size_t length>
requires std::is_default_constructible_v<ContentType> && std::is_nothrow_copy_assignable_v<ContentType> && std::is_nothrow_copy_constructible_v<ContentType> && (std::has_single_bit(length))
struct TraversableQueue{
private:
    std::int64_t n_enqueues = 0;
    std::int64_t n_dequeues = 0;
    static constexpr size_t alignment = std::max(size_t(64), alignof(ContentType));
    const std::unique_ptr<ContentType[]> memory_ptr;
    const std::span<ContentType, length> memory_span;

    template<auto, typename...>
    auto traverse_content_logic(){};

    template<auto traverser, typename TraverseStateType, typename... Args>
    requires (sizeof...(Args) <= 1) &&
    std::is_invocable_r_v<std::tuple<TraverseStateType, bool, bool>, decltype(traverser), TraverseStateType, ContentType&, Args...>
    TraverseStateType traverse_content_logic(TraverseStateType, Args...) noexcept;

public:
    explicit TraversableQueue();
    ~TraversableQueue() = default;
    TraversableQueue(TraversableQueue&) = delete;
    TraversableQueue(TraversableQueue&&) = delete;
    TraversableQueue& operator=(TraversableQueue&) = delete;
    TraversableQueue& operator=(TraversableQueue&&) = delete;
    [[nodiscard]] bool enqueue(const ContentType&) noexcept;
    [[nodiscard]] std::optional<ContentType> dequeue() noexcept;

    // case if f takes a single additional argument
    template<auto f>
    requires (function_signature::arg_types_t<f>::size == 3)
    std::tuple_element_t<0, function_signature::ret_type_t<f>>
    traverse_content(function_signature::arg_types_t<f>::template index_t<0>,
        function_signature::arg_types_t<f>::template truncate_front_t<3>::template index_t<0>) noexcept;

    // case if f takes no additional argument
    template<auto f>
    requires (function_signature::arg_types_t<f>::size == 2)
    std::tuple_element_t<0, function_signature::ret_type_t<f>>
    traverse_content(function_signature::arg_types_t<f>::template index_t<0>) noexcept;
};
}

#define TEMPL_PARAMS template<typename ContentType, size_t length>\
requires std::is_default_constructible_v<ContentType> &&\
std::is_nothrow_copy_assignable_v<ContentType> && std::is_nothrow_copy_constructible_v<ContentType> &&\
(std::has_single_bit(length))

#define TEMPL_SPECIALIZATION TraversableQueue::TraversableQueue<ContentType, length>

TEMPL_PARAMS
TEMPL_SPECIALIZATION::TraversableQueue():memory_ptr{new(std::align_val_t{alignment}) ContentType[length]()}, memory_span{memory_ptr.get(), length}{};

TEMPL_PARAMS
bool TEMPL_SPECIALIZATION::enqueue(const ContentType& enqueue_content) noexcept{
    const bool slots_available = (this->n_enqueues - this->n_dequeues) < length;
    if(slots_available) this->memory_span[n_enqueues % length] = enqueue_content;
    this->n_enqueues += slots_available;
    return slots_available;
};

TEMPL_PARAMS
std::optional<ContentType> TEMPL_SPECIALIZATION::dequeue() noexcept{
    const bool empty_queue = this->n_enqueues == this->n_dequeues;
    const auto ret_opt = !empty_queue ? std::make_optional(this->memory_span[this->n_dequeues % length]) : std::nullopt;
    this->n_dequeues += !empty_queue;
    return ret_opt;
};

TEMPL_PARAMS
template<auto traverser, typename TraverseStateType, typename... Args>
requires(sizeof...(Args) <= 1) && std::is_invocable_r_v<std::tuple<TraverseStateType,bool,bool>,decltype(traverser),TraverseStateType,ContentType&,Args...>
TraverseStateType TEMPL_SPECIALIZATION::traverse_content_logic(TraverseStateType traversal_state, Args... args) noexcept{
    for (std::int64_t index = this->n_dequeues; index < this->n_enqueues; ++index){
        ContentType& element = this->memory_span[index % length];
        bool dequeue{false}, end_iteration{false};
        std::tie(traversal_state, dequeue, end_iteration) = traverser(traversal_state, element, std::forward<Args>(args)...);
        this->n_dequeues += dequeue;
        if(end_iteration)
            break;
    };
return traversal_state;
};

TEMPL_PARAMS
template<auto f>
requires (function_signature::arg_types_t<f>::size == 3)
std::tuple_element_t<0, function_signature::ret_type_t<f>>
TEMPL_SPECIALIZATION::traverse_content(function_signature::arg_types_t<f>::template index_t<0> initial_state,
    function_signature::arg_types_t<f>::template truncate_front_t<3>::template index_t<0> arg) noexcept{
    using Arg = function_signature::arg_types_t<f>::template truncate_front_t<3>::template index_t<0>;
    using State = function_signature::arg_types_t<f>::template index_t<0>;
    return traverse_content_logic<f, State, Arg>(initial_state, std::forward<Arg>(arg));
};

TEMPL_PARAMS
template<auto f>
requires (function_signature::arg_types_t<f>::size == 2)
std::tuple_element_t<0, function_signature::ret_type_t<f>>
TEMPL_SPECIALIZATION::traverse_content(function_signature::arg_types_t<f>::template index_t<0> initial_state) noexcept{
    using State = function_signature::arg_types_t<f>::template index_t<0>;
    return traverse_content_logic<f, State>(initial_state);
};

#undef TEMPL_PARAMS
#undef TEMPL_SPECIALIZATION

## TraversableQueue
Simple queue that provides the user with an interface to query, manipulate and dequeue its content.

- non thread safe queue using a ring buffer
- provides a member function template with a callable non-type template parameter to statically generate the traversal functionality
- performant but compromises on encapsulation/seperation of concerns
- relies on functionality from `function_signature` and `param_pack` contained in the `TMP_lib` repo
- to be used for adding order-matching capabilites to the `CppOrderBook2` project

#### TraversableQueue class template
`template<typename ContentType, size_t length>`<br>
`requires std::is_default_constructible_v<ContentType> && std::is_nothrow_copy_assignable_v<ContentType> &&`<br> `std::is_nothrow_copy_constructible_v<ContentType> && (std::has_single_bit(length))`<br>
`struct TraversableQueue`
- `ContentType`: type that will be enqueued
- type constaints ensure that queue elements can be default constructed when the queue object is constructed, (nothrow) copy assigned when an element is enqueued and (nothrow) copy constructed when it's dequeued
- `length`: max number of element the queue can contain, determines the size of the ring buffer
- constrained to powers of two to prevent costly modulus operations when computing indices in ring buffer

### interfaces

#### `enqueue`
`bool enqueue(const ContentType&) noexcept`
- enqueues a new element
- returns `true` if enqueueing was successful, `false` if the queue was already filled to capacity so the new element could not be enqueued

#### `dequeue`
`std::optional<ContentType> dequeue() noexcept`
- dequeues an element from the queue
- returns a `std::optional` containing the dequeued element if the queue wasn't empty and an empty `std::optional` if it was

#### the traverser object
- centerpiece of the `traverse_content` method templates
- has to be `constexpr`
- has to be callable with a first argument of type `TraverseStateType`, a second of type `ContentType&` and an optional third argument of the users choosing (more arguments can obviously be passed if the second argument is a sum type)
- `TraverseStateType` is provided by the user via the choice of `traveser`
- has to return `std::tuple<TraverseStateType, bool, bool>`
- when iterating over content, `TraverseStateType` is returned during every iteration and passed to `traverser` in the next, thus tranferring state across iterations
- `traverser` may change the queue's content via its second argument
- the first `bool` in the return type indicates whether an element is to be dequeued, the second `bool` stops the traversal if `true` is returned
- if the first `bool` is `false` once (i.e. entry in currenct iteration is not dequeued), returning `true` in a later iteration constitutes an error, it's up to the user providing `traverser` to ensure that doesn't happen

#### `traverse_content`
`template<auto traverser>`<br>
`requires (function_signature::arg_types_t<traverser>::size == 3)`<br>
`std::tuple_element_t<0, function_signature::ret_type_t<traverser>>`<br>
`traverse_content(function_signature::arg_types_t<traverser>::template index_t<0> initial_state, function_signature::arg_types_t<traverser>::template index_t<3> arg) noexcept`
- method template that takes a `traverser` object as a non-type parameter
- the properties of `traverser` described above are checked statically
- the method takes `TraverseStateType` as its first argument, signifying the initial state of traversal
- the second argument is optional and can be chosen by user (method template overload omitting the second argument is provided and has an analogous signature)
- type checks use `function_signature` and `param_pack` from the `TMP_lib` repo



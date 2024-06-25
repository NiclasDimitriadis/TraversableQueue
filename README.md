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

#### `bool enqueue(const ContentType&) noexcept`
- enqueues a new element
- returns `true` if enqueueing was successful, `false` if the queue was already filled to capacity so the new element could not be enqueued

#### `std::optional<ContentType> dequeue() noexcept`
- dequeues an element from the queue
- returns a `std::optional` containing the dequeued element if the queue wasn't empty and an empty `std::optional` if it was


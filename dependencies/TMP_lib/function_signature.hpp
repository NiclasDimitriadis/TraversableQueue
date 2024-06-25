#pragma once

#include <functional>

#include "helpers.hpp"
#include "param_pack.hpp"

namespace function_signature{

template<typename...>
struct function_signature{
    static_assert(false);
};

template<typename F, typename Ret, typename... Args>
struct function_signature<Ret (F::*) (Args...) const>{
    using return_type = Ret;
    using arg_types = param_pack::type_pack_t<Args...>;
};

// deduce return type from a callable type
template<auto f>
// require that std::function can be specialized by f
requires helpers::specializes_class_template_v<std::function, decltype(std::function(f))>
// construct std::function f, deduce std::function's type,
// deduce the type/signature of pointer to function type's operator() and pass it to
// ret_type class-template to deduce the return type
using ret_type_t = function_signature<decltype(&decltype(std::function(f))::operator())>::return_type;


// deduce argument types from a callable type
template<auto f>
// require that std::function can be specialized by f
requires helpers::specializes_class_template_v<std::function, decltype(std::function(f))>
// construct std::function from f, deduce std::function's type,
// deduce the type/signature of pointer to function type's operator() and pass it to
// ret_type class-template to deduce the argument types
using arg_types_t = function_signature<decltype(&decltype(std::function(f))::operator())>::arg_types;
};

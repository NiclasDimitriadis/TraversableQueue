template <typename... Ts> struct type_pack_t {
private:
  template <typename> struct append_logic {
    static_assert(false);
  };

  template <typename... Us> struct append_logic<type_pack_t<Us...>> {
    using type = type_pack_t<Ts..., Us...>;
  };

  template <size_t n, typename... Us>
    requires(n <= sizeof...(Us))
  struct truncate_front_logic {
    static_assert(false);
  };

  // empty pack type if n equals the number of type contained
  template <typename... Us> struct truncate_front_logic<sizeof...(Ts), Us...> {
    using type = type_pack_t<>;
  };

  // regular recursion step
  template <size_t n, typename U, typename... Us>
    requires(sizeof...(Us) != sizeof...(Ts) - n - 1)
  struct truncate_front_logic<n, U, Us...> {
    using type = truncate_front_logic<n, Us...>::type;
  };

  // stop recursion when n first entries have been removed
  template <typename... Us>
  struct truncate_front_logic<sizeof...(Ts) - sizeof...(Us), Us...> {
    using type = type_pack_t<Us...>;
  };

  template <size_t n, typename...>
    requires(n <= sizeof...(Ts))
  struct truncate_back_logic {
    static_assert(false);
  };

  template <size_t n, typename U, typename... Us>
  struct truncate_back_logic<n, U, Us...> {
    using type = type_pack_t<U>::template append_t<
        typename truncate_back_logic<n, Us...>::type>;
  };

  template <typename U, typename... Us>
  struct truncate_back_logic<sizeof...(Us), U, Us...> {
    using type = type_pack_t<U>;
  };

  template <size_t index, typename... Us> struct reverse_logic {
    using type = type_pack_t<pack_index_t<index, Us...>>::template append_t<
        typename reverse_logic<index - 1, Us...>::type>;
  };

  template <typename... Us> struct reverse_logic<0, Us...> {
    using type = type_pack_t<pack_index_t<0, Us...>>;
  };

  template <size_t index> struct reverse_logic<index> {
    using type = type_pack_t<>;
  };

  template <template <typename...> class, typename...>
  struct functor_map_logic {
    using type = type_pack_t<>;
  };

  template <template <typename...> class Class_Template, typename Fst,
            typename... Tail>
  struct functor_map_logic<Class_Template, Fst, Tail...> {
    using type = type_pack_t<Class_Template<Fst>>::template append_t<
        typename functor_map_logic<Class_Template, Tail...>::type>;
  };

  template <template <typename...> class Class_Template, typename Last>
  struct functor_map_logic<Class_Template, Last> {
    using type = type_pack_t<Class_Template<Last>>;
  };

  template <template <typename...> class Class_Template> struct functor_map {
    using type = functor_map_logic<Class_Template, Ts...>::type;
  };

  template <typename Other_Pack>
    requires helpers::specializes_class_template_v<type_pack_t, Other_Pack>
  struct same_size {
    static constexpr bool value = sizeof...(Ts) == Other_Pack::size;
  };

  template <size_t index> struct extract_nth_type {
    template <typename Type_Pack>
    using type = typename Type_Pack::index_t<index>;
  };

  template <template <typename...> class Class_Template, const size_t index,
            typename Nested_Pack>
  struct pure_step {
    // generate type_pack_t of all types at index 'index' of all the packs (get
    // 'row')
    using intermediate_pack = Nested_Pack::template functor_map_t<
        extract_nth_type<index>::template type>;
    // apply template to intermediate_pack
    using type =
        intermediate_pack::template specialize_template_t<Class_Template>;
  };

  template <template <typename...> class Class_Template, const size_t index,
            typename Nested_Pack, typename Res_Pack>
  struct pure_logic {
    using iter_res_pack = type_pack_t<
        typename pure_step<Class_Template, index, Nested_Pack>::type>;
    using type =
        pure_logic<Class_Template, index + 1, Nested_Pack,
                   typename Res_Pack::template append_t<iter_res_pack>>::type;
  };

  template <template <typename...> class Class_Template, typename Nested_Pack,
            typename Res_Pack>
  struct pure_logic<Class_Template, sizeof...(Ts), Nested_Pack, Res_Pack> {
    using type = Res_Pack;
  };

  template <template <typename...> class Class_Templ, typename... Other_Packs>
    requires type_pack_t<Other_Packs...>::template
  functor_map_t<same_size>::template fold_t<
      helpers::And, std::true_type>::value struct applicative_pure {
    using type = pure_logic<Class_Templ, 0,
                            type_pack_t<type_pack_t<Ts...>, Other_Packs...>,
                            type_pack_t<>>::type;
  };

  template <template <typename...> class, typename...>
  struct monadic_bind_logic {
    using type = type_pack_t<>;
  };

  template <template <typename...> class Class_Template, typename Fst,
            typename... Tail>
    requires helpers::specializes_class_template_v<type_pack_t,
                                                   Class_Template<Fst>>
  struct monadic_bind_logic<Class_Template, Fst, Tail...> {
    using type = Class_Template<Fst>::template append_t<
        typename monadic_bind_logic<Class_Template, Tail...>::type>;
  };

  template <template <typename...> class Class_Template, typename Last>
    requires helpers::specializes_class_template_v<type_pack_t,
                                                   Class_Template<Last>>
  struct monadic_bind_logic<Class_Template, Last> {
    using type = Class_Template<Last>;
  };

  template <template <typename...> class Class_Template> struct monadic_bind {
    using type = monadic_bind_logic<Class_Template, Ts...>::type;
  };

  template <size_t...> struct subset {
    using type = type_pack_t<>;
  };

  template <size_t fst, size_t... tail> struct subset<fst, tail...> {
    using type = type_pack_t<pack_index_t<fst, Ts...>>::template append_t<
        typename subset<tail...>::type>;
  };

  template <size_t last> struct subset<last> {
    using type = type_pack_t<pack_index_t<last, Ts...>>;
  };

  template <size_t... is> using subset_t = subset<is...>::type;

  template <typename...> struct subset_from_pack {
    static_assert(false);
  };

  template <typename Uint, size_t... is>
    requires std::unsigned_integral<Uint>
  struct subset_from_pack<non_type_pack<Uint, is...>> {
    using type = subset_t<is...>;
  };

  template <typename NT_Pack>
  using subset_from_pack_t = subset_from_pack<NT_Pack>::type;

  template <typename T_, size_t> struct split_logic {
    static_assert(false);
  };

  template <size_t len, typename Uint, size_t... is>
    requires(sizeof...(is) > len) &&
            (sizeof...(is) % len == 0) && std::unsigned_integral<Uint>
  struct split_logic<non_type_pack<Uint, is...>, len> {
  private:
    using arg_pack = non_type_pack_t<is...>;
    using subset_pack = arg_pack::template head_t<len>;
    using pass_down_pack = arg_pack::template truncate_front_t<len>;

  public:
    using type =
        type_pack_t<subset_from_pack_t<subset_pack>>::template append_t<
            typename split_logic<pass_down_pack, len>::type>;
  };

  template <size_t len, typename Uint, size_t... is>
    requires(sizeof...(is) == len) && std::unsigned_integral<Uint>
  struct split_logic<non_type_pack<Uint, is...>, len> {
  private:
    using subset_pack = non_type_pack<Uint, is...>;

  public:
    using type = type_pack_t<subset_from_pack_t<subset_pack>>;
  };

  template <template <typename...> class F, typename Init, typename...>
  struct fold_logic {
    using type = F<Init>;
  };

  template <template <typename...> class F, typename First, typename Second,
            typename... Tail>
    requires helpers::specializes_class_template_v<F, F<First, Second>>
  struct fold_logic<F, First, Second, Tail...> {
    using type = fold_logic<F, F<First, Second>, Tail...>::type;
  };

  template <template <typename...> class F, typename Second_To_Last,
            typename Last>
    requires helpers::specializes_class_template_v<F, F<Second_To_Last, Last>>
  struct fold_logic<F, Second_To_Last, Last> {
    using type = F<Second_To_Last, Last>;
  };

  template <template <typename...> class, size_t>
  struct apply_templ_to_nr_combs {
    using type = type_pack_t<>;
  };

  template <template <typename...> class templ, size_t order>
    requires(order > 0) && (sizeof...(Ts) > 0)
  struct apply_templ_to_nr_combs<templ, order> {
  private:
    template <typename T>
    using feed_template = T::template specialize_template_t<templ>;

    using combinations = generate_non_type_pack_t<
        non_rep_combinations::non_rep_index_combinations_t<sizeof...(Ts),
                                                           order>>;
    using split_up =
        type_pack_t<Ts...>::template split_logic<combinations, order>::type;

  public:
    using type = split_up::template functor_map_t<feed_template>;
  };

public:
  static constexpr size_t size = sizeof...(Ts);

  // extract i-th entry in parameter pack
  template <size_t i> using index_t = pack_index_t<i, Ts...>;

  // append another tpye_pack_t
  template <typename Append_Pack>
  using append_t = append_logic<Append_Pack>::type;

  // removes first n entries
  template <size_t n>
  using truncate_front_t = truncate_front_logic<n, Ts...>::type;

  // removes last n entries
  template <size_t n>
  using truncate_back_t = truncate_back_logic<n, Ts...>::type;

  // yields first n elements
  template <size_t n> using head_t = truncate_back_t<sizeof...(Ts) - n>;

  // yields last n elements
  template <size_t n> using tail_t = truncate_front_t<sizeof...(Ts) - n>;

  // use Ts as type-arguments to specialize other type template
  template <template <typename...> class Class_Template>
  using specialize_template_t = Class_Template<Ts...>;

  // reverses order of Ts...
  using reverse_t = reverse_logic<sizeof...(Ts) - 1, Ts...>::type;

  // apply a class template to Ts in a functorial way
  template <template <typename...> class Class_Template>
  using functor_map_t = functor_map<Class_Template>::type;

  // apply a class template to multiple type_pack_t-types as an applicative
  // functor
  template <template <typename...> class Class_Template, typename... Type_Packs>
  using applicative_pure_t = applicative_pure<Class_Template, Type_Packs>::type;

  // apply a class template to Ts in a monadic way
  template <template <typename...> class Class_Template>
  using monadic_bind_t = monadic_bind<Class_Template>::type;

  // generates a nested type pack of subsets of Ts... of length len from indices
  // specified in a non type pack of type size_t
  template <typename NT_Pack, size_t len>
  using split_t = split_logic<NT_Pack, len>::type;

  // generates a type by folding Ts... via a class template
  template <template <typename...> class F, typename Init>
  using fold_t = fold_logic<F, Init, Ts...>::type;

  // splits types into non-repeating groups of a certain size and uses each to
  // specialize a template
  template <template <typename...> class Templ, size_t order>
  using apply_templ_to_nr_combs_t = apply_templ_to_nr_combs<Templ, order>::type;
};

template <typename> struct generate_type_pack {};

template <template <typename...> class Class_Templ, typename... Ts>
struct generate_type_pack<Class_Templ<Ts...>> {
  using type = type_pack_t<Ts...>;
};

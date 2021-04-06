#ifndef BOOST_VARIANT2_HPP
#define BOOST_VARIANT2_HPP

#include <boost/variant2/variant.hpp>

#include "geometry.hpp"

namespace boost { namespace geometry { namespace traits {

template <typename ...Ts>
struct tag<boost::variant2::variant<Ts...>>
{
    typedef dynamic_geometry_tag type;
};

template <typename ...Ts>
struct visit<boost::variant2::variant<Ts...>>
{
    template <typename Function, typename Variant>
    static void apply(Function && function, Variant & variant)
    {
        boost::variant2::visit(std::forward<Function>(function), variant);
    }
};

template <typename ...Ts, typename ...Us>
struct visit<boost::variant2::variant<Ts...>, boost::variant2::variant<Us...>>
{
    template <typename Function, typename Variant1, typename Variant2>
    static void apply(Function && function, Variant1 & variant1, Variant2 & variant2)
    {
        boost::variant2::visit(std::forward<Function>(function), variant1, variant2);
    }
};

template <typename ...Ts>
struct geometry_types<boost::variant2::variant<Ts...>>
{
    typedef util::type_sequence<Ts...> type;
};

}}} // namespace boost::geometry::traits

#endif // BOOST_VARIANT2_HPP

#ifndef BOOST_VARIANT_HPP
#define BOOST_VARIANT_HPP

#include <boost/variant.hpp>

#include "geometry.hpp"

namespace boost { namespace geometry { namespace traits {

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct tag<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    typedef dynamic_geometry_tag type;
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct visit<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    template <typename Function>
    struct visitor : boost::static_visitor<>
    {
        visitor(Function function)
            : m_function(function)
        {}

        template <typename Geometry>
        void operator()(Geometry & geometry)
        {
            m_function(geometry);
        }

        Function m_function;
    };

    template <typename Function, typename Variant>
    static void apply(Function function, Variant & variant)
    {
        visitor<Function> visitor(function);
        boost::apply_visitor(visitor, variant);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T), BOOST_VARIANT_ENUM_PARAMS(typename U)>
struct visit<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>, boost::variant<BOOST_VARIANT_ENUM_PARAMS(U)>>
{
    template <typename Function>
    struct visitor : boost::static_visitor<>
    {
        visitor(Function function)
            : m_function(function)
        {}

        template <typename Geometry1, typename Geometry2>
        void operator()(Geometry1 & geometry1, Geometry2 & geometry2)
        {
            m_function(geometry1, geometry2);
        }

        Function m_function;
    };

    template <typename Function, typename Variant1, typename Variant2>
    static void apply(Function function, Variant1 & variant1, Variant2 & variant2)
    {
        visitor<Function> visitor(function);
        boost::apply_visitor(visitor, variant1, variant2);
    }
};

template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
struct geometry_types<boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)>>
{
    // TODO: not correct, this will put here null_types in VS2015
    typedef util::type_sequence<BOOST_VARIANT_ENUM_PARAMS(T)> type;
};

}}} // namespace boost::geometry::traits

#endif // BOOST_VARIANT_HPP

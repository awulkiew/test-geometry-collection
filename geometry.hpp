#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>

namespace boost { namespace geometry {

struct dynamic_geometry_tag {};

// NOTE: A tuple of geometries could be treated as GeometryCollection.
//   For practical reasons though these are now separate concepts.
//   In fact since now we can move() StaticGeometries then an algorithm returning a GC
//   would probably use TupledGeometry internally and then it would simply be returned
//   or StaticGeometries would be moved to GeometryCollection.
// NOTE: Right now these are not used. TupledGeometry is a tuple type containing StaticGeometries.
//   They could be a separate Geometries kinds but then for consistency access would have to be
//   implemented as traits as opposed to utilities that we have now.
//struct tuple_geometry_tag {};
//struct tupled_geometry_tag {};

namespace util
{

template <typename T>
struct is_dynamic_geometry
    : std::is_same<dynamic_geometry_tag, typename tag<T>::type>
{};

template <typename T>
struct is_geometry_collection
    : std::is_same<geometry_collection_tag, typename tag<T>::type>
{};

} // namespace util


namespace traits {

// TODO: Alternatives:
// - free function
//    template <typename Visitor, typename ...Variants>
//    auto visit(Visitor &&, Variants && ...) {}
//
// - additional Enable tparam
//    template <bool Enable, typename ...DynamicGeometries>
//    struct visit {};

template <typename ...DynamicGeometries>
struct visit
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for these DynamicGeometries types.",
        DynamicGeometries...);
};

// By default call 1-parameter visit for each geometry
template <typename DynamicGeometry1, typename DynamicGeometry2>
struct visit<DynamicGeometry1, DynamicGeometry2>
{
    template <typename Function, typename Variant1, typename Variant2>
    static void apply(Function && function, Variant1 & variant1, Variant2 & variant2)
    {
        visit<util::remove_cref_t<Variant1>>::apply([&](auto & g1)
        {
            visit<util::remove_cref_t<Variant2>>::apply([&](auto & g2)
            {
                function(g1, g2);
            }, variant1);
        }, variant2);
    }
};

// By default treat GeometryCollection as a range of DynamicGeometries
// NOTE: The name was derived from std::iter_swap (there is also another function likt that?)
template <typename GeometryCollection>
struct visit_iterator
{
    template <typename Function, typename Iterator>
    static void apply(Function && function, Iterator iterator)
    {
        using value_t = typename boost::range_value<GeometryCollection>::type;
        visit<value_t>::apply(std::forward<Function>(function), *iterator);
    }
};

template <typename Geometry, typename Tag = typename geometry::tag<Geometry>::type>
struct geometry_types_impl
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};

// By default treat GeometryCollection as a range of DynamicGeometries
template <typename Geometry>
struct geometry_types_impl<Geometry, geometry_collection_tag>
    : geometry_types_impl<typename boost::range_value<Geometry>::type>
{};

// DynamicGeometry or GeometryCollection
template <typename Geometry>
struct geometry_types
    : geometry_types_impl<Geometry>
{};

// TODO: also implement push_back taking r-value reference

template <typename Range>
struct emplace_back
{
    // When specializing it'd be enough to only implement it for one argument
    // because we'll use it to pass one object of type potentially different
    // than range_value but which range_value can be constructed from.
    template <typename ...Args>
    static inline void apply(typename rvalue_type<Range>::type range,
                             Args&&... args)
    {
        range.emplace_back(std::forward<Args>(args)...);
    }
};

} // namespace traits

}} // namespace boost::geometry

#define BOOST_GEOMETRY_REGISTER_GEOMETRY_COLLECTION(GeometryCollection, ...) \
namespace boost { namespace geometry { namespace traits { \
    template<> struct tag<GeometryCollection> { typedef geometry_collection_tag type; }; \
    template<> struct geometry_types<GeometryCollection> { typedef util::type_sequence<__VA_ARGS__> type; }; \
}}}

#define BOOST_GEOMETRY_REGISTER_DYNAMIC_GEOMETRY(DynamicGeometry, ...) \
namespace boost { namespace geometry { namespace traits { \
    template<> struct tag<DynamicGeometry> { typedef dynamic_geometry_tag type; }; \
    template<> struct geometry_types<DynamicGeometry> { typedef util::type_sequence<__VA_ARGS__> type; }; \
}}}

namespace boost { namespace geometry {

namespace range {

// TODO: also implement push_back taking r-value reference

template <typename Range, typename ...Args>
inline void emplace_back(Range & rng, Args&&... args)
{
    geometry::traits::emplace_back<Range>::apply(rng, std::forward<Args>(args)...);
}

} // namespace range

namespace model {

template
<
    typename DynamicGeometry,
    template <typename, typename> class Container = std::vector,
    template <typename> class Allocator = std::allocator
>
class geometry_collection
    : public Container<DynamicGeometry, Allocator<DynamicGeometry>>
{
    typedef Container<DynamicGeometry, Allocator<DynamicGeometry>> base_type;

public:
    geometry_collection() = default;

    template <typename Iterator>
    geometry_collection(Iterator begin, Iterator end)
        : base_type(begin, end)
    {}

    geometry_collection(std::initializer_list<DynamicGeometry> l)
        : base_type(l.begin(), l.end())
    {}
};

} // namespace model

namespace traits {

template
<
    typename DynamicGeometry,
    template <typename, typename> class Container,
    template <typename> class Allocator
>
struct tag<model::geometry_collection<DynamicGeometry, Container, Allocator>>
{
    typedef geometry_collection_tag type;
};

} // namespace traits

namespace detail {

//template <typename T>
//using enable_if_geometry_t = std::enable_if_t<boost::geometry::util::is_geometry<std::remove_const_t<T>>::value, int>;

//template <typename T>
//using enable_if_not_geometry_t = std::enable_if_t<boost::geometry::util::is_geometry<std::remove_const_t<T>>::value, int>;

} // namespace detail

namespace dispatch
{

template <typename Geometry, bool IsConst>
struct check<Geometry, dynamic_geometry_tag, IsConst>
    // TODO traits::visit
{};

template <typename Geometry, bool IsConst>
struct check<Geometry, geometry_collection_tag, IsConst>
    // TODO range of dynamic geometries
{};

template <typename Geometry>
struct clear<Geometry, dynamic_geometry_tag>
{
    static void apply(Geometry& geometry)
    {
        traits::visit<Geometry>::apply([](auto & g)
        {
            clear<std::remove_reference_t<decltype(g)>>::apply(g);
        }, geometry);
    }
};

template <typename Geometry>
struct clear<Geometry, geometry_collection_tag>
    : detail::clear::collection_clear<Geometry>
{};

}

namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct visit_one
{
    template <typename F, typename G>
    static void apply(F && f, G & g)
    {
        f(g);
    }
};

template <typename Geometry>
struct visit_one<Geometry, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};

template <typename Geometry>
struct visit_one<Geometry, dynamic_geometry_tag>
{
    template <typename F, typename Geom>
    static void apply(F && function, Geom & geom)
    {
        traits::visit
            <
                util::remove_cref_t<Geom>
            >::apply(std::forward<F>(function), geom);
    }
};

} // namespace dispatch


// NOTE - This function mimics std::visit
//   We could change the order of the arguments but I'm not sure if that's a good idea.
template <typename UnaryFunction, typename Geometry>
inline void visit(UnaryFunction && function, Geometry & geometry)
{
    dispatch::visit_one
        <
            Geometry
        >::apply(std::forward<UnaryFunction>(function), geometry);
}


namespace dispatch
{

template <typename Geometry1, typename Geometry2, typename Tag1 = typename tag<Geometry1>::type, typename Tag2 = typename tag<Geometry2>::type>
struct visit_two
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        f(geom1, geom2);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag2>
struct visit_two<Geometry1, Geometry2, void, Tag2>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry1 type.",
        Geometry1);
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct visit_two<Geometry1, Geometry2, Tag1, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry2 type.",
        Geometry2);
};

template <typename Geometry1, typename Geometry2>
struct visit_two<Geometry1, Geometry2, void, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for these types.",
        Geometry1, Geometry2);
};

template <typename Geometry1, typename Geometry2, typename Tag2>
struct visit_two<Geometry1, Geometry2, dynamic_geometry_tag, Tag2>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit<util::remove_cref_t<G1>>::apply([&](auto & g1)
        {
            f(g1, geom2);
        }, geom1);
    }
};

template <typename Geometry1, typename Geometry2, typename Tag1>
struct visit_two<Geometry1, Geometry2, Tag1, dynamic_geometry_tag>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit<util::remove_cref_t<G2>>::apply([&](auto & g2)
        {
            f(geom1, g2);
        }, geom2);
    }
};

template <typename Geometry1, typename Geometry2>
struct visit_two<Geometry1, Geometry2, dynamic_geometry_tag, dynamic_geometry_tag>
{
    template <typename F, typename G1, typename G2>
    static void apply(F && f, G1 & geom1, G2 & geom2)
    {
        traits::visit
            <
                util::remove_cref_t<G1>, util::remove_cref_t<G2>
            >::apply([&](auto & g1, auto & g2)
            {
                f(g1, g2);
            }, geom1, geom2);
    }
};

} // namespace dispatch


// NOTE - This function mimics std::visit
//   We could change the order of the arguments but I'm not sure if that's a good idea.
template <typename UnaryFunction, typename Geometry1, typename Geometry2>
inline void visit(UnaryFunction && function, Geometry1 & geometry1, Geometry2 & geometry2)
{
    dispatch::visit_two
        <
            Geometry1, Geometry2
        >::apply(std::forward<UnaryFunction>(function), geometry1, geometry2);
}


namespace dispatch
{

template <typename Geometry, typename Tag = typename tag<Geometry>::type>
struct visit_breadth_first
{
    template <typename F, typename G>
    static void apply(F f, G & g)
    {
        f(g);
    }
};

template <typename Geometry>
struct visit_breadth_first<Geometry, void>
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};

template <typename Geometry>
struct visit_breadth_first<Geometry, dynamic_geometry_tag>
{
    template <typename Geom, typename F>
    static void apply(F function, Geom & geom)
    {
        traits::visit<util::remove_cref_t<Geom>>::apply([&](auto & g)
        {
            visit_breadth_first<decltype(g)>::apply(function, g);
        }, geom);
    }
};

template <typename Geometry>
struct visit_breadth_first<Geometry, geometry_collection_tag>
{
    template <typename F, typename Geom>
    static void apply(F function, Geom & geom)
    {
        using iter_t = typename boost::range_iterator<Geom>::type;
        std::deque<iter_t> queue;

        iter_t it = boost::begin(geom);
        iter_t end = boost::end(geom);
        for(;;)
        {
            for (; it != end; ++it)
            {
                traits::visit_iterator<util::remove_cref_t<Geom>>::apply([&](auto & g)
                {
                    visit_or_enqueue(function, g, queue, it);
                }, it);
            }

            if (queue.empty())
            {
                break;
            }
        
            // Alternatively store a pointer to GeometryCollection
            // so this call can be avoided.
            traits::visit_iterator<util::remove_cref_t<Geom>>::apply([&](auto & g)
            {
                set_iterators(g, it, end);
            }, queue.front());
            queue.pop_front();
        }
    }

private:
    template <typename F, typename Geom, typename Iterator, std::enable_if_t<util::is_geometry_collection<Geom>::value, int> = 0>
    static void visit_or_enqueue(F &, Geom &, std::deque<Iterator> & queue, Iterator iter)
    {
        queue.push_back(iter);
    }
    template <typename F, typename Geom, typename Iterator, std::enable_if_t<! util::is_geometry_collection<Geom>::value, int> = 0>
    static void visit_or_enqueue(F & f, Geom & g, std::deque<Iterator> & , Iterator)
    {
        f(g);
    }

    template <typename Geom, typename Iterator, std::enable_if_t<util::is_geometry_collection<Geom>::value, int> = 0>
    static void set_iterators(Geom & g, Iterator & first, Iterator & last)
    {
        first = boost::begin(g);
        last = boost::end(g);
    }
    template <typename Geom, typename Iterator, std::enable_if_t<!util::is_geometry_collection<Geom>::value, int> = 0>
    static void set_iterators(Geom &, Iterator &, Iterator &)
    {}
};

} // namespace dispatch

// NOTE - This function mimics std::visit
//   We could change the order of the arguments but I'm not sure if that's a good idea.
//   This algorithm is similar to std::for_each so it could be called e.g. std::visit_each or
//   std::for_each_visit but it doesn't work the same way, i.e. traverses recursive
//   GeometryCollections.
// NOTE: This algorithm works for all Geometries.
//   It visits DynamicGeometries and calls function for StaticGeometries.
//   TODO: Or should this work only for GeometryCollection?
// NOTE: The number of elements visited may be different than the size of the
//   top-most GeometryCollection.
template <typename UnaryFunction, typename Geometry>
inline void visit_breadth_first(UnaryFunction function, Geometry & geometry)
{
    dispatch::visit_breadth_first<Geometry>::apply(function, geometry);
}


}} // namespace boost::geometry

#endif // GEOMETRY_HPP

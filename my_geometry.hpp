#ifndef MY_GEOMETRY_HPP
#define MY_GEOMETRY_HPP

#include "geometry.hpp"

enum MyGeometryId { MyPointId, MyLinestringId, MyGCollId };

struct MyGeometry
{
    virtual MyGeometryId which() const = 0;
};
struct MyPoint : MyGeometry
{
    MyGeometryId which() const { return MyPointId; }

    double x, y;
};
struct MyLinestring : MyGeometry, std::vector<MyPoint>
{
    MyGeometryId which() const { return MyLinestringId; }
};
struct MyGColl : MyGeometry, std::vector<std::unique_ptr<MyGeometry>>
{
    MyGeometryId which() const { return MyGCollId; }

    MyGColl() = default;
    template <typename G>
    MyGColl(std::initializer_list<G> l)
    {
        for (auto const& g : l)
            push_back(std::unique_ptr<MyGeometry>(new G(g)));
    }
};

BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint, double, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_LINESTRING(MyLinestring)

namespace boost { namespace geometry { namespace traits {

template <>
struct tag<MyGColl>
{
    typedef geometry_collection_tag type;
};

template <>
struct visit_iterator<MyGColl>
{
    template <typename Function, typename Iterator>
    static void apply(Function function, Iterator iterator)
    {
        auto & ptr = *iterator;
        using unique_ptr_t = std::remove_reference_t<decltype(ptr)>;

        if (ptr->which() == MyPointId)
            function(static_cast<util::transcribe_const_t<unique_ptr_t, MyPoint>&>(*ptr));
        else if (ptr->which() == MyLinestringId)
            function(static_cast<util::transcribe_const_t<unique_ptr_t, MyLinestring>&>(*ptr));
        else if (ptr->which() == MyGCollId)
            function(static_cast<util::transcribe_const_t<unique_ptr_t, MyGColl>&>(*ptr));
    }
};

template <>
struct geometry_types<MyGColl>
{
    typedef util::type_sequence<MyPoint, MyLinestring, MyGColl> type;
};

template <>
struct emplace_back<MyGColl>
{
    template <typename Geometry>
    static inline void apply(MyGColl& range, Geometry&& geometry)
    {
        range.emplace_back(std::unique_ptr<MyGeometry>(new Geometry(std::forward<Geometry>(geometry))));
    }
};

}}} // namespace boost::geometry::traits

#endif // MY_GEOMETRY_HPP

#ifndef MY_GEOMETRY1_HPP
#define MY_GEOMETRY1_HPP

#include "geometry.hpp"

enum MyGeometry1Id { MyPoint1Id, MyLinestring1Id, MyGColl1Id };

struct MyGeometry1Base
{
    virtual MyGeometry1Id which() const = 0;
};
struct MyGeometry1
{
    template <typename G>
    MyGeometry1(G const& g) : ptr(new G(g)) {}
    std::shared_ptr<MyGeometry1Base> ptr;
};
struct MyPoint1 : MyGeometry1Base
{
    MyGeometry1Id which() const { return MyPoint1Id; }

    double x, y;
};
struct MyLinestring1 : MyGeometry1Base, std::vector<MyPoint1>
{
    MyGeometry1Id which() const { return MyLinestring1Id; }
};
struct MyGColl1 : MyGeometry1Base, std::vector<MyGeometry1>
{
    MyGeometry1Id which() const { return MyGColl1Id; }

    MyGColl1() = default;
    MyGColl1(std::initializer_list<MyGeometry1> l) : std::vector<MyGeometry1>(l) {}
};

BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint1, double, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_LINESTRING(MyLinestring1)

namespace boost { namespace geometry { namespace traits {

template <>
struct tag<MyGeometry1>
{
    typedef dynamic_geometry_tag type;
};

template <>
struct visit<MyGeometry1>
{
    template <typename Function, typename Geometry>
    static void apply(Function function, Geometry & geometry)
    {
        if (geometry.ptr->which() == MyPoint1Id)
            function(static_cast<util::transcribe_const_t<Geometry, MyPoint1>&>(*geometry.ptr));
        else if (geometry.ptr->which() == MyLinestring1Id)
            function(static_cast<util::transcribe_const_t<Geometry, MyLinestring1>&>(*geometry.ptr));
        else if (geometry.ptr->which() == MyGColl1Id)
            function(static_cast<util::transcribe_const_t<Geometry, MyGColl1>&>(*geometry.ptr));
    }
};

template <>
struct geometry_types<MyGeometry1>
{
    typedef util::type_sequence<MyPoint1, MyLinestring1, MyGColl1> type;
};

template <>
struct tag<MyGColl1>
{
    typedef geometry_collection_tag type;
};

}}} // namespace boost::geometry::traits

#endif // MY_GEOMETRY1_HPP

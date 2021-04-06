#ifndef MY_GEOMETRY2_HPP
#define MY_GEOMETRY2_HPP

#include "geometry.hpp"

struct MyPoint2;
struct MyLinestring2;
struct MyGColl2;

struct MyVisitorBase2
{
    virtual void apply(MyPoint2 &) = 0;
    virtual void apply(MyLinestring2 &) = 0;
    virtual void apply(MyGColl2 &) = 0;
};

struct MyVisitorBase2const
{
    virtual void apply(MyPoint2 const&) = 0;
    virtual void apply(MyLinestring2 const&) = 0;
    virtual void apply(MyGColl2 const&) = 0;
};

struct MyGeometryBase2
{
    virtual void apply(MyVisitorBase2 & vis) = 0;
    virtual void apply(MyVisitorBase2const & vis) const = 0;
};
struct MyGeometry2
{
    template <typename G>
    MyGeometry2(G const& g) : ptr(new G(g)) {}
    std::shared_ptr<MyGeometryBase2> ptr;
};
struct MyPoint2 : MyGeometryBase2
{
    void apply(MyVisitorBase2 & vis) { vis.apply(*this); }
    void apply(MyVisitorBase2const & vis) const { vis.apply(*this); }

    double x, y;
};
struct MyLinestring2 : MyGeometryBase2, std::vector<MyPoint2>
{
    void apply(MyVisitorBase2 & vis) { vis.apply(*this); }
    void apply(MyVisitorBase2const & vis) const { vis.apply(*this); }
};
struct MyGColl2 : MyGeometryBase2, std::vector<MyGeometry2>
{
    void apply(MyVisitorBase2 & vis) { vis.apply(*this); }
    void apply(MyVisitorBase2const & vis) const { vis.apply(*this); }

    MyGColl2() = default;
    MyGColl2(std::initializer_list<MyGeometry2> l) : std::vector<MyGeometry2>(l) {}
};

template <typename UnaryFunction>
struct MyVisitorBase2Impl : MyVisitorBase2
{
    MyVisitorBase2Impl(UnaryFunction function)
        : m_function(function) {}

    void apply(MyPoint2 & g) { m_function(g); }
    void apply(MyLinestring2 & g) { m_function(g); }
    void apply(MyGColl2 & g) { m_function(g); }
    
    UnaryFunction m_function;
};

template <typename UnaryFunction>
struct MyVisitorBase2Implconst : MyVisitorBase2const
{
    MyVisitorBase2Implconst(UnaryFunction function)
        : m_function(function) {}

    void apply(MyPoint2 const& g) { m_function(g); }
    void apply(MyLinestring2 const& g) { m_function(g); }
    void apply(MyGColl2 const& g) { m_function(g); }

    UnaryFunction m_function;
};

BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint2, double, cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_LINESTRING(MyLinestring2)

namespace boost { namespace geometry { namespace traits {

template <>
struct tag<MyGeometry2>
{
    typedef dynamic_geometry_tag type;
};

template <>
struct visit<MyGeometry2>
{
    template <typename Function, typename Geometry>
    static void apply(Function function, Geometry & geometry)
    {
        MyVisitorBase2Impl<Function> visitor(function);
        static_cast<MyGeometryBase2&>(*geometry.ptr).apply(visitor);
    }

    template <typename Function, typename Geometry>
    static void apply(Function function, Geometry const& geometry)
    {
        MyVisitorBase2Implconst<Function> visitor(function);
        static_cast<MyGeometryBase2 const&>(*geometry.ptr).apply(visitor);
    }
};

template <>
struct geometry_types<MyGeometry2>
{
    typedef util::type_sequence<MyPoint2, MyLinestring2, MyGColl2> type;
};

template <>
struct tag<MyGColl2>
{
    typedef geometry_collection_tag type;
};

}}} // namespace boost::geometry::traits

#endif // MY_GEOMETRY2_HPP

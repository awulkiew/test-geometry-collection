#include "boost_any.hpp"
#include "boost_variant.hpp"
#include "boost_variant2.hpp"
#include "geometry.hpp"
#include "my_geometry.hpp"
#include "my_geometry1.hpp"
#include "my_geometry2.hpp"

#include <iostream>

namespace bg = boost::geometry;

using point = bg::model::point<double, 2, bg::cs::cartesian>;
using linestring = bg::model::linestring<point>;
using polygon = bg::model::polygon<point>;
using mpoint = bg::model::multi_point<point>;
using mlinestring = bg::model::multi_linestring<linestring>;
using mpolygon = bg::model::multi_polygon<polygon>;

struct geometry_collection1;
using variant1 = boost::variant<point, linestring, polygon, mpoint, mlinestring, mpolygon, geometry_collection1>;
struct geometry_collection1 : std::vector<variant1>
{
    geometry_collection1() = default;
    geometry_collection1(std::initializer_list<variant1> l) : std::vector<variant1>(l) {}
};

struct geometry_collection2;
using variant2 = boost::variant2::variant<point, linestring, polygon, mpoint, mlinestring, mpolygon, geometry_collection2>;
struct geometry_collection2 : std::vector<variant2>
{
    geometry_collection2() = default;
    geometry_collection2(std::initializer_list<variant2> l) : std::vector<variant2>(l) {}
};

BOOST_GEOMETRY_REGISTER_GEOMETRY_COLLECTION(geometry_collection1, point, linestring, polygon, mpoint, mlinestring, mpolygon, geometry_collection1)
BOOST_GEOMETRY_REGISTER_GEOMETRY_COLLECTION(geometry_collection2, point, linestring, polygon, mpoint, mlinestring, mpolygon, geometry_collection2)
BOOST_GEOMETRY_REGISTER_DYNAMIC_GEOMETRY(boost::any, point, linestring, polygon, mpoint, mlinestring, mpolygon, bg::model::geometry_collection<boost::any>)

template <typename Geometry>
void print(Geometry & geometry)
{
    bg::visit_breadth_first([&](auto & g) {
        std::cout << bg::wkt(g) << ' ';
    }, geometry);
    std::cout << std::endl;
}

template <typename Geometry1, typename Geometry2>
void test_visit(Geometry1 & geometry1, Geometry2 & geometry2)
{
    bg::visit([](auto & g) {
        std::cout << typeid(g).name() << std::endl;
    }, geometry1);
    bg::visit([](auto & g) {
        std::cout << "  " << typeid(g).name() << std::endl;
    }, geometry2);
    bg::visit([](auto & g1, auto & g2) {
        std::cout << "  " << typeid(g1).name() << "\n    " << typeid(g2).name() << std::endl;
    }, geometry1, geometry2);
}

template <typename Geometry, typename Element, std::enable_if_t<bg::util::is_geometry_collection<Geometry>::value, int> = 0>
void emplace_back_if_gc(Geometry & geom, Element && el)
{
    bg::range::emplace_back(geom, std::forward<Element>(el));
}

template <typename Geometry, typename Element, std::enable_if_t<!bg::util::is_geometry_collection<Geometry>::value, int> = 0>
void emplace_back_if_gc(Geometry & , Element && )
{}

int main()
{
    using variant = boost::variant<point, linestring, polygon, mpoint, mlinestring, mpolygon>;
    bg::model::geometry_collection<variant> gc{ linestring(), point(), point(), point(), point() };
    bg::model::geometry_collection<variant> const cgc{ point(), linestring(), point(), point(), point() };

    variant1 g1{ geometry_collection1{ point(), point(), linestring(), point(), point() } };
    variant1 const cg1{ geometry_collection1{ point(), point(), point(), linestring(), point() } };

    variant2 g2{ geometry_collection2{ point(), point(), point(), point(), linestring() } };
    variant2 const cg2{ geometry_collection2{ linestring(), point(), point(), point(), point() } };

    MyGColl mgc { MyPoint(), MyPoint(), MyPoint(), MyPoint(), MyPoint() };
    MyGColl const cmgc{ MyPoint(), MyPoint(), MyPoint(), MyPoint(), MyPoint() };

    MyGeometry1 g3{ MyGColl1{ MyPoint1(), MyLinestring1(), MyPoint1(), MyPoint1(), MyPoint1() } };
    MyGeometry1 const cg3{ MyGColl1{ MyPoint1(), MyPoint1(), MyLinestring1(), MyPoint1(), MyPoint1() } };

    MyGeometry2 g4{ MyGColl2{ MyPoint2(), MyPoint2(), MyPoint2(), MyLinestring2(), MyPoint2() } };
    MyGeometry2 const cg4{ MyGColl2{ MyPoint2(), MyPoint2(), MyPoint2(), MyPoint2(), MyLinestring2() } };

    boost::any g5{ bg::model::geometry_collection<boost::any>{ linestring(), point(), point(), point(), point() } };
    boost::any const cg5{ bg::model::geometry_collection<boost::any>{ point(), linestring(), point(), point(), point() } };

    print(gc);
    print(cgc);
    print(g1);
    print(cg1);
    print(g2);
    print(cg2);
    print(mgc);
    print(cmgc);
    print(g3);
    print(cg3);
    print(g4);
    print(cg4);
    print(g5);
    print(cg5);
    
    bg::clear(gc);
    bg::clear(g1);
    bg::clear(g2);
    bg::clear(mgc);
    bg::clear(g3);
    bg::clear(g4);
    bg::clear(g5);

    test_visit(g1, cg1);
    test_visit(g2, cg2);
    test_visit(g3, cg3);
    test_visit(g4, cg4);
    test_visit(g5, cg5);

    // emplace_back works for dynamic geometries with explicit constructor
    bg::range::emplace_back(gc, point());
    bg::visit([](auto & g) { emplace_back_if_gc(g, point()); }, g1);
    bg::visit([](auto & g) { emplace_back_if_gc(g, point()); }, g2);
    bg::range::emplace_back(mgc, MyPoint());
    bg::visit([](auto & g) { emplace_back_if_gc(g, MyPoint1()); }, g3);
    bg::visit([](auto & g) { emplace_back_if_gc(g, MyPoint2()); }, g4);
    bg::visit([](auto & g) { emplace_back_if_gc(g, point()); }, g5);

    bg::traits::geometry_types<variant>::type gt;
    bg::traits::geometry_types<variant1>::type gt1;
    bg::traits::geometry_types<variant2>::type gt2;
    bg::traits::geometry_types<MyGColl>::type gt3;
    bg::traits::geometry_types<MyGeometry1>::type gt4;
    bg::traits::geometry_types<MyGeometry2>::type gt5;
    bg::traits::geometry_types<boost::any>::type gt6;

    boost::ignore_unused(gt, gt1, gt2, gt3, gt4, gt5, gt6);

    return 0;
}

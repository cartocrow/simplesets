#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "library/settings.h"
#include "library/partition_algorithm.h"
#include "library/drawing_algorithm.h"

#include <cartocrow/circle_segment_helpers/cs_render_helpers.h>
#include "cartocrow/renderer/svg_renderer.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

using namespace cartocrow;
using namespace cartocrow::simplesets;

/// The partitions are computed up to time maxCover * dilationRadius.
constexpr double maxCover = 8;


/// SVG path data of a RenderPath, with y-axis pointing down.
/// Same as renderPathToSVGCommands(p, false), but with enough digits for large coordinates such as
/// Web Mercator (renderPathToSVGCommands uses the default six significant digits, which rounds those to 10 m).
std::string svgPathData(const RenderPath& p) {
    std::stringstream ss;
    ss << std::setprecision(12);

    Point<Inexact> from;
    for (RenderPath::Command c : p.commands()) {
        if (std::holds_alternative<RenderPath::MoveTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::MoveTo>(c).m_to;
            ss << "M " << to.x() << " " << to.y() << " ";
            from = to;
        } else if (std::holds_alternative<RenderPath::LineTo>(c)) {
            Point<Inexact> to = std::get<RenderPath::LineTo>(c).m_to;
            ss << "L " << to.x() << " " << to.y() << " ";
            from = to;
        } else if (std::holds_alternative<RenderPath::ArcTo>(c)) {
            Point<Inexact> center = std::get<RenderPath::ArcTo>(c).m_center;
            Point<Inexact> to = std::get<RenderPath::ArcTo>(c).m_to;
            bool clockwise = std::get<RenderPath::ArcTo>(c).m_clockwise;
            double radius = sqrt((center - to).squared_length());
            bool centerOnLeft = CGAL::orientation(from, to, center) == CGAL::LEFT_TURN;
            int largeArc = (centerOnLeft == clockwise) ? 1 : 0;
            // The y-axis points down, so the sweep direction is mirrored.
            int sweep = clockwise ? 0 : 1;
            ss << "A " << radius << " " << radius << " 0 " << largeArc << " " << sweep << " " << to.x() << " "
               << to.y() << " ";
            from = to;
        } else if (std::holds_alternative<RenderPath::Close>(c)) {
            ss << "Z ";
        }
    }

    return ss.str();
}

/// Collects the paths drawn by DilatedPatternDrawing as JavaScript objects {kind, category, pathData}.
/// The colors in the DrawSettings passed to the drawing should encode the category in the red channel (see categoryColors).
/// Kind "faceFill" is a filled piece of a shape; kind "faceStroke" is a piece of the contour of a shape.
/// Contour pieces have no category: drawFaceStroke does not set a fill color, so it cannot be recovered.
class PathCollector : public GeometryRenderer {
    int m_mode = 0;
    int m_category = -1;

public:
    val elements = val::array();

    void draw(const RenderPath& p) override {
        val obj = val::object();
        if (m_mode & fill) {
            obj.set("kind", std::string("faceFill"));
            obj.set("category", m_category);
        } else {
            obj.set("kind", std::string("faceStroke"));
        }
        obj.set("pathData", svgPathData(p));
        elements.call<void>("push", obj);
    }
    void setMode(int mode) override { m_mode = mode; }
    void setFill(Color color) override { m_category = color.r; }

    void draw(const Point<Inexact>&) override {}
    void draw(const Circle<Inexact>&) override {}
    void draw(const Ellipse&) override {}
    void draw(const CubicBezierSpline&) override {}
    void draw(const Line<Inexact>&) override {}
    void draw(const Ray<Inexact>&) override {}
    void draw(const Halfplane<Inexact>&) override {}
    void drawText(const Point<Inexact>&, const std::string&, bool) override {}
    void pushStyle() override {}
    void popStyle() override {}
    void setStroke(Color, double, bool) override {}
    void setStrokeOpacity(int) override {}
    void setFillOpacity(int) override {}
    void setPointSize(double, bool) override {}
    void setClipPath(const RenderPath&) override {}
    void setClipping(bool) override {}
    void setLineJoin(LineJoin) override {}
    void setLineCap(LineCap) override {}
    void setHorizontalTextAlignment(HorizontalTextAlignment) override {}
    void setVerticalTextAlignment(VerticalTextAlignment) override {}
    void setFontFamily(std::string) override {}
    void setFontSize(double) override {}
    void setFontWeight(bool) override {}
    void useDefaultFont() override {}
};

/// Draw settings whose color for category i is (i, 0, 0), so that PathCollector can recover the category.
DrawSettings categoryColors(const std::vector<CatPoint>& points) {
    DrawSettings ds;
    ds.whiten = 0;
    unsigned int numCategories = 0;
    for (const auto& p : points) {
        numCategories = std::max(numCategories, p.category + 1);
    }
    for (int i = 0; i < numCategories; ++i) {
        ds.colors.push_back(Color{i, 0, 0});
    }
    return ds;
}

/// A pattern as a JavaScript object {kind, category, points, closed, coordinates}:
/// - kind is "point", "matching", "island" or "bank", and points are the x and y of its points, in order,
///   so that the pattern can be reconstructed with patternFromJS;
/// - coordinates are the x and y of the vertices of its polygon (closed) or polyline, for drawing it.
val patternToJS(const PolyPattern& pattern) {
    val coordinates = val::array();
    bool closed = std::visit(
        [&coordinates](const auto& poly) {
            for (auto v = poly.vertices_begin(); v != poly.vertices_end(); ++v) {
                coordinates.call<void>("push", v->x());
                coordinates.call<void>("push", v->y());
            }
            return std::is_same_v<std::decay_t<decltype(poly)>, Polygon<Inexact>>;
        },
        pattern.poly());
    std::string kind = "point";
    if (dynamic_cast<const Bank*>(&pattern)) {
        kind = "bank";
    } else if (dynamic_cast<const Island*>(&pattern)) {
        kind = "island";
    } else if (dynamic_cast<const Matching*>(&pattern)) {
        kind = "matching";
    }
    val points = val::array();
    for (const auto& catPoint : pattern.catPoints()) {
        points.call<void>("push", catPoint.point.x());
        points.call<void>("push", catPoint.point.y());
    }

    val obj = val::object();
    obj.set("kind", kind);
    obj.set("category", pattern.category());
    obj.set("points", points);
    obj.set("closed", closed);
    obj.set("coordinates", coordinates);
    return obj;
}

/// The pattern described by a JavaScript object from patternToJS.
std::shared_ptr<PolyPattern> patternFromJS(val obj) {
    auto category = obj["category"].as<unsigned int>();
    auto kind = obj["kind"].as<std::string>();
    val points = obj["points"];
    std::vector<CatPoint> catPoints;
    auto n = points["length"].as<unsigned int>();
    for (unsigned int i = 0; i + 1 < n; i += 2) {
        catPoints.emplace_back(category, Point<Inexact>(points[i].as<double>(), points[i + 1].as<double>()));
    }
    if (kind == "bank") return std::make_shared<Bank>(catPoints);
    if (kind == "island") return std::make_shared<Island>(catPoints);
    if (kind == "matching") return std::make_shared<Matching>(catPoints.at(0), catPoints.at(1));
    return std::make_shared<SinglePoint>(catPoints.at(0));
}

/// Wraps the SimpleSets pipeline (partition, then drawing) for use from JavaScript.
/// All inputs and outputs are plain JavaScript values, so that results can be sent between a web worker and the main thread.
/// Points are in SVG coordinates (y-axis pointing down).
/// Nothing is stored between calls other than the input and the settings, so the partition and the drawing can be
/// computed by different instances (for example in different web workers).
class SimpleSets {
    // Input
    std::vector<CatPoint> m_points;

    // Settings
    GeneralSettings m_gs;
    PartitionSettings m_ps;
    ComputeDrawingSettings m_cds;

public:
    SimpleSets() {}

    GeneralSettings getGeneralSettings() const {
        return m_gs;
    }

    ComputeDrawingSettings getComputeDrawingSettings() const {
        return m_cds;
    }

    PartitionSettings getPartitionSettings() const {
        return m_ps;
    }

    void setGeneralSettings(const GeneralSettings& gs) {
        m_gs = gs;
    }

    void setPartitionSettings(const PartitionSettings& ps) {
        m_ps = ps;
    }

    void setComputeDrawingSettings(const ComputeDrawingSettings& cds) {
        m_cds = cds;
    }

    /// Set the input points: an array of objects {category, x, y}.
    void setPoints(val points) {
        m_points.clear();
        auto n = points["length"].as<unsigned int>();
        for (unsigned int i = 0; i < n; ++i) {
            val p = points[i];
            m_points.emplace_back(p["category"].as<unsigned int>(),
                                  Point<Inexact>(p["x"].as<double>(), p["y"].as<double>()));
        }
    }

    /// Compute the sequence of partitions.
    /// \p progress is a JavaScript function that is called with a number between 0 and 1.
    /// Returns an object {times, patterns, steps} that describes the sequence:
    /// - times[i] is the cover (time divided by the dilation radius) at which partition i was created;
    ///   the times do not decrease (see below);
    /// - patterns[id] is a pattern as described at patternToJS;
    /// - steps[i] is {removed, added}: the ids of the patterns that partition i removes from and adds to partition i - 1.
    val computePartitions(val progress) {
        auto dilationRadius = CGAL::to_double(m_gs.dilationRadius());
        auto partitions = partition(m_points, m_gs, m_ps, maxCover * dilationRadius,
                                    [&progress](double p) { progress(p); });

        val times = val::array();
        val patterns = val::array();
        val steps = val::array();
        // The partitions share patterns, so patterns are identified by address.
        std::unordered_map<const PolyPattern*, int> ids;
        const Partition* previous = nullptr;
        // An event can lie in the past: a merge can enable merges at a smaller cover than the current time, which are
        // then handled immediately (see Section 4 of the SimpleSets paper). Its partition is created at the current
        // time of the simulation, not at the time of the event, so use the maximum of the event times so far.
        // This matches the Kotlin implementation, which selects the partitions before the first one with time >= cover.
        Number<Inexact> currentTime = 0;
        for (const auto& [time, partition] : partitions) {
            currentTime = std::max(currentTime, time);
            times.call<void>("push", currentTime / dilationRadius);

            val added = val::array();
            std::unordered_set<const PolyPattern*> current;
            for (const auto& pattern : partition) {
                current.insert(pattern.get());
                if (!ids.contains(pattern.get())) {
                    int id = ids.size();
                    ids[pattern.get()] = id;
                    patterns.call<void>("push", patternToJS(*pattern));
                    added.call<void>("push", id);
                }
            }
            val removed = val::array();
            if (previous) {
                for (const auto& pattern : *previous) {
                    if (!current.contains(pattern.get())) {
                        removed.call<void>("push", ids[pattern.get()]);
                    }
                }
            }

            val step = val::object();
            step.set("removed", removed);
            step.set("added", added);
            steps.call<void>("push", step);
            previous = &partition;
        }

        val result = val::object();
        result.set("times", times);
        result.set("patterns", patterns);
        result.set("steps", steps);
        return result;
    }

    /// Compute the drawing of a partition: an array of patterns as described at patternToJS.
    /// \p progress is a JavaScript function that is called with a number between 0 and 1.
    /// Returns an object {elements, shapes}:
    /// - elements is an array of objects {kind, category, pathData}, in the order in which they should be drawn.
    ///   If the enclosing shapes can be stacked in a total order, each element is a complete shape of kind "shape".
    ///   Otherwise, the shapes are drawn per face of their arrangement; see PathCollector.
    /// - shapes is empty if there is a total order; otherwise it contains an object {category, pathData} with the
    ///   complete shape of each pattern, so that a single category can be shown on top of the others.
    /// - warnings are messages about problems that did not stop the computation (see DilatedPatternDrawing::m_warnings).
    val computeDrawing(val patterns, val progress) const {
        Partition partition;
        std::vector<CatPoint> points;
        auto n = patterns["length"].as<unsigned int>();
        for (unsigned int i = 0; i < n; ++i) {
            partition.push_back(patternFromJS(patterns[i]));
            const auto& catPoints = partition.back()->catPoints();
            std::copy(catPoints.begin(), catPoints.end(), std::back_inserter(points));
        }
        for (int i = 0; i < points.size(); ++i) {
            for (int j = i + 1; j < points.size(); ++j) {
                if (points[i].category != points[j].category &&
                    CGAL::squared_distance(points[i].point, points[j].point) < 4 * m_gs.pointSize * m_gs.pointSize) {
                    throw std::runtime_error("Points of different category are too close together.");
                }
            }
        }

        // Painting takes under 1% of the time of DilatedPatternDrawing, so progress is only reported for the latter.
        DilatedPatternDrawing dpd(partition, m_gs, m_cds, [&progress](double p) { progress(p); });
        SimpleSetsPainting painting(dpd, DrawSettings{});
        auto polys = painting.getStackedPolygons();

        val result = val::object();
        val shapes = val::array();
        result.set("shapes", shapes);
        val warnings = val::array();
        for (const auto& warning : dpd.m_warnings) {
            warnings.call<void>("push", warning);
        }
        result.set("warnings", warnings);

        if (!polys) {
            // As in SimpleSetsPainting::paint: first fill all faces, then draw all contours.
            PathCollector collector;
            auto ds = categoryColors(points);
            for (auto fit = dpd.m_arr.faces_begin(); fit != dpd.m_arr.faces_end(); ++fit) {
                if (fit->is_unbounded()) continue;
                dpd.drawFaceFill(fit.ptr(), collector, m_gs, ds);
            }
            for (auto fit = dpd.m_arr.faces_begin(); fit != dpd.m_arr.faces_end(); ++fit) {
                if (fit->is_unbounded()) continue;
                dpd.drawFaceStroke(fit.ptr(), collector, m_gs, ds);
            }
            result.set("elements", collector.elements);

            for (int i = 0; i < dpd.m_dilated.size(); ++i) {
                val obj = val::object();
                obj.set("category", dpd.m_dilated[i].category());
                obj.set("pathData", svgPathData(renderPath(painting.patternPolygon(i))));
                shapes.call<void>("push", obj);
            }
            return result;
        }

        val elements = val::array();
        for (const auto& [category, poly] : *polys) {
            val obj = val::object();
            obj.set("kind", std::string("shape"));
            obj.set("category", category);
            obj.set("pathData", svgPathData(renderPath(poly)));
            elements.call<void>("push", obj);
        }
        result.set("elements", elements);
        return result;
    }
};

/// Call f, and turn C++ exceptions into JavaScript errors with their message.
/// With -fwasm-exceptions, a C++ exception reaches JavaScript as a WebAssembly.Exception, which has no readable
/// message and cannot be sent from a web worker to the main thread.
template <typename F> val withJSErrors(F f) {
    try {
        return f();
    } catch (const std::exception& e) {
        val::global("Error").new_(std::string(e.what())).throw_();
    } catch (...) {
        val::global("Error").new_(std::string("Unknown C++ exception")).throw_();
    }
}

EMSCRIPTEN_BINDINGS(simplesets) {
    // Settings structs
    value_object<GeneralSettings>("GeneralSettings")
            .field("pointSize", &GeneralSettings::pointSize)
            .field("inflectionLimit", &GeneralSettings::inflectionLimit)
            .field("maxBendAngle", &GeneralSettings::maxBendAngle)
            .field("maxTurnAngle", &GeneralSettings::maxTurnAngle);

    value_object<PartitionSettings>("PartitionSettings")
            .field("banks", &PartitionSettings::banks)
            .field("islands", &PartitionSettings::islands)
            .field("regularityDelay", &PartitionSettings::regularityDelay)
            .field("intersectionDelay", &PartitionSettings::intersectionDelay)
            .field("admissibleRadiusFactor", &PartitionSettings::admissibleRadiusFactor);

    value_object<ComputeDrawingSettings>("ComputeDrawingSettings")
            .field("cutoutRadiusFactor", &ComputeDrawingSettings::cutoutRadiusFactor)
            .field("smooth", &ComputeDrawingSettings::smooth)
            .field("smoothingRadiusFactor", &ComputeDrawingSettings::smoothingRadiusFactor);

    // SimpleSets class
    class_<SimpleSets>("SimpleSets")
            .constructor<>()
            .function("setPoints", &SimpleSets::setPoints)
            .function("computePartitions", optional_override([](SimpleSets& s, val progress) {
                return withJSErrors([&] { return s.computePartitions(progress); });
            }))
            .function("computeDrawing", optional_override([](SimpleSets& s, val patterns, val progress) {
                return withJSErrors([&] { return s.computeDrawing(patterns, progress); });
            }))
            .function("getComputeDrawingSettings", &SimpleSets::getComputeDrawingSettings)
            .function("getPartitionSettings", &SimpleSets::getPartitionSettings)
            .function("getGeneralSettings", &SimpleSets::getGeneralSettings)
            .function("setComputeDrawingSettings", &SimpleSets::setComputeDrawingSettings)
            .function("setPartitionSettings", &SimpleSets::setPartitionSettings)
            .function("setGeneralSettings", &SimpleSets::setGeneralSettings);
}

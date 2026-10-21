#include <iostream>

#include "library/parse_input.h"
#include "library/settings.h"
#include "library/partition_algorithm.h"
#include "library/drawing_algorithm.h"

#include <cartocrow/circle_segment_helpers/cs_render_helpers.h>
#include "cartocrow/renderer/svg_renderer.h"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <nlohmann/json.hpp>

using namespace emscripten;

using namespace cartocrow;
using namespace cartocrow::simplesets;

using json = nlohmann::json;

// Wrapper functions

Number<Inexact> dilationRadius(const GeneralSettings& gs) {
    return CGAL::to_double(gs.dilationRadius());
}

Number<Inexact> pointStrokeWeight(
        const DrawSettings& ds,
        const GeneralSettings& gs)
{
    return ds.pointStrokeWeight(gs);
}

Number<Inexact> contourStrokeWeight(
        const DrawSettings& ds,
        const GeneralSettings& gs)
{
    return ds.contourStrokeWeight(gs);
}

Color getColor(
        const DrawSettings& ds,
        int category)
{
    return ds.getColor(category);
}

class SimpleSets {
    // Input
    std::vector<CatPoint> m_points;

    // Settings
    GeneralSettings m_gs;
    PartitionSettings m_ps;
    ComputeDrawingSettings m_cds;
    DrawSettings m_ds;
private:

    // Computed
    std::optional<std::vector<std::pair<Number<Inexact>, Partition>>> m_partitions;
    std::shared_ptr<DilatedPatternDrawing> m_dpd = nullptr;

    val catPointToJS(const CatPoint& cp) const {
        val pt = val::object();
        pt.set("category", cp.category);
        pt.set("x", cp.point.x());
        pt.set("y", cp.point.y());
        return pt;
    }

public:
    SimpleSets() {}

    GeneralSettings getGeneralSettings() const {
        return m_gs;
    }

    DrawSettings getDrawSettings() const {
        return m_ds;
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

    void setDrawSettings(const DrawSettings& ds) {
        m_ds = ds;
    }

    void setComputeDrawingSettings(const ComputeDrawingSettings& cds) {
        m_cds = cds;
    }

    void setPartitionSettings(const PartitionSettings& ps) {
        m_ps = ps;
    }

    void load(std::string textInput) {
        std::cout << "Hatseflats" << std::endl;
        m_points = parseCatPoints(textInput);
        std::cout << "<Hatseflats/>" << std::endl;
        m_partitions = std::nullopt;
        m_dpd = nullptr;
    }

    val getPoints() const {
        val arr = val::array();

        int i = 0;
        for (const auto& pt : m_points) {
            arr.set(i++, catPointToJS(pt));
        }

        return arr;
    }

    void computePartitions() {
        m_partitions = partition(m_points, m_gs, m_ps, 8 * CGAL::to_double(m_gs.dilationRadius()));
    }

    Partition& getPartition(double cover) {
//    void getPartition(double cover) {
        if (!m_partitions.has_value()) throw std::runtime_error("No partitions have been computed!");

        Partition* thePartition;
        bool found = false;
        for (auto& [time, partition] : *m_partitions) {
            if (time < cover * m_gs.dilationRadius()) {
                thePartition = &partition;
                found = true;
            }
        }
        return found ? *thePartition : m_partitions->front().second;
    }

    void computeDrawing(double cover) {
//        try {
            auto &partition = getPartition(cover);

            bool wellSeparated = true;
            for (const auto &p: m_points) {
                for (const auto &q: m_points) {
                    if (p.category == q.category) continue;
                    if (CGAL::squared_distance(p.point, q.point) < 4 * m_gs.pointSize * m_gs.pointSize) {
                        wellSeparated = false;
                    }
                }
            }
            if (wellSeparated) {
                m_dpd = std::make_shared<DilatedPatternDrawing>(partition, m_gs, m_cds);
            } else {
                std::cerr << "Points of different category are too close together; not computing a drawing."
                          << std::endl;
            }
//        } catch (const std::exception& ex) {
//            std::cerr << "Exception caught!!!" << std::endl;
//            std::cerr << ex.what() << std::endl;
//        }
    }

    val getStackedPolygons() {
        val arr = val::array();

        SimpleSetsPainting painting(*m_dpd, m_ds);
        auto polys = painting.getStackedPolygons();
        if (!polys) return arr;
        for (const auto& poly : *polys) {
            auto rp = renderPath(poly.second);
            auto svgCmds = renderPathToSVGCommands(rp, false);
            val obj = val::object();
            obj.set("pathData", svgCmds);
            obj.set("category", poly.first);
            arr.call<void>("push", obj);
        }

        return arr;
    }
};

EMSCRIPTEN_BINDINGS(doesntmatter) {
    // Auxiliary types
    value_object<Color>("Color")
            .field("r", &Color::r)
            .field("g", &Color::g)
            .field("b", &Color::b);

    register_vector<Color>("ColorVector");

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

    value_object<DrawSettings>("DrawSettings")
            .field("colors", &DrawSettings::colors)
            .field("whiten", &DrawSettings::whiten);

    // Utility functions
    function("dilationRadius", &dilationRadius);
    function("pointStrokeWeight", &pointStrokeWeight);
    function("contourStrokeWeight", &contourStrokeWeight);
    function("getColor", &getColor);

    // SimpleSets class
    class_<SimpleSets>("SimpleSets")
            .constructor<>()
            .function("load", &SimpleSets::load)
            .function("getPoints", &SimpleSets::getPoints)
            .function("computePartition", &SimpleSets::computePartitions)
//            .function("getPartition", &SimpleSets::getPartition)
            .function("computeDrawing", &SimpleSets::computeDrawing)
            .function("getDrawSettings", &SimpleSets::getDrawSettings)
            .function("getComputeDrawingSettings", &SimpleSets::getComputeDrawingSettings)
            .function("getPartitionSettings", &SimpleSets::getPartitionSettings)
            .function("getGeneralSettings", &SimpleSets::getGeneralSettings)
            .function("getStackedPolygons", &SimpleSets::getStackedPolygons)
            .function("setDrawSettings", &SimpleSets::setDrawSettings)
            .function("setComputeDrawingSettings", &SimpleSets::setComputeDrawingSettings)
            .function("setPartitionSettings", &SimpleSets::setPartitionSettings)
            .function("setGeneralSettings", &SimpleSets::setGeneralSettings);
}

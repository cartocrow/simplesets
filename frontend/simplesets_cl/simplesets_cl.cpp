#include <filesystem>
#include <fstream>

#include <QApplication>

#include <nlohmann/json.hpp>

#include "library/parse_input.h"
#include "library/settings.h"
#include "library/partition_algorithm.h"
#include "library/drawing_algorithm.h"

#include <cartocrow/renderer/painting_renderer.h>
#include <cartocrow/renderer/svg_renderer.h>

using namespace cartocrow;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: cartocrow <project_file> <output_file> [<map_file>]\n";
        std::cout << "where <project_file> is a JSON file describing the map to generate,\n";
        std::cout << "<output_file> is the SVG file to write the output to, and <map_file>\n";
        std::cout << "is an Ipe file containing the underlying map (if necessary for the\n";
        std::cout << "map type generated.)\n";
        return 1;
    }

    const std::filesystem::path projectFilename = argv[1];
    const std::filesystem::path outputFilename = argv[2];
    std::string mapFilename = "";
    if (argc == 4) {
        mapFilename = argv[3];
    }
    std::ifstream f(projectFilename);
    json projectData = json::parse(f);

    if (projectData["type"] != "simplesets") return -1;

    std::shared_ptr<renderer::GeometryPainting> painting;
    std::shared_ptr<renderer::GeometryPainting> debugPainting;

    // Parse points
    auto filePath = projectFilename.parent_path() / projectData["points"];
    std::ifstream inputStream(filePath, std::ios_base::in);
    if (!inputStream.good()) {
        throw std::runtime_error("Failed to read input");
    }
    std::stringstream buffer;
    buffer << inputStream.rdbuf();
    auto points = simplesets::parseCatPoints(buffer.str());

    // Parse settings
    const auto& pd = projectData;

    simplesets::GeneralSettings gs;
    const auto& pdgs = pd["generalSettings"];
    gs.pointSize = pdgs["pointSize"];
    gs.inflectionLimit = pdgs["inflectionLimit"];
    gs.maxBendAngle = pdgs["maxBendAngle"];
    gs.maxTurnAngle = pdgs["maxTurnAngle"];

    simplesets::DrawSettings ds;
    const auto& pdds = pd["drawSettings"];
    auto pdColors = pdds["colors"];
    std::vector<Color> colors;
    for (const auto& entry : pdColors) {
        std::string hexString = entry.get<std::string>();
        colors.emplace_back(std::strtol(hexString.c_str(), nullptr, 0));
    }
    ds.colors = colors;
    ds.whiten = pdds["whiten"];

    simplesets::PartitionSettings ps;
    const auto& pdps = pd["partitionSettings"];
    ps.banks = pdps["banks"];
    ps.islands = pdps["islands"];
    ps.regularityDelay = pdps["regularityDelay"];
    ps.intersectionDelay = pdps["intersectionDelay"];
    ps.admissibleRadiusFactor = pdps["admissibleRadiusFactor"];

    simplesets::ComputeDrawingSettings cds;
    const auto& pdcds = pd["computeDrawingSettings"];
    cds.smooth = pdcds["smooth"];
    cds.cutoutRadiusFactor = pdcds["cutoutRadiusFactor"];
    cds.smoothingRadiusFactor = pdcds["smoothingRadiusFactor"];

    double cover = pd["cover"];

    // Partition
    auto partitions = simplesets::partition(points, gs, ps, 8 * CGAL::to_double(gs.dilationRadius()));

    // Draw
    simplesets::Partition* thePartition;
    bool found = false;
    for (auto& [time, partition] : partitions) {
        if (time < cover * gs.dilationRadius()) {
            thePartition = &partition;
            found = true;
        }
    }
    simplesets::Partition& partition = found ? (*thePartition) : partitions.front().second;

    bool wellSeparated = true;
    for (const auto& p : points) {
        for (const auto& q : points) {
            if (p.category == q.category) continue;
            if (CGAL::squared_distance(p.point, q.point) < 4 * gs.pointSize * gs.pointSize) {
                wellSeparated = false;
            }
        }
    }
    if (wellSeparated) {
        auto dpd = simplesets::DilatedPatternDrawing(partition, gs, cds);
        auto ssPainting = simplesets::SimpleSetsPainting(dpd, ds);
        auto pr = std::make_shared<renderer::PaintingRenderer>();
        ssPainting.paint(*pr);
        painting = pr;
    } else {
        std::cerr << "Points of different category are too close together; not computing a drawing." << std::endl;
        return -1;
    }

    std::vector<Point<Inexact>> pts;
    std::transform(points.begin(), points.end(), std::back_inserter(pts),
                   [](const auto& pt) { return pt.point; });
    Box box = CGAL::bbox_2(pts.begin(), pts.end());
    auto delta = CGAL::to_double(2 * gs.dilationRadius());
    Box expanded(box.xmin() - delta, box.ymin() - delta, box.xmax() + delta, box.ymax() + delta);

    cartocrow::renderer::SvgRenderer renderer(painting);
    renderer.save(outputFilename, expanded);
    return 0;
}
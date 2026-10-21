#ifndef CARTOCROW_PARTITION_PAINTING_H
#define CARTOCROW_PARTITION_PAINTING_H

#include "types.h"
#include "settings.h"
#include "partition.h"
#include <cartocrow/renderer/geometry_painting.h>

namespace cartocrow::simplesets {
class PartitionPainting : public GeometryPainting {
  public:
	PartitionPainting(const Partition& partition, const GeneralSettings& gs, const DrawSettings& ds);
	void paint(GeometryRenderer& renderer) const override;

  private:
	const Partition& m_partition;
	const GeneralSettings& m_gs;
	const DrawSettings& m_ds;
};

void draw_poly_pattern(const PolyPattern& pattern, GeometryRenderer& renderer, const GeneralSettings& gs, const DrawSettings& ds);
}

#endif //CARTOCROW_PARTITION_PAINTING_H

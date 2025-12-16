#ifndef CARTOCROW_PATTERN_H
#define CARTOCROW_PATTERN_H

#include <cartocrow/core/core.h>
#include <cartocrow/core/polyline.h>
#include "../cat_point.h"
#include "../settings.h"
#include "../types.h"
#include <variant>

namespace cartocrow::simplesets {
class Pattern {
  public:
	virtual std::variant<Polyline<Inexact>, Polygon<Inexact>, CSPolygon> contour() const = 0;
	virtual const std::vector<CatPoint>& catPoints() const = 0;
	/// Return the category to which the points of this category belong..
	int category() const {
		return catPoints()[0].category;
	}
};
}

#endif //CARTOCROW_PATTERN_H

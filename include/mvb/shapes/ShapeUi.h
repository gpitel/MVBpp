#pragma once

#include "mvb/shapes/ShapeU.h"

namespace mvb {
namespace shapes {

// UI: a U piece closed by a flat I plate (MAS coreType "pieceAndPlate").
//
// Derives from ShapeU because the U half is identical -- same profile, same
// winding window, same machining tool. Only applyExtras differs: it mirrors the
// piece into MKF's column frame, adds the plate, and re-datums the height.
class ShapeUi : public ShapeU {
protected:
    TopoDS_Shape applyExtras(const std::map<std::string, double>& dims,
                             const TopoDS_Shape& piece) const override;
};

} // namespace shapes
} // namespace mvb

#pragma once

#include "MAS.hpp"
#include "constructive_models/Bobbin.h"
#include "Utils.h"
#include "NamedShape.h"
#include <TopoDS_Shape.hxx>

namespace mvb {

class BobbinBuilder {
public:
    // polygonSegments: number of segments for round-column cylinders.
    //   >0  → faceted polygonal prism (matches the core/turn tessellation,
    //          so subsequent boolean cuts with the turns are an order of
    //          magnitude faster than NURBS-vs-NURBS).
    //   <=0 → exact NURBS cylinder (legacy behaviour).
    // Has no effect on rectangular-column bobbins (already boxes).
    //
    // pinRails (ABT #1249): MKF's Bobbin::get_pin_rails(), axis-aligned boxes in MKF's bobbin frame
    // (column axis Y, pin rows along X, the frame the pins are drawn in). They are FUSED with the
    // flange they stand on, so the flange and its rails are one solid of the bobbin compound (body,
    // top flange, bottom flange + rails) under the bobbin's own name and Role::Bobbin. A vertical
    // former's rails hang from the bottom flange's outer face; a horizontal former's stand outboard
    // of both end flanges, the -Y row on the bottom flange's outer face and the +Y row on the top
    // flange's. Each flange and its rails must fuse into ONE solid: a rail that does not touch the
    // flange it stands on is refused, never drawn floating. The pins are
    // separate solids that start on the rails' underside (touching, not overlapping), so the pin
    // cutters of the bobbin cut leave the rails intact. Needs axisIsY (only a Y-axis former has a
    // bottom flange in MKF's frame). Empty: exactly the former without rails.
    static TopoDS_Shape buildBobbin(const MAS::CoreBobbinProcessedDescription& bobbin,
                                    double flangeThickness,
                                    bool axisIsY = false,
                                    int polygonSegments = DEFAULT_CORE_POLYGON_SEGMENTS,
                                    const std::vector<OpenMagnetics::Bobbin::PinRailBlock>& pinRails = {});

    // The rail blocks as OCCT boxes in MKF's bobbin frame, named "<bobbinName> <block name>".
    static std::vector<NamedShape> buildPinRailsNamed(const std::vector<OpenMagnetics::Bobbin::PinRailBlock>& pinRails,
                                                      const std::string& bobbinName);
};

} // namespace mvb

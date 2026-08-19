#include "mvb/shapes/ShapeUi.h"
#include "mvb/Utils.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Pnt.hxx>
#include <numbers>

namespace mvb {
namespace shapes {

// A UI core is one U piece plus a flat I plate closing its open end. Unlike a UU
// set there is no second, mirrored half, so the whole assembly is emitted as a
// single fused solid -- which is also how MKF's CorePieceUi reports it (height
// B + plate thickness, one closed magnetic circuit).
//
// ShapeU has already produced the U in local coordinates: z in [-B, 0], with the
// winding window cut at z in [-D, 0] so the legs open toward +z, and the winding
// column centred at x = 0 with the return leg toward -x.
TopoDS_Shape ShapeUi::applyExtras(const std::map<std::string, double>& dims,
                                  const TopoDS_Shape& piece) const {
    TopoDS_Shape result = ShapeU::applyExtras(dims, piece);

    double a = dims.count("A") ? dims.at("A") : 0.0;
    double b = dims.count("B") ? dims.at("B") : 0.0;
    double c = dims.count("C") ? dims.at("C") : 0.0;
    double d = dims.count("D") ? dims.at("D") : 0.0;
    double e = dims.count("E") ? dims.at("E") : 0.0;
    double b2 = dims.count("B2") ? dims.at("B2") : 0.0;
    if (a == 0.0 || b == 0.0 || c == 0.0 || d == 0.0) return result;

    // Same rule as CorePieceUi::get_plate_thickness(): B2 when the shape gives
    // one, otherwise the U's own yoke thickness. Every catalog UI shape happens
    // to satisfy B2 == B - D, so the fallback is only exercised by shapes that
    // omit B2 -- keep them both, they are not equivalent in general.
    double plateThickness = (b2 > 0.0) ? b2 : (b - d);
    double windingColumnWidth = (a - e) / 2.0;

    // ShapeU puts the return leg at negative x, but MKF's CorePieceUi places the
    // winding column at {0,0,0} and the lateral column at +(A+E)/2. For U and UR
    // that flip is applied downstream, by the {0, pi, 0} rotation Core.cpp gives
    // the bottom half of a two-piece set. A UI core is a single closed element
    // with no rotation, so it has to mirror itself here -- otherwise every turn
    // would be wound around the return leg with the window on the wrong side.
    //
    // R_y(pi) flips z as well as x, and that is wanted: it turns the U over so
    // the yoke ends up on top and the leg ends face -z, ready for the plate. The
    // profile is symmetric in y, so the y extent is unaffected.
    result = rotate_shape(result, 0.0, std::numbers::pi, 0.0);
    // Now x in [-windingColumnWidth/2, A - windingColumnWidth/2], z in [0, B],
    // with the window at z in [0, D] and the leg ends on the z = 0 plane.

    if (plateThickness > 0.0) {
        gp_Pnt corner(-windingColumnWidth / 2.0, -c / 2.0, -plateThickness);
        TopoDS_Shape plate = BRepPrimAPI_MakeBox(corner, a, c, plateThickness).Shape();
        // The plate's top face is coincident with both leg ends, so this fuses to
        // one solid rather than leaving a two-body compound.
        BRepAlgoAPI_Fuse assembly(result, plate);
        if (assembly.IsDone()) {
            result = assembly.Shape();
        }
    }

    // MKF centres the winding window on the origin (both columns sit at y = 0 in
    // the post-rotation frame), whereas ShapeU leaves the mating face there --
    // correct for a half that gets mirrored, wrong for a self-contained assembly.
    return translate_shape(result, 0.0, 0.0, -d / 2.0);
}

} // namespace shapes
} // namespace mvb

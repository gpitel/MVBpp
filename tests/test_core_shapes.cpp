#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "mvb/MagneticBuilder.h"
#include "mvb/Utils.h"
#include "MAS.hpp"
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <vector>
#include <numbers>

using Catch::Matchers::WithinRel;

static MAS::Dimension make_dim(double v) {
    return MAS::Dimension(v);
}

static MAS::Magnetic make_simple_e_magnetic() {
    MAS::Magnetic magnetic;

    // Core shape: E 19/8/5
    MAS::CoreShape shape;
    shape.set_family(MAS::CoreShapeFamily::E);
    shape.set_type(MAS::FunctionalDescriptionType::STANDARD);
    std::map<std::string, MAS::Dimension> dims;
    dims["A"] = make_dim(0.019);
    dims["B"] = make_dim(0.008);
    dims["C"] = make_dim(0.005);
    dims["D"] = make_dim(0.005);
    dims["E"] = make_dim(0.012);
    dims["F"] = make_dim(0.006);
    shape.set_dimensions(dims);

    // Geometrical description: two half sets
    MAS::CoreGeometricalDescriptionElement piece1;
    piece1.set_type(MAS::CoreGeometricalDescriptionElementType::HALF_SET);
    piece1.set_coordinates({0.0, 0.0, 0.0});
    piece1.set_rotation(std::optional<std::vector<double>>(std::vector<double>{0.0, 0.0, 0.0}));
    piece1.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::CoreGeometricalDescriptionElement piece2;
    piece2.set_type(MAS::CoreGeometricalDescriptionElementType::HALF_SET);
    piece2.set_coordinates({0.0, 0.0, 0.0});
    piece2.set_rotation(std::optional<std::vector<double>>(std::vector<double>{std::numbers::pi, 0.0, 0.0}));
    piece2.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::MagneticCore core;
    core.set_geometrical_description(std::optional<std::vector<MAS::CoreGeometricalDescriptionElement>>(std::vector<MAS::CoreGeometricalDescriptionElement>{piece1, piece2}));
    magnetic.set_core(core);

    MAS::Coil coil;
    magnetic.set_coil(coil);

    return magnetic;
}

TEST_CASE("E core builds manually", "[core][e]") {
    auto magnetic = make_simple_e_magnetic();

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());

    REQUIRE(pieces.size() == 2);

    double totalVolume = 0.0;
    for (const auto& ns : pieces) {
        const auto& s = ns.shape;
        REQUIRE(!s.IsNull());

        Bnd_Box box;
        BRepBndLib::Add(s, box);
        double xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        REQUIRE(xmax > xmin);
        REQUIRE(ymax > ymin);
        REQUIRE(zmax > zmin);

        GProp_GProps props;
        BRepGProp::VolumeProperties(s, props);
        totalVolume += props.Mass();
    }

    REQUIRE(totalVolume > 0.0);
}

static MAS::Magnetic make_simple_t_magnetic() {
    MAS::Magnetic magnetic;

    MAS::CoreShape shape;
    shape.set_family(MAS::CoreShapeFamily::T);
    shape.set_type(MAS::FunctionalDescriptionType::STANDARD);
    std::map<std::string, MAS::Dimension> dims;
    dims["A"] = make_dim(0.020); // outer diameter
    dims["B"] = make_dim(0.010); // inner diameter
    dims["C"] = make_dim(0.008); // height
    shape.set_dimensions(dims);

    MAS::CoreGeometricalDescriptionElement piece;
    piece.set_type(MAS::CoreGeometricalDescriptionElementType::TOROIDAL);
    piece.set_coordinates({0.0, 0.0, 0.0});
    piece.set_rotation(std::optional<std::vector<double>>(std::vector<double>{0.0, 0.0, 0.0}));
    piece.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::MagneticCore core;
    core.set_geometrical_description(std::optional<std::vector<MAS::CoreGeometricalDescriptionElement>>(std::vector<MAS::CoreGeometricalDescriptionElement>{piece}));
    magnetic.set_core(core);

    MAS::Coil coil;
    magnetic.set_coil(coil);

    return magnetic;
}

static MAS::Magnetic make_simple_pq_magnetic() {
    MAS::Magnetic magnetic;

    MAS::CoreShape shape;
    shape.set_family(MAS::CoreShapeFamily::PQ);
    shape.set_type(MAS::FunctionalDescriptionType::STANDARD);
    std::map<std::string, MAS::Dimension> dims;
    // PQ 32/12 — note e*cos_g > c, exercises the self-intersection fix
    dims["A"] = make_dim(0.033);
    dims["B"] = make_dim(0.00594);
    dims["C"] = make_dim(0.022);
    dims["D"] = make_dim(0.0034);
    dims["E"] = make_dim(0.027);
    dims["F"] = make_dim(0.0135);
    shape.set_dimensions(dims);

    MAS::CoreGeometricalDescriptionElement piece1, piece2;
    piece1.set_type(MAS::CoreGeometricalDescriptionElementType::HALF_SET);
    piece1.set_coordinates({0.0, 0.0, 0.0});
    piece1.set_rotation(std::optional<std::vector<double>>(std::vector<double>{std::numbers::pi, std::numbers::pi, 0.0}));
    piece1.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    piece2.set_type(MAS::CoreGeometricalDescriptionElementType::HALF_SET);
    piece2.set_coordinates({0.0, 0.0, 0.0});
    piece2.set_rotation(std::optional<std::vector<double>>(std::vector<double>{0.0, 0.0, 0.0}));
    piece2.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::MagneticCore core;
    core.set_geometrical_description(std::optional<std::vector<MAS::CoreGeometricalDescriptionElement>>(
        std::vector<MAS::CoreGeometricalDescriptionElement>{piece1, piece2}));
    magnetic.set_core(core);

    MAS::Coil coil;
    magnetic.set_coil(coil);

    return magnetic;
}

TEST_CASE("PQ core builds manually with non-zero volume", "[core][pq]") {
    auto magnetic = make_simple_pq_magnetic();

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());

    REQUIRE(pieces.size() == 2);

    double totalVolume = 0.0;
    for (const auto& ns : pieces) {
        const auto& s = ns.shape;
        REQUIRE(!s.IsNull());

        Bnd_Box box;
        BRepBndLib::Add(s, box);
        REQUIRE(!box.IsVoid());

        GProp_GProps props;
        BRepGProp::VolumeProperties(s, props);
        totalVolume += props.Mass();
    }

    printf("PQ 32/12 total volume (2 pieces): %.8e m^3\n", totalVolume);
    REQUIRE(totalVolume > 0.0);
    REQUIRE(totalVolume > 5.32e-6);  // Python reference: 5.37779e-6 m^3, within 1%
    REQUIRE(totalVolume < 5.43e-6);
}

static MAS::Magnetic make_simple_drum_magnetic() {
    MAS::Magnetic magnetic;

    MAS::CoreShape shape;
    shape.set_family(MAS::CoreShapeFamily::DRUM);
    shape.set_type(MAS::FunctionalDescriptionType::STANDARD);
    std::map<std::string, MAS::Dimension> dims;
    // TDG DRH-14X20-4C (ABT #331)
    dims["A"] = make_dim(0.014);   // flange OD
    dims["B"] = make_dim(0.020);   // total height
    dims["C"] = make_dim(0.009);   // post OD
    dims["D"] = make_dim(0.00375); // top flange thickness
    dims["E"] = make_dim(0.0125);  // groove height
    dims["F"] = make_dim(0.00375); // bottom flange thickness
    dims["H"] = make_dim(0.0032);  // bore
    shape.set_dimensions(dims);

    // Single solid piece: MKF emits drums as one CLOSED element (openness is a
    // property of the magnetic circuit, not of the solid).
    MAS::CoreGeometricalDescriptionElement piece;
    piece.set_type(MAS::CoreGeometricalDescriptionElementType::CLOSED);
    piece.set_coordinates({0.0, 0.0, 0.0});
    piece.set_rotation(std::optional<std::vector<double>>(std::vector<double>{0.0, 0.0, 0.0}));
    piece.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::MagneticCore core;
    core.set_geometrical_description(std::optional<std::vector<MAS::CoreGeometricalDescriptionElement>>(
        std::vector<MAS::CoreGeometricalDescriptionElement>{piece}));
    magnetic.set_core(core);

    MAS::Coil coil;
    magnetic.set_coil(coil);

    return magnetic;
}

TEST_CASE("Drum core builds from its CLOSED geometrical element", "[core][drum]") {
    auto magnetic = make_simple_drum_magnetic();

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());

    // Regression: CLOSED elements used to be silently skipped, yielding a
    // coreless magnetic (only winding + leads in the STEP).
    REQUIRE(pieces.size() == 1);

    const auto& s = pieces[0].shape;
    REQUIRE(!s.IsNull());

    Bnd_Box box;
    BRepBndLib::Add(s, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    // Flange OD across x/z, total height across y, centred at the origin.
    REQUIRE_THAT(xmax - xmin, WithinRel(0.014, 0.05));
    REQUIRE_THAT(ymax - ymin, WithinRel(0.020, 0.05));
    REQUIRE(std::abs(ymax + ymin) < 1e-4);

    GProp_GProps props;
    BRepGProp::VolumeProperties(s, props);
    double volume = props.Mass();
    // Analytic (circular): 2 flanges pi/4(A^2-H^2)D + post pi/4(C^2-H^2)E = 1.789e-6 m^3.
    // The polygonal flange profile undercuts the circle by a few percent.
    REQUIRE(volume > 1.60e-6);
    REQUIRE(volume < 1.85e-6);
}

TEST_CASE("T core builds manually", "[core][t]") {
    auto magnetic = make_simple_t_magnetic();

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());

    REQUIRE(pieces.size() == 1);

    const auto& s = pieces[0].shape;
    REQUIRE(!s.IsNull());

    Bnd_Box box;
    BRepBndLib::Add(s, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    REQUIRE(xmax > xmin);
    REQUIRE(ymax > ymin);
    REQUIRE(zmax > zmin);

    GProp_GProps props;
    BRepGProp::VolumeProperties(s, props);
    double volume = props.Mass();
    REQUIRE(volume > 0.0);
}

// ---------------------------------------------------------------------------
// UI: a U piece closed by a flat I plate, emitted as one fused solid.
// ---------------------------------------------------------------------------

// plateThickness lets a case override B2 independently of B - D. Every catalog
// UI shape satisfies B2 == B - D, so without an override the tests cannot tell a
// correct implementation from one that always falls back to B - D.
static MAS::Magnetic make_simple_ui_magnetic(double plateThickness = 0.0) {
    MAS::Magnetic magnetic;

    MAS::CoreShape shape;
    shape.set_family(MAS::CoreShapeFamily::UI);
    shape.set_type(MAS::FunctionalDescriptionType::STANDARD);
    std::map<std::string, MAS::Dimension> dims;
    // UI 93/76/20. E is pinned rather than taken from the catalog: no UI record
    // has a nominal E, so flatten_dimension resolves it from the bounds and a
    // later catalog edit would silently move every derived quantity.
    dims["A"] = make_dim(0.093);    // overall width
    dims["B"] = make_dim(0.076);    // U height, plate excluded
    dims["C"] = make_dim(0.020);    // depth
    dims["D"] = make_dim(0.048);    // window height
    dims["E"] = make_dim(0.0346);   // window width
    dims["B2"] = make_dim(plateThickness > 0.0 ? plateThickness : 0.028);
    shape.set_dimensions(dims);

    // One CLOSED element: unlike a UU set there is no mirrored second half, so
    // ShapeUi fuses the U and the plate into a single solid.
    MAS::CoreGeometricalDescriptionElement piece;
    piece.set_type(MAS::CoreGeometricalDescriptionElementType::CLOSED);
    piece.set_coordinates({0.0, 0.0, 0.0});
    piece.set_rotation(std::optional<std::vector<double>>(std::vector<double>{0.0, 0.0, 0.0}));
    piece.set_shape(std::optional<MAS::CoreShapeDataOrNameUnion>(shape));

    MAS::MagneticCore core;
    core.set_geometrical_description(std::optional<std::vector<MAS::CoreGeometricalDescriptionElement>>(
        std::vector<MAS::CoreGeometricalDescriptionElement>{piece}));
    magnetic.set_core(core);

    MAS::Coil coil;
    magnetic.set_coil(coil);

    return magnetic;
}

TEST_CASE("UI core builds as one fused solid", "[core][ui]") {
    auto magnetic = make_simple_ui_magnetic();

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());

    REQUIRE(pieces.size() == 1);
    const auto& s = pieces[0].shape;
    REQUIRE(!s.IsNull());

    const double a = 0.093, b = 0.076, c = 0.020, d = 0.048, e = 0.0346;
    const double plate = 0.028;
    const double legWidth = (a - e) / 2.0;

    Bnd_Box box;
    BRepBndLib::Add(s, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    // Overall envelope is A x (B + plate) x C, matching CorePieceUi's
    // set_width(A) / set_height(B + plateThickness) / set_depth(C).
    REQUIRE_THAT(xmax - xmin, WithinRel(a, 0.02));
    REQUIRE_THAT(ymax - ymin, WithinRel(b + plate, 0.02));
    REQUIRE_THAT(zmax - zmin, WithinRel(c, 0.02));

    // The assembly is deliberately NOT centred on y: MKF puts the winding window
    // centre at the origin, so the U reaches up to B - D/2 and the plate hangs
    // down to -(D/2 + plate). Checking the total height alone would pass even
    // with the mirror or the datum shift wrong, since both errors preserve it.
    REQUIRE_THAT(ymax, WithinRel(b - d / 2.0, 0.02));
    REQUIRE_THAT(ymin, WithinRel(-(d / 2.0 + plate), 0.02));

    // Winding column centred on x = 0 with the return leg at POSITIVE x, per
    // CorePieceUi::process_columns. Without ShapeUi's mirror the core would sit
    // at negative x and every turn would be wound around the wrong leg.
    REQUIRE_THAT(xmin, WithinRel(-legWidth / 2.0, 0.02));
    REQUIRE(xmax > 0.0);

    // Volume: U (A*C*B minus the window) plus the plate.
    GProp_GProps props;
    BRepGProp::VolumeProperties(s, props);
    const double expected = (a * c * b - e * c * d) + (a * c * plate);
    REQUIRE_THAT(props.Mass(), WithinRel(expected, 0.02));
}

TEST_CASE("UI plate thickness follows B2, not B - D", "[core][ui]") {
    // B2 = 10 mm against B - D = 28 mm. An implementation that ignores B2 and
    // always uses B - D still produces a plausible-looking core, so this is the
    // only case that distinguishes them.
    const double plate = 0.010;
    auto magnetic = make_simple_ui_magnetic(plate);

    mvb::MagneticBuilder builder;
    auto pieces = builder.buildCoreNamed(magnetic.get_core().value());
    REQUIRE(pieces.size() == 1);

    const double b = 0.076, d = 0.048;
    Bnd_Box box;
    BRepBndLib::Add(pieces[0].shape, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    REQUIRE_THAT(ymax - ymin, WithinRel(b + plate, 0.02));
    REQUIRE_THAT(ymin, WithinRel(-(d / 2.0 + plate), 0.02));
}

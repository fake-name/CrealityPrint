#include "Debugger.hpp"
#include "Layer.hpp"
#include "ClipperZUtils.hpp"

namespace Slic3r {
	void bench_debug_volume_slices(Print* print, PrintObject* object, const std::vector<VolumeSlices>& slices, const std::vector<float>& slice_zs)
	{
#ifdef BENCH_DEBUG
		print->debugger->volume_slices(print, object, slices, slice_zs);
#endif
	}

	void bench_debug_lslices(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->raw_lslices(print, object);
#endif
	}

	void begin_debug_concial_overhang(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->begin_concial_overhang(print, object);
#endif
	}

	void end_debug_concial_overhang(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->end_concial_overhang(print, object);
#endif
	}

	void begin_debug_hole_to_polyhole(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->begin_hole_to_polyhole(print, object);
#endif
	}

	void end_debug_hole_to_polyhole(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->end_hole_to_polyhole(print, object);
#endif
	}

	void debug_perimeters(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->perimeters(print, object);
#endif
	}

	void debug_estimate_curled_extrusions(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->curled_extrusions(print, object);
#endif
	}

	void debug_surfaces_type(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->surfaces_type(print, object);
#endif
	}

	void debug_infill_surfaces_type(Print* print, PrintObject* object, int step)
	{
#ifdef BENCH_DEBUG
		print->debugger->infill_surfaces_type(print, object, step);
#endif
	}

	void debug_infills(Print* print, PrintObject* object)
	{
#ifdef BENCH_DEBUG
		print->debugger->infills(print, object);
#endif
	}

	void bench_debug_generate(Print* print, int layer, const std::string& code, bool by_object)
	{
#ifdef BENCH_DEBUG
		print->debugger->gcode("generate", layer, by_object, code);
#endif
	}

	void bench_debug_cooling(Print* print, int layer, const std::string& code, bool by_object)
	{
#ifdef BENCH_DEBUG
		print->debugger->gcode("cooling", layer, by_object, code);
#endif
	}

	void bench_debug_fanmove(Print* print, int layer, const std::string& code, bool by_object)
	{
#ifdef BENCH_DEBUG
		print->debugger->gcode("fanmove", layer, by_object, code);
#endif
	}

	void bench_debug_output(Print* print, int layer, const std::string& code, bool by_object)
	{
#ifdef BENCH_DEBUG
		print->debugger->gcode("output", layer, by_object, code);
#endif
	}

	void bench_debug_seamplacer(Print* print, SeamPlacer* placer)
	{
#ifdef BENCH_DEBUG
		print->debugger->seamplacer(print, placer);
#endif
	}

    ////////////////////using immediate window for debug////////////////////////////
    void to_svg(const char* path, const ExPolygons& expolygons, bool fill/* = true*/)
    {
        if (fill)
        {
            SVG::ExPolygonAttributes attri("red", "", "", 0);
            std::vector<std::pair<Slic3r::ExPolygons, SVG::ExPolygonAttributes>> expolygons_with_attributes;
            expolygons_with_attributes.push_back({ expolygons, attri });
            SVG::export_expolygons(path, expolygons_with_attributes);
        }
        else
        {
            //Polygons polygons = to_polygons(expolygons);
            BoundingBox bb = get_extents(expolygons);
            SVG svg(path, bb);
            svg.draw_outline(expolygons, "red", "red", 10000);
            svg.Close();
        }
    }

    void to_svg(const char* path, const ExPolygon& expolygon, bool fill/* = true*/)
    {
        ExPolygons temp;
        temp.push_back(expolygon);
        to_svg(path, temp, fill);
    }

    void to_svg(const char* path, const ExPolygons& expolys0, const ExPolygons& expolys1, bool fill/* = true*/)
    {
        if (fill)
        {
            SVG::ExPolygonAttributes attri_0("red", "", "", 0);
            SVG::ExPolygonAttributes attri_1("green", "", "", 0);

            std::vector<std::pair<Slic3r::ExPolygons, SVG::ExPolygonAttributes>> expolygons_with_attributes;
            expolygons_with_attributes.push_back({ expolys0, attri_0 });
            expolygons_with_attributes.push_back({ expolys1, attri_1 });
            SVG::export_expolygons(path, expolygons_with_attributes);
        }
        else
        {
            BoundingBox bb = get_extents(expolys0);
            bb.merge(get_extents(expolys1));

            SVG svg(path, bb);
            svg.draw_outline(expolys0, "red", "red", 10000);
            svg.draw_outline(expolys1, "green", "green", 10000);
            svg.Close();
        }
    }

    void to_svg(const char* path, const ExPolygon& expoly0, const ExPolygon& expoly1, bool fill/* = true*/)
    {
        ExPolygons tmp0;
        ExPolygons tmp1;
        tmp0.push_back(expoly0);
        tmp1.push_back(expoly1);
        to_svg(path, tmp0, tmp1, fill);
    }

    void to_svg(const char* path, const ExPolygons& expolys0, const ExPolygon& expoly, bool fill/* = true*/)
    {
        ExPolygons tmp1;
        tmp1.push_back(expoly);
        to_svg(path, expolys0, tmp1, fill);
    }

    void to_svg(const char* path, const ExPolygon& expoly, const ExPolygons& expolys1, bool fill/* = true*/)
    {
        ExPolygons tmp0;
        tmp0.push_back(expoly);
        to_svg(path, tmp0, expolys1, fill);
    }

    void to_svg(const char* dir, const std::vector<ExPolygons>& expolyss, bool fill/* = true*/)
    {
        std::string file = std::string(dir) + "\\test";

        for (int i = 0; i < expolyss.size(); i++)
        {
            if (expolyss[i].empty())
            {
                continue;
            }

            std::string tem_file = file + std::to_string(i) + ".svg";

            to_svg(tem_file.c_str(), expolyss[i], fill);
        }
    }
    //------------------------------------------------------------------------------

    void to_svg(const char* path, const Polygon& polygon, bool fill/* = true*/)
    {
        SVG svg(path, get_extents(polygon));
        if (fill)
        {
            svg.draw(polygon, "red");
        }
        else
        {
            svg.draw_outline(polygon, "red", 10000);
        }

        svg.Close();
    }

    void to_svg(const char* path, const Polygon& poly0, const Polygon& poly1, bool fill/* = true*/)
    {
        std::string color_0 = "blue";
        std::string color_1 = "green";

        BoundingBox bb = get_extents(poly0);
        bb.merge(get_extents(poly1));

        SVG svg(path, bb);
        if (fill)
        {
            svg.draw(poly0, color_0);
            svg.draw(poly1, color_1);
        }
        else
        {
            svg.draw_outline(poly0, color_0, 10000);
            svg.draw_outline(poly1, color_1, 10000);
        }

        svg.Close();
    }

    void to_svg(const char* path, const Polygon& poly, const Polylines& polylines, bool fill/* = true*/)
    {
        SVG svg(path, get_extents(poly));

        if (fill)
        {
            svg.draw(poly, "red");
        }
        else
        {
            svg.draw_outline(poly, "red", 10000);
        }

        svg.draw(polylines, "green", 10000);
        svg.Close();
    }

    void to_svg(const char* path, const Polygon& poly, const BoundingBox& bbox, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(poly);
        bb.merge(bbox);

        SVG svg(path, bb);
        if (fill)
        {
            svg.draw(poly, "red");
            svg.draw(bbox.polygon(), "red");
        }
        else
        {
            svg.draw_outline(poly, "red", 10000);
            svg.draw_outline(bbox.polygon(), "green", 1000);
        }

        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polygons, bool fill/* = true*/)
    {
        if (fill)
        {
            ExPolygons temp1 = union_ex(polygons);
            to_svg(path, temp1, fill);
        }
        else
        {
            SVG svg(path, get_extents(polygons));
            svg.draw_outline(polygons, "red", 10000);
            svg.Close();

        }
    }

    void to_svg(const char* path, const Polygons& polys0, const Polygons& polys1, bool fill/* = true*/)
    {
        if (fill)
        {
            ExPolygons temp0 = union_ex(polys0);
            ExPolygons temp1 = union_ex(polys1);
            to_svg(path, temp0, temp1, fill);
        }
        else
        {
            std::string stroke_str0 = "red";
            std::string stroke_str1 = "green";

            BoundingBox bb = get_extents(polys0);
            bb.merge(get_extents(polys1));

            SVG svg(path, bb);
            svg.draw_outline(polys0, stroke_str0, 10000);
            svg.draw_outline(polys1, stroke_str1, 10000);
            svg.Close();
        }
    }

    void to_svg(const char* path, const Polygons& polys, const Polylines& polylines, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys);
        bb.merge(get_extents(polylines));
        SVG svg(path, bb);

        if (fill)
        {
            ExPolygons temp1 = union_ex(polys);
            svg.draw(temp1, "red", 0.5);
        }
        else
        {
            svg.draw_outline(polys, "red", 10000);
        }

        //svg.draw(polylines, "green", 10000);
        for (auto& line : polylines)
        {
            svg.draw(line.points, "green", 50000);
        }

        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polys, const BoundingBox& bbox, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys);
        bb.merge(bbox);
        SVG svg(path, bb);
        if (fill)
        {
            ExPolygons temp1 = union_ex(polys);
            svg.draw(temp1, "red", 0.5);
        }
        else
        {
            svg.draw_outline(polys, "red", 10000);
        }

        svg.draw_outline(bbox.polygon(), "green", 10000);
        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polys, const ExPolygon& expoly, bool fill/* = true*/)
    {
        to_svg(path, union_ex(polys), expoly, fill);
    }

    void to_svg(const char* path, const Polygons& polys, const ExPolygons& expoly, bool fill/* = true*/)
    {
        to_svg(path, union_ex(polys), expoly, fill);
    }

    void to_svg(const char* path, const Polygons& polys, const Point& pt, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys);
        bb.merge(pt);
        SVG svg(path, bb);

        if (fill)
        {
            ExPolygons temp1 = union_ex(polys);
            svg.draw(temp1, "red", 0.5);
        }
        else
        {
            svg.draw_outline(polys, "red", 10000);
        }

        svg.draw(pt, "green", 100000);
        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polys, const Point& pt0, const Point& pt1, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys);
        bb.merge(pt0);
        bb.merge(pt1);
        SVG svg(path, bb);

        if (fill)
        {
            ExPolygons temp1 = union_ex(polys);
            svg.draw(temp1, "red", 0.5);
        }
        else
        {
            svg.draw_outline(polys, "red", 10000);
        }

        svg.draw(pt0, "red", 100000);
        svg.draw(pt1, "green", 100000);
        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polys, const Point& pt0, const Point& pt1, const Point& pt2, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys);
        bb.merge(pt0);
        bb.merge(pt1);
        bb.merge(pt2);
        SVG svg(path, bb);

        if (fill)
        {
            ExPolygons temp1 = union_ex(polys);
            svg.draw(temp1, "red", 0.5);
        }
        else
        {
            svg.draw_outline(polys, "red", 10000);
        }

        svg.draw(pt0, "red", 100000);
        svg.draw(pt1, "green", 100000);
        svg.draw(pt2, "blue", 100000);
        svg.Close();
    }

    void to_svg(const char* path, const Polygons& polys0, const Polygons& polys1, const Polygons& polys2, bool fill/* = true*/)
    {
        BoundingBox bb = get_extents(polys0);
        bb.merge(get_extents(polys1));
        bb.merge(get_extents(polys2));
        SVG svg(path, bb);

        if (fill)
        {
            ExPolygons temp0 = union_ex(polys0);
            ExPolygons temp1 = union_ex(polys1);
            ExPolygons temp2 = union_ex(polys2);

            svg.draw(temp0, "red", 0.5);
            svg.draw(temp1, "green", 0.5);
            svg.draw(temp2, "blue", 0.5);
        }
        else
        {
            svg.draw_outline(polys0, "red", 10000);
            svg.draw_outline(polys1, "green", 10000);
            svg.draw_outline(polys2, "blue", 10000);
        }

        svg.Close();
    }

    void to_svg(const char* dir, const std::vector<Polygons>& polyss, bool fill/* = true*/)
    {
        std::string file = std::string(dir) + "\\test";

        for (int i = 0; i < polyss.size(); i++)
        {
            if (polyss[i].empty())
            {
                continue;
            }

            std::string tem_file = file + std::to_string(i) + ".svg";

            to_svg(tem_file.c_str(), polyss[i], fill);
        }
    }
    //-------------------------------------------------------------------------------

    void to_svg(const char* path, const Polyline& polyline)
    {
        SVG svg(path, get_extents(polyline));
        svg.draw(polyline, "red", 10000);
        svg.Close();
    }

    void to_svg(const char* path, const Polyline& polyline0, const Polyline& polyline1)
    {
        BoundingBox bb = get_extents(polyline0);
        bb.merge(get_extents(polyline1));

        SVG svg(path, bb);
        svg.draw(polyline0, "red", 10000);
        svg.draw(polyline1, "green", 10000);
        svg.Close();
    }

    void to_svg(const char* path, const Polylines& polylines)
    {
        SVG svg(path, get_extents(polylines));
        svg.draw(polylines, "red", 10000);
        svg.Close();
    }

    void to_svg(const char* dir, const std::vector<Polylines>& polyliness)
    {
        std::string file = std::string(dir) + "\\test";

        for (int i = 0; i < polyliness.size(); i++)
        {
            if (polyliness[i].empty())
            {
                continue;
            }

            std::string tem_file = file + std::to_string(i) + ".svg";

            to_svg(tem_file.c_str(), polyliness[i]);
        }
    }
    //-------------------------------------------------------------------------------

    void to_svg(const char* path, const SurfaceCollection& surfacs)
    {
        const_cast<SurfaceCollection&>(surfacs).export_to_svg(path, true);
    }
    //-------------------------------------------------------------------------------

    void to_obj(const char* path, const indexed_triangle_set& its)
    {
        its_write_obj(its, path);
    }

    void to_obj(const char* path, const std::vector<indexed_triangle_set>& itss)
    {
        if (itss.empty())
        {
            return;
        }

        indexed_triangle_set its = itss[0];

        for (int i = 1; i < itss.size(); i++)
        {
            its_merge(its, itss[i]);
        }

        to_obj(path, its);
    }

    //one file for std::vector<indexed_triangle_set>
    void to_obj(const char* dir, const std::vector<std::vector<indexed_triangle_set>>& itsss, bool sperate_file/* = false*/)
    {
        if (itsss.empty())
        {
            return;
        }

        //put into one file
        if (sperate_file == false)
        {
            std::string file = std::string(dir) + "\\test0.obj";

            indexed_triangle_set its_temp;
            for (int i = 0; i < itsss.size(); i++)
            {
                for (int j = 0; j < itsss[i].size(); j++)
                {
                    its_merge(its_temp, itsss[i][j]);
                }
            }

            if (!its_temp.empty())
            {
                to_obj(file.c_str(), its_temp);
            }
        }
        else
        {
            std::string file = std::string(dir) + "\\test";

            for (int i = 0; i < itsss.size(); i++)
            {
                if (itsss[i].empty())
                {
                    continue;
                }

                std::string tem_file = file + std::to_string(i) + ".obj";

                to_obj(tem_file.c_str(), itsss[i]);
            }
        }
    }

    void to_obj(const char* path, const TriangleMesh& tm)
    {

    }

    void to_obj(const char* path, const std::vector<TriangleMesh>& tms)
    {

    }

    void to_obj(const char* dir, const std::vector<std::vector<TriangleMesh>>& tmss)
    {

    }

    ///////////////////////////////////////////////////////////////////////////////
}

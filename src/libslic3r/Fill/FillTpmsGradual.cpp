#include "../ClipperUtils.hpp"
#include "../ShortestPath.hpp"
#include "../Surface.hpp"
#include "cr_tpms_library.h"
#include "FillTpmsGradual.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Slic3r {

using namespace std;

void FillTpmsGradual::cal_scalar_field(const FillParams&                                 params,
                                 std::vector<std::vector<MarchingSquares::Point>>& posxy,
                                 std::vector<std::vector<double>>&                 data)
{
    // tpms variant start
    int           _celltype        = Lattice_Id;
    int           _infill_density1 = params.config->tpms_start_infill_density.value;    
    if (_infill_density1 < 1) {
        _infill_density1 = 1;
    } else if (_infill_density1 > 99) {
        _infill_density1 = 99;
    }
    int           _infill_density2 = params.config->tpms_end_infill_density.value;
    if (_infill_density2 < 1) {
        _infill_density2 = 1;
    } else if (_infill_density2 > 99) {
        _infill_density2 = 99;
    }
    int           _axis            = params.config->tpms_gradual_direction.value;
    Slic3r::Vec3d bbox_min;
    Slic3r::Vec3d bbox_max;
    bbox_min[0] = unscale_(bounding_box.min[0]);
    bbox_min[1] = unscale_(bounding_box.min[1]);
    bbox_min[2] = 0;
    bbox_max[0] = unscale_(bounding_box.max[0]);
    bbox_max[1] = unscale_(bounding_box.max[1]);
    bbox_max[2] = unscale_(bounding_box_height);

    float bbox_min_x = bbox_min[0];
    float bbox_min_y = bbox_min[1];
    float bbox_min_z = bbox_min[2];
    float bbox_max_x = bbox_max[0];
    float bbox_max_y = bbox_max[1];
    float bbox_max_z = bbox_max[2];
    // tpms variant end

    int   width      = posxy[0].size();
    int   height     = posxy.size();
    int   total_size = (height) * (width);
    float curz       = z;

    tbb::parallel_for(tbb::blocked_range<size_t>(0, total_size),
                      [&width, &data, &posxy, &curz, &bbox_min_x, &bbox_min_y, &bbox_min_z, &bbox_max_x, &bbox_max_y, &bbox_max_z, &_axis,
                       &_celltype, &_infill_density1, &_infill_density2](const tbb::blocked_range<size_t>& range) {
                          for (size_t k = range.begin(); k < range.end(); ++k) {
                              int i      = k / (width); // 计算行索引
                              int j      = k % (width); // 计算列索引
                              data[i][j] = scalar_function_test(posxy[i][j].x, posxy[i][j].y, curz, bbox_min_x, bbox_min_y, bbox_min_z,
                                                                bbox_max_x, bbox_max_y, bbox_max_z, _axis, _celltype, _infill_density1,
                                                                _infill_density2);
                          }
                      });
}



} // namespace Slic3r

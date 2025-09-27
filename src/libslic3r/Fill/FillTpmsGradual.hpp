#ifndef slic3r_FillTpmsGradual_hpp_
#define slic3r_FillTpmsGradual_hpp_

#include "../libslic3r.h"
#include "FillBase.hpp"
#include"FillTpmsD.hpp"

namespace Slic3r {

class FillTpmsGradual : public FillTpmsD
{
public:
    FillTpmsGradual(size_t lattice_Id) { Lattice_Id = lattice_Id; };
    Fill* clone() const override { return new FillTpmsGradual(*this); }
    // type of Lattice   0-g; 1-d; 2-p; 3-Fischer - Koch S; 
    size_t Lattice_Id;

    virtual void cal_scalar_field(const FillParams&                                 params,
                                  std::vector<std::vector<MarchingSquares::Point>>& posxy,
                                  std::vector<std::vector<double>>&                 data);

};

} // namespace Slic3r


#endif // slic3r_FillTpmsGradual_hpp_

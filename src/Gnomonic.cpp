/**
 * \file Gnomonic.cpp
 * \brief Implementation for GeographicLib::Gnomonic class
 *
 * Copyright (c) Charles Karney (2010, 2011) <charles@karney.com> and licensed
 * under the LGPL.  For more information, see
 * http://geographiclib.sourceforge.net/
 **********************************************************************/

#include <GeographicLib/Gnomonic.hpp>

#define GEOGRAPHICLIB_GNOMONIC_CPP "$Id$"

RCSID_DECL(GEOGRAPHICLIB_GNOMONIC_CPP)
RCSID_DECL(GEOGRAPHICLIB_GNOMONIC_HPP)

namespace GeographicLib {

  using namespace std;

  const Math::real Gnomonic::eps0_ = numeric_limits<real>::epsilon();
  const Math::real Gnomonic::eps_ = real(0.01) * sqrt(eps0_);

  void Gnomonic::Forward(real lat0, real lon0, real lat, real lon,
                         real& x, real& y, real& azi, real& rk)
    const throw() {
    real azi0, m, M, t;
    _earth.GenInverse(lat0, lon0, lat, lon,
                      Geodesic::AZIMUTH | Geodesic::REDUCEDLENGTH |
                      Geodesic::GEODESICSCALE,
                      t, azi0, azi, m, M, t, t);
    rk = M;
    if (M <= 0)
      x = y = Math::NaN();
    else {
      real rho = m/M;
      azi0 *= Math::degree<real>();
      x = rho * sin(azi0);
      y = rho * cos(azi0);
    }
  }

  void Gnomonic::Reverse(real lat0, real lon0, real x, real y,
                         real& lat, real& lon, real& azi, real& rk)
    const throw() {
    real
      azi0 = atan2(x, y) / Math::degree<real>(),
      rho = Math::hypot(x, y),
      s = _a * atan(rho/_a);
    bool little = rho <= _a;
    if (!little)
      rho = 1/rho;
    GeodesicLine line(_earth.Line(lat0, lon0, azi0,
                                  Geodesic::LATITUDE | Geodesic::LONGITUDE |
                                  Geodesic::AZIMUTH | Geodesic::DISTANCE_IN |
                                  Geodesic::REDUCEDLENGTH |
                                  Geodesic::GEODESICSCALE));
    int count = numit_, trip = 0;
    real lat1, lon1, azi1, M;
    while (count--) {
      real m, t;
      line.Position(s, lat1, lon1, azi1, m, M, t);
      if (trip)
        break;
      // If little, solve rho(s) = rho with drho(s)/ds = 1/M^2
      // else solve 1/rho(s) = 1/rho with d(1/rho(s))/ds = -1/m^2
      real ds = little ? (m/M - rho) * M * M : (rho - M/m) * m * m;
      s -= ds;
      if (!(abs(ds) >= eps_ * _a))
        ++trip;
    }
    if (trip) {
      lat = lat1; lon = lon1; azi = azi1; rk = M;
    } else
      lat = lon = azi = rk = Math::NaN();
    return;
  }

} // namespace GeographicLib
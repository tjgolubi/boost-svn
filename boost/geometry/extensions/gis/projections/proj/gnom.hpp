#ifndef BOOST_GEOMETRY_PROJECTIONS_GNOM_HPP
#define BOOST_GEOMETRY_PROJECTIONS_GNOM_HPP

// Boost.Geometry - extensions-gis-projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright (c) 2008-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels (Geodan, Amsterdam)

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry { namespace projection
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace gnom{
            static const double EPS10 = 1.e-10;
            static const int N_POLE = 0;
            static const int S_POLE = 1;
            static const int EQUIT = 2;
            static const int OBLIQ = 3;

            struct par_gnom
            {
                double    sinph0;
                double    cosph0;
                int        mode;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_gnom_spheroid : public base_t_fi<base_gnom_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;

                par_gnom m_proj_parm;

                inline base_gnom_spheroid(const Parameters& par)
                    : base_t_fi<base_gnom_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    double  coslam, cosphi, sinphi;

                    sinphi = sin(lp_lat);
                    cosphi = cos(lp_lat);
                    coslam = cos(lp_lon);
                    switch (this->m_proj_parm.mode) {
                    case EQUIT:
                        xy_y = cosphi * coslam;
                        break;
                    case OBLIQ:
                        xy_y = this->m_proj_parm.sinph0 * sinphi + this->m_proj_parm.cosph0 * cosphi * coslam;
                        break;
                    case S_POLE:
                        xy_y = - sinphi;
                        break;
                    case N_POLE:
                        xy_y = sinphi;
                        break;
                    }
                    if (xy_y <= EPS10) throw proj_exception();;
                    xy_x = (xy_y = 1. / xy_y) * cosphi * sin(lp_lon);
                    switch (this->m_proj_parm.mode) {
                    case EQUIT:
                        xy_y *= sinphi;
                        break;
                    case OBLIQ:
                        xy_y *= this->m_proj_parm.cosph0 * sinphi - this->m_proj_parm.sinph0 * cosphi * coslam;
                        break;
                    case N_POLE:
                        coslam = - coslam;
                    case S_POLE:
                        xy_y *= cosphi * coslam;
                        break;
                    }
                }

                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    double  rh, cosz, sinz;

                    rh = boost::math::hypot(xy_x, xy_y);
                    sinz = sin(lp_lat = atan(rh));
                    cosz = sqrt(1. - sinz * sinz);
                    if (fabs(rh) <= EPS10) {
                        lp_lat = this->m_par.phi0;
                        lp_lon = 0.;
                    } else {
                        switch (this->m_proj_parm.mode) {
                        case OBLIQ:
                            lp_lat = cosz * this->m_proj_parm.sinph0 + xy_y * sinz * this->m_proj_parm.cosph0 / rh;
                            if (fabs(lp_lat) >= 1.)
                                lp_lat = lp_lat > 0. ? HALFPI : - HALFPI;
                            else
                                lp_lat = asin(lp_lat);
                            xy_y = (cosz - this->m_proj_parm.sinph0 * sin(lp_lat)) * rh;
                            xy_x *= sinz * this->m_proj_parm.cosph0;
                            break;
                        case EQUIT:
                            lp_lat = xy_y * sinz / rh;
                            if (fabs(lp_lat) >= 1.)
                                lp_lat = lp_lat > 0. ? HALFPI : - HALFPI;
                            else
                                lp_lat = asin(lp_lat);
                            xy_y = cosz * rh;
                            xy_x *= sinz;
                            break;
                        case S_POLE:
                            lp_lat -= HALFPI;
                            break;
                        case N_POLE:
                            lp_lat = HALFPI - lp_lat;
                            xy_y = -xy_y;
                            break;
                        }
                        lp_lon = atan2(xy_x, xy_y);
                    }
                }
            };

            // Gnomonic
            template <typename Parameters>
            void setup_gnom(Parameters& par, par_gnom& proj_parm)
            {
                if (fabs(fabs(par.phi0) - HALFPI) < EPS10)
                    proj_parm.mode = par.phi0 < 0. ? S_POLE : N_POLE;
                else if (fabs(par.phi0) < EPS10)
                    proj_parm.mode = EQUIT;
                else {
                    proj_parm.mode = OBLIQ;
                    proj_parm.sinph0 = sin(par.phi0);
                    proj_parm.cosph0 = cos(par.phi0);
                }
                // par.inv = s_inverse;
                // par.fwd = s_forward;
                par.es = 0.;
            }

        }} // namespace detail::gnom
    #endif // doxygen

    /*!
        \brief Gnomonic projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Azimuthal
         - Spheroid
        \par Example
        \image html ex_gnom.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct gnom_spheroid : public detail::gnom::base_gnom_spheroid<Geographic, Cartesian, Parameters>
    {
        inline gnom_spheroid(const Parameters& par) : detail::gnom::base_gnom_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::gnom::setup_gnom(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class gnom_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<gnom_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void gnom_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("gnom", new gnom_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

}}} // namespace boost::geometry::projection

#endif // BOOST_GEOMETRY_PROJECTIONS_GNOM_HPP


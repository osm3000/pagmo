/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#ifndef PLANET_H
#define PLANET_H

#include "astro_constants.h"
#include "epoch.h"
#include <vector>
#include <string>

namespace kep_toolbox{

    /// planet class.
    /**
 * This class is intended to represent bodies in a keplerian orbit around a primary body. A planet
 * is described by its gravity, the gravity of the body it is orbiting, its radius, its safe radius
 * and its orbit. The orbit is internally represented by the planet cartesian coordinates
 *  at a given reference epoch.
 *
 * @author Dario Izzo (dario.izzo _AT_ googlemail.com)
 */

    class planet
    {
        friend std::ostream &operator<<(std::ostream &, const planet &);
    public:
        enum common_name {MERCURY=1, VENUS=2, EARTH=3, MARS=4, JUPITER=5, SATURN=6, NEPTUNE=7, URANUS=8};
        /// Constructor
        /**
                * Constructs a planet from its elements and its phyisical parameters
                * \param[in] ref_epoch epoch to which the elements are referred to
                * \param[in] elem A STL vector containing the keplerian parameters (a,e,i,Om,om,M). AU and degrees are assumed
                * \param[in] mu_central_body The gravitational parameter of the attracting body (SI units)
                * \param[in] radius The epoch to which the mean anomaly is referred to
                * \param[in] mu_self The gravitational parameter of the planet (SI units)
                */
        planet(const epoch& ref_epoch, const array6D& elem, const double & mu_central_body, const double & radius, const double &mu_self);

        /// Constructor
        /**
                * Construct a planet from its common name (e.g. VENUS)
                * \param[in] name One of the available names in planet::common_name
                */
        planet(const common_name& name);
        /**
                * Construct a planet from its common name (e.g. VENUS)
                * \param[in] name a string describing a planet
                */
	planet(const std::string& name);
	
        /** @name Getters and Setters */
        //@{
        /// Gets the planet position and velocity
        /**
                * \param[in] when Epoch in which ephemerides are required
                * \param[out] r Planet position at epoch (SI units)
                * \param[out] v Planet velocity at epoch (SI units)
                */
        void get_eph(const epoch& when, array3D &r, array3D &v) const;

        /// Getter for the central body gravitational parameter
        /**
         * Gets the gravitational parameter of the central body
         *
         * @return mu_central_body (SI Units)
         */
        double get_mu_central_body() const {return mu_central_body;}
	
        /// Getter for the planet gravitational parameter
        /**
         * Gets the gravitational parameter of the planet
         *
         * @return mu_self (SI Units)
         */
        double get_mu_self() const {return mu_self;}

        /// Getter for the planet radius
        /**
         * Gets the radius of the planet
         *
         * @return const reference to radius (SI Units)
         */
        const double& get_radius() const {return radius;}

        /// Getter for the planet safe-radius
        /**
         * Gets the safe-radius of the planet. This is intended to be the minimum distance
         * from the planet center that is safe ... It may be used, for example,  during fly-bys as a constarint
         * on the spacecraft trajectory
         *
         * @return const reference to safe_radius (SI Units)
         */
        const double& get_safe_radius() const {return safe_radius;}

        //@}

        /** @name Ephemerides calculations */
        //@{
        /// Returns the planet position
        /**
         * \param[in] when Epoch in which position is requested
         *
         * @return a boost array containing the planet position in epoch (SI Units)
         */
        array3D get_position(const epoch& when) const;

        /// Returns the planet velocity
        /**
          * \param[in] when Epoch in which velocity is requested
          *
          * @return a boost array containing the planet velocity in epoch (SI Units)
          */

        array3D get_velocity(const epoch& when) const;

        /// Returns the planet orbital elements (a,e,i,Om,om,M)
        /**
         * \param[in] when Epoch in which orbital elements are required
         *
         * @return a boost array containing the planet elements in epoch (SI Units) (a,e,i,Om,om,M)
         */
        //@}

        array6D get_elements(const epoch& when) const;


    private:
	void initialize_planet(planet::common_name name);

	array6D keplerian_elements;
	double time_coefficient;
        double ref_mjd2000;
        double radius;
        double safe_radius;
        double mu_self;
        double mu_central_body;

	mutable epoch cached_epoch;
	mutable array3D cached_r;
	mutable array3D cached_v;
    };

    /// Overload the stream operator for kep_toolbox::planet
    /**
     * Streams out the planet object in a human readable format
     *
     * \param[in] s stream to which the planet will be sent
     * \param[in] body planet to be sent to stream
     *
     * \return reference to s
     *
     */
    std::ostream &operator<<(std::ostream & s, const planet &body);
} /// End of namespace kep_toolbox
#endif // PLANET_H
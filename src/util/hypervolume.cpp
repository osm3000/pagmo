/*****************************************************************************
 *   Copyright (C) 2004-2013 The PaGMO development team,                     *
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

#include "hypervolume.h"

namespace pagmo { namespace util {

/// Constructor from population
/**
 * Constructs a hypervolume object, where points are elicited from the referenced population object.
 *
 * @param[in] pop reference to population object from which Pareto front is computed
 * @param[in] verify flag stating whether the points should be verified after the construction. This turns off the validation for the further computation as well, use 'set_verify' flag to alter it later.
 */
hypervolume::hypervolume(boost::shared_ptr<population> pop, const bool verify) : m_copy_points(true), m_verify(verify) {
	std::vector<std::vector<population::size_type> > pareto_fronts = pop->compute_pareto_fronts();
	m_points.resize(pareto_fronts[0].size());
	for (population::size_type idx = 0 ; idx < pareto_fronts[0].size() ; ++idx) {
		m_points[idx] = fitness_vector(pop->get_individual(pareto_fronts[0][idx]).cur_f);
	}

	if (m_verify) {
		verify_after_construct();
	}
}

/// Constructor from a vector of points
/**
 * Constructs a hypervolume object from a provided set of points.
 *
 * @param[in] points vector of points for which the hypervolume is computed
 * @param[in] verify flag stating whether the points should be verified after the construction. This turns off the validation for the further computation as well, use 'set_verify' flag to alter it later.
 */
hypervolume::hypervolume(const std::vector<fitness_vector> &points, const bool verify) : m_points(points), m_copy_points(true), m_verify(verify) {
	if  (m_verify) {
		verify_after_construct();
	}
}

/// Setter for m_copy_points flag
/**
 * Sets the hypervolume as a single use object.
 * It is used in cases where we are certain that we can alter the original set of points from the hypervolume object.
 * This is useful when we don't want to make a copy of the points first, as most algorithms alter the original set.
 *
 * This may result in unexpected behaviour when used incorrectly (e.g. requesting the computation twice out of the same object)
 *
 * @param[in] copy_points boolean value stating whether the hypervolume computation may use original set
 */
void hypervolume::set_copy_points(const bool copy_points) {
	m_copy_points = copy_points;
}

/// Getter for m_copy_points flag
bool hypervolume::get_copy_points() {
	return m_copy_points;
}

/// Setter the m_verify flag
/**
 * Turns off the verification phase.
 * By default, the hypervolume object verifies whether certain characteristics of the point set before computing, such as valid dimension sizes, or matchin reference point.
 * In order to optimize the computation when the rules above are certain, we can turn off that phase.
 *
 * This may result in unexpected behaviour when used incorrectly (e.g. requesting the computation of empty set of points)
 *
 * @param[in] verify boolean value stating whether the hypervolume computation is to be executed without verification
 */
void hypervolume::set_verify(const bool verify) {
	m_verify = verify;
}

/// Getter for the m_verify flag
bool hypervolume::get_verify() {
	return m_verify;
}

/// Copy constructor.
/**
 * Will perform a deep copy of hypervolume object
 *
 * @param[in] hv hypervolume object to be copied
 */
hypervolume::hypervolume(const hypervolume &hv): m_points(hv.m_points) { }


/// Default constructor
/**
 * Initiates hypervolume with empty set of points.
 * Used for serialization purposes.
 */
hypervolume::hypervolume() {
	m_points.resize(0);
}

/// verify after construct
/**
 * Verifies whether basic requirements are met for the initial set of points.
 *
 * @throws value_error if point size is empty or when the dimensions among the points differ
 */
void hypervolume::verify_after_construct() const {
	if ( m_points.size() == 0 ) {
		pagmo_throw(value_error, "Point set cannot be empty.");
	}
	fitness_vector::size_type f_dim = m_points[0].size();
	if (f_dim <= 1) {
		pagmo_throw(value_error, "Points of dimension > 1 required.");
	}
	for (std::vector<fitness_vector>::size_type idx = 1 ; idx < m_points.size() ; ++idx) {
		if ( m_points[idx].size() != f_dim ) {
			pagmo_throw(value_error, "All point set dimensions must be equal.");
		}
	}
}

// verify before compute
/**
 * Verifies whether reference point and the hypervolume method meet certain criteria.
 *
 * @param[in] r_point fitness vector describing the reference point
 *
 * @throws value_error if reference point's and point set dimension do not agree
 */
void hypervolume::verify_before_compute(const fitness_vector &r_point, hv_algorithm::base_ptr hv_algorithm) const {
	if ( m_points[0].size() != r_point.size() ) {
		pagmo_throw(value_error, "Point set dimensions and reference point dimension must be equal.");
	}
	hv_algorithm->verify_before_compute(m_points, r_point);
}

// choose the best method for given dimension
hv_algorithm::base_ptr hypervolume::get_best_method(const fitness_vector &r_point) const {
	switch(r_point.size()) {
		case 2:
			return hv_algorithm::base_ptr(new hv_algorithm::native2d());
			break;
		case 3:
			return hv_algorithm::base_ptr(new hv_algorithm::beume3d());
			break;
		default:
			return hv_algorithm::base_ptr(new hv_algorithm::wfg());
	}
}

// compute hypervolume
/**
 * Computes hypervolume provided a reference point and an algorithm object.
 *
 * @param[in] r_point fitness vector describing the reference point
 * @param[in] hv_algorithm algorithm object used for computing the hypervolume
 *
 * @return value representing the hypervolume
 */
double hypervolume::compute(const fitness_vector &r_point, hv_algorithm::base_ptr hv_algorithm) const {

	if (m_verify) {
		verify_before_compute(r_point, hv_algorithm);
	}

	// copy the initial set of points, as the algorithm may alter its contents
	if (m_copy_points) {
		std::vector<fitness_vector> points_cpy(m_points.begin(), m_points.end());
		return hv_algorithm->compute(points_cpy, r_point);
	} else {
		return hv_algorithm->compute(const_cast<std::vector<fitness_vector> &>(m_points), r_point);
	}
}

// compute hypervolume
/**
 * Computes hypervolume provided a reference point and an algorithm object.
 * This method chooses the hv_algorithm dynamically.
 *
 * @param[in] r_point fitness vector describing the reference point
 * @param[in] hv_algorithm algorithm object used for computing the hypervolume
 *
 * @return value representing the hypervolume
 */
double hypervolume::compute(const fitness_vector &r_point) const {
	return compute(r_point, get_best_method(r_point));
}

// compute exclusive contribution
/**
 * Computes exclusive hypervolume for given indivdual.
 *
 * @param[in] p_idx index of the individual for whom we compute the exclusive contribution to the hypervolume
 * @param[in] r_point fitness vector describing the reference point
 * @param[in] hv_algorithm algorithm object used for computing the hypervolume
 *
 * @return value representing the hypervolume
 */
double hypervolume::exclusive(const unsigned int p_idx, const fitness_vector &r_point, hv_algorithm::base_ptr hv_algorithm) const {

	if (m_verify) {
		verify_before_compute(r_point, hv_algorithm);
	}

	if (p_idx >= m_points.size()) {
		pagmo_throw(value_error, "Index of the individual is out of bounds.");

	}

	// copy the initial set of points, as the algorithm may alter its contents
	if (m_copy_points) {
		std::vector<fitness_vector> points_cpy(m_points.begin(), m_points.end());
		return hv_algorithm->exclusive(p_idx, points_cpy, r_point);
	} else {
		return hv_algorithm->exclusive(p_idx, const_cast<std::vector<fitness_vector> &>(m_points), r_point);
	}
}

// compute exclusive contribution
/**
 * Computes exclusive hypervolume for given indivdual.
 * This methods chooses the hv_algorithm dynamically.
 *
 * @param[in] p_idx index of the individual for whom we compute the exclusive contribution to the hypervolume
 * @param[in] r_point fitness vector describing the reference point
 *
 * @return value representing the hypervolume
 */
double hypervolume::exclusive(const unsigned int p_idx, const fitness_vector &r_point) const {
	return exclusive(p_idx, r_point, get_best_method(r_point));
}

// locate the least contributing individual
/**
 * Locates the individual contributing the least to the total hypervolume.
 *
 * @param[in] r_point fitness vector describing the reference point
 * @param[in] hv_algorithm algorithm object used for computation
 *
 * @return index of the least contributing point
 */
unsigned int hypervolume::least_contributor(const fitness_vector &r_point, hv_algorithm::base_ptr hv_algorithm) const {

	if (m_verify) {
		verify_before_compute(r_point, hv_algorithm);
	}

	// copy the initial set of points, as the algorithm may alter its contents
	if (m_copy_points) {
		std::vector<fitness_vector> points_cpy(m_points.begin(), m_points.end());
		return hv_algorithm->least_contributor(points_cpy, r_point);
	} else {
		return hv_algorithm->least_contributor(const_cast<std::vector<fitness_vector> &>(m_points), r_point);
	}
}

// locate the least contributing individual
/**
 * Locates the individual contributing the least to the total hypervolume.
 * This method chooses the best performing hv_algorithm dynamically
 *
 * @param[in] r_point fitness vector describing the reference point
 *
 * @return index of the least contributing point
 */
unsigned int hypervolume::least_contributor(const fitness_vector &r_point) const {
	return least_contributor(r_point, get_best_method(r_point));
}

// locate the most contributing individual
/**
 * Locates the individual contributing the most to the total hypervolume.
 *
 * @param[in] r_point fitness vector describing the reference point
 * @param[in] hv_algorithm algorithm object used for computation
 *
 * @return index of the most contributing point
 */
unsigned int hypervolume::greatest_contributor(const fitness_vector &r_point, hv_algorithm::base_ptr hv_algorithm) const {

	if (m_verify) {
		verify_before_compute(r_point, hv_algorithm);
	}

	// copy the initial set of points, as the algorithm may alter its contents
	if (m_copy_points) {
		std::vector<fitness_vector> points_cpy(m_points.begin(), m_points.end());
		return hv_algorithm->greatest_contributor(points_cpy, r_point);
	} else {
		return hv_algorithm->greatest_contributor(const_cast<std::vector<fitness_vector> &>(m_points), r_point);
	}
}

// locate the most contributing individual
/**
 * Locates the individual contributing the most to the total hypervolume.
 * This method chooses the best performing hv_algorithm dynamically
 *
 * @param[in] r_point fitness vector describing the reference point
 *
 * @return index of the most contributing point
 */
unsigned int hypervolume::greatest_contributor(const fitness_vector &r_point) const {
	return greatest_contributor(r_point, get_best_method(r_point));
}

/// get expected numer of operations
/**
 * Returns the expected average amount of elementary operations for given front size (n) and dimension size (d).
 * As of yet it is relatively simple way of handling that.
 *
 */
unsigned long long hypervolume::get_expected_operations(const unsigned int n, const unsigned int d) {
	if (d == 2) {
		return 2. * n * log(n);  // native2d
	} else if (d == 3) {
		return 3. * n * log(n);  // beume3d
	} else {
		return n * log(n) * pow(n,d/2);  // temporary complexity of hso, while we don't know good candiate for wfg yet
	}
}

// calculate the nadir point
/**
 * Calculates the nadir point, used as the reference point
 *
 * @param[in] epsilon value that is to be added to each objective to assure strict domination nadir point by each other point in a set
 *
 * @return value representing the hypervolume
 */
fitness_vector hypervolume::get_nadir_point(const double epsilon) const {
	fitness_vector nadir_point(m_points[0].begin(), m_points[0].end());
	for (std::vector<fitness_vector>::size_type idx = 1 ; idx < m_points.size() ; ++ idx){
		for (fitness_vector::size_type f_idx = 0 ; f_idx < m_points[0].size() ; ++f_idx){
			// assuming minimization problem, thus maximum value by each dimension is taken
			nadir_point[f_idx] = fmax(nadir_point[f_idx], m_points[idx][f_idx]);
		}
	}
	for (fitness_vector::size_type f_idx = 0 ; f_idx < nadir_point.size() ; ++f_idx) {
		nadir_point[f_idx] += epsilon;
	}
	return nadir_point;
}


/// Get points
/**
 * Will return a vector containing the points as they were set up during construction of the hypervolume object.
 *
 * @return const reference to the vector containing the fitness_vectors representing the points in the hyperspace.
 */
const std::vector<fitness_vector> &hypervolume::get_points() const {
	return m_points;
}

hypervolume_ptr hypervolume::clone() const
{
	return hypervolume_ptr(new hypervolume(*this));
}

}}

BOOST_CLASS_EXPORT_IMPLEMENT(pagmo::util::hypervolume);

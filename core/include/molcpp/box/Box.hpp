#ifndef MOLCPP_BOX_BOX_HPP
#define MOLCPP_BOX_BOX_HPP

#include <cmath>
#include <stdexcept>
#include <xtensor/xarray.hpp>
#include <xtensor/xfixed.hpp>
#include <xtensor/xmath.hpp>

#include "../types.hpp"

namespace molcpp {
namespace box {

//! Box class for handling periodic boundary conditions
/*! The Box class encapsulates all information about the simulation box, including
    its dimensions and periodic boundary conditions. It provides methods for
    wrapping positions, computing minimum image distances, and other box-related operations.
*/
class Box {
public:
    //! Default constructor creates a cubic box
    Box() : m_Lx(1.0), m_Ly(1.0), m_Lz(1.0), m_xy(0.0), m_xz(0.0), m_yz(0.0), 
            m_periodic_x(true), m_periodic_y(true), m_periodic_z(true), m_2d(false) {
        updateCachedValues();
    }

    //! Constructor with box dimensions
    /*! \param Lx Box length in x direction
        \param Ly Box length in y direction
        \param Lz Box length in z direction
        \param xy Tilt factor xy
        \param xz Tilt factor xz
        \param yz Tilt factor yz
        \param is_2d Whether this is a 2D box
    */
    Box(double Lx, double Ly, double Lz, double xy = 0.0, double xz = 0.0, double yz = 0.0, bool is_2d = false)
        : m_Lx(Lx), m_Ly(Ly), m_Lz(Lz), m_xy(xy), m_xz(xz), m_yz(yz),
          m_periodic_x(true), m_periodic_y(true), m_periodic_z(!is_2d), m_2d(is_2d) {
        if (Lx <= 0 || Ly <= 0 || (!is_2d && Lz <= 0)) {
            throw std::invalid_argument("Box dimensions must be positive");
        }
        updateCachedValues();
    }

    //! Get box length in x direction
    double getLx() const { return m_Lx; }
    
    //! Get box length in y direction
    double getLy() const { return m_Ly; }
    
    //! Get box length in z direction
    double getLz() const { return m_Lz; }
    
    //! Get tilt factor xy
    double getxy() const { return m_xy; }
    
    //! Get tilt factor xz
    double getxz() const { return m_xz; }
    
    //! Get tilt factor yz
    double getyz() const { return m_yz; }
    
    //! Check if box is 2D
    bool is2D() const { return m_2d; }
    
    //! Get box volume
    double getVolume() const { return m_volume; }

    //! Get the box vectors as a 3x3 matrix
    Mat3 getVectors() const {
        Mat3 vectors;
        vectors.fill(0.0);
        vectors(0, 0) = m_Lx;
        vectors(0, 1) = m_xy * m_Ly;
        vectors(0, 2) = m_xz * m_Lz;
        vectors(1, 1) = m_Ly;
        vectors(1, 2) = m_yz * m_Lz;
        vectors(2, 2) = m_Lz;
        return vectors;
    }

    //! Get the inverse box vectors
    const Mat3& getInverseVectors() const { return m_inv_vectors; }

    //! Get nearest plane distance
    Vec3 getNearestPlaneDistance() const {
        Vec3 dist;
        dist(0) = m_Lx / std::sqrt(1.0 + m_xy * m_xy + m_xz * m_xz);
        dist(1) = m_Ly / std::sqrt(1.0 + m_yz * m_yz);
        dist(2) = m_2d ? 0.0 : m_Lz;
        return dist;
    }

    //! Wrap a position into the box
    /*! \param pos Position to wrap
        \returns Wrapped position
    */
    Vec3 wrap(const Vec3& pos) const {
        Vec3 wrapped = pos;
        
        // Convert to fractional coordinates
        Vec3 frac;
        for (size_t i = 0; i < 3; ++i) {
            frac(i) = 0.0;
            for (size_t j = 0; j < 3; ++j) {
                frac(i) += m_inv_vectors(i, j) * pos(j);
            }
        }
        
        // Wrap fractional coordinates
        if (m_periodic_x) frac(0) = frac(0) - std::floor(frac(0));
        if (m_periodic_y) frac(1) = frac(1) - std::floor(frac(1));
        if (m_periodic_z && !m_2d) frac(2) = frac(2) - std::floor(frac(2));
        
        // Convert back to real coordinates
        wrapped.fill(0.0);
        Mat3 vectors = getVectors();
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                wrapped(i) += vectors(i, j) * frac(j);
            }
        }
        
        return wrapped;
    }

    //! Compute minimum image vector between two positions
    /*! \param r_i First position
        \param r_j Second position
        \returns Minimum image vector from r_i to r_j
    */
    Vec3 minimumImage(const Vec3& r_i, const Vec3& r_j) const {
        Vec3 dr = r_j - r_i;
        
        // Convert to fractional coordinates
        Vec3 frac;
        for (size_t i = 0; i < 3; ++i) {
            frac(i) = 0.0;
            for (size_t j = 0; j < 3; ++j) {
                frac(i) += m_inv_vectors(i, j) * dr(j);
            }
        }
        
        // Apply minimum image convention
        if (m_periodic_x) frac(0) = frac(0) - std::round(frac(0));
        if (m_periodic_y) frac(1) = frac(1) - std::round(frac(1));
        if (m_periodic_z && !m_2d) frac(2) = frac(2) - std::round(frac(2));
        
        // Convert back to real coordinates
        Vec3 dr_min;
        dr_min.fill(0.0);
        Mat3 vectors = getVectors();
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                dr_min(i) += vectors(i, j) * frac(j);
            }
        }
        
        return dr_min;
    }

    //! Compute minimum image distance between two positions
    /*! \param r_i First position
        \param r_j Second position
        \returns Minimum image distance
    */
    double distance(const Vec3& r_i, const Vec3& r_j) const {
        Vec3 dr = minimumImage(r_i, r_j);
        return std::sqrt(dr(0) * dr(0) + dr(1) * dr(1) + dr(2) * dr(2));
    }

    //! Set periodic boundary conditions
    void setPeriodic(bool x, bool y, bool z) {
        m_periodic_x = x;
        m_periodic_y = y;
        m_periodic_z = z && !m_2d;
    }

    //! Check if x direction is periodic
    bool isPeriodicX() const { return m_periodic_x; }
    
    //! Check if y direction is periodic
    bool isPeriodicY() const { return m_periodic_y; }
    
    //! Check if z direction is periodic
    bool isPeriodicZ() const { return m_periodic_z; }

private:
    //! Update cached values when box parameters change
    void updateCachedValues() {
        // Compute volume
        if (m_2d) {
            m_volume = m_Lx * m_Ly;
        } else {
            m_volume = m_Lx * m_Ly * m_Lz * (1.0 - m_xy * m_yz * m_xz);
        }
        
        // Compute inverse box vectors
        Mat3 vectors = getVectors();
        double det = vectors(0, 0) * (vectors(1, 1) * vectors(2, 2) - vectors(1, 2) * vectors(2, 1))
                   - vectors(0, 1) * (vectors(1, 0) * vectors(2, 2) - vectors(1, 2) * vectors(2, 0))
                   + vectors(0, 2) * (vectors(1, 0) * vectors(2, 1) - vectors(1, 1) * vectors(2, 0));
        
        if (std::abs(det) < 1e-10) {
            // For 2D or degenerate boxes
            m_inv_vectors.fill(0.0);
            if (m_Lx > 0) m_inv_vectors(0, 0) = 1.0 / m_Lx;
            if (m_Ly > 0) {
                m_inv_vectors(0, 1) = -m_xy / m_Lx;
                m_inv_vectors(1, 1) = 1.0 / m_Ly;
            }
            if (!m_2d && m_Lz > 0) {
                m_inv_vectors(0, 2) = (m_xy * m_yz - m_xz) / m_Lx;
                m_inv_vectors(1, 2) = -m_yz / m_Ly;
                m_inv_vectors(2, 2) = 1.0 / m_Lz;
            }
        } else {
            // General 3D case
            m_inv_vectors(0, 0) = (vectors(1, 1) * vectors(2, 2) - vectors(1, 2) * vectors(2, 1)) / det;
            m_inv_vectors(0, 1) = (vectors(0, 2) * vectors(2, 1) - vectors(0, 1) * vectors(2, 2)) / det;
            m_inv_vectors(0, 2) = (vectors(0, 1) * vectors(1, 2) - vectors(0, 2) * vectors(1, 1)) / det;
            m_inv_vectors(1, 0) = (vectors(1, 2) * vectors(2, 0) - vectors(1, 0) * vectors(2, 2)) / det;
            m_inv_vectors(1, 1) = (vectors(0, 0) * vectors(2, 2) - vectors(0, 2) * vectors(2, 0)) / det;
            m_inv_vectors(1, 2) = (vectors(0, 2) * vectors(1, 0) - vectors(0, 0) * vectors(1, 2)) / det;
            m_inv_vectors(2, 0) = (vectors(1, 0) * vectors(2, 1) - vectors(1, 1) * vectors(2, 0)) / det;
            m_inv_vectors(2, 1) = (vectors(0, 1) * vectors(2, 0) - vectors(0, 0) * vectors(2, 1)) / det;
            m_inv_vectors(2, 2) = (vectors(0, 0) * vectors(1, 1) - vectors(0, 1) * vectors(1, 0)) / det;
        }
    }

    double m_Lx, m_Ly, m_Lz;           //!< Box dimensions
    double m_xy, m_xz, m_yz;           //!< Tilt factors
    bool m_periodic_x, m_periodic_y, m_periodic_z; //!< Periodic flags
    bool m_2d;                         //!< 2D box flag
    double m_volume;                   //!< Cached volume
    Mat3 m_inv_vectors;                //!< Cached inverse box vectors
};

} // namespace box
} // namespace molcpp

#endif // MOLCPP_BOX_BOX_HPP
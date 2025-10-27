/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Copyright (c) 2025, Davide Stocco and Enrico Bertolazzi.                                      *
 *                                                                                               *
 * The Pipal project is distributed under the MIT License.                                       *
 *                                                                                               *
 * Davide Stocco                                                               Enrico Bertolazzi *
 * University of Trento                                                     University of Trento *
 * e-mail: davide.stocco@unitn.it                             e-mail: enrico.bertolazzi@unitn.it *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// STL includes
#include <memory>
#include <limits>

// GTest library
#include <gtest/gtest.h>

// Pipal includes
#include "Pipal.hh"

using Pipal::Integer;
using Real         = double;
using Vector       = Pipal::Vector<Real>;
using Matrix       = Pipal::Matrix<Real>;
using SparseMatrix = Pipal::SparseMatrix<Real>;

constexpr bool    VERBOSE{false};
constexpr Real    SOLVER_TOLERANCE{1.0e-10};
constexpr Real    APPROX_TOLERANCE{1.0e-3};
constexpr Integer MAX_ITERATIONS{100};

TEST(Test1, ProblemWrapper) {
  Pipal::Solver<Real> solver("test_rosenbrock_suzuki",
    [] (Vector const & x, Real & out) { // Objective function
      out = x(0)*x(0) + x(1)*x(1) + 2.0*x(2)*x(2) + x(3)*x(3) - 5.0*x(0) - 5.0*x(1) - 21.0*x(2) + 7.0*x(3);
      return std::isfinite(out);
    },
    [] (Vector const & x, Vector & out) { // Gradient of the objective function
      out.resize(4);
      out << 2.0*x(0)-5.0, 2.0*x(1)-5.0, 4.0*x(2)-21.0, 2.0*x(3)+7.0;
      return out.allFinite();
    },
    [] (Vector const & x, Vector & out) { // Constraints function
      out.resize(4);
      out <<
        (x(0)*x(0) + x(1)*x(1) + x(2)*x(2) + x(3)*x(3) + x(0) - x(1) + x(2) - x(3)) - 8.0,
        (x(0)*x(0) + 2.0*x(1)*x(1) + x(2)*x(2) + 2.0*x(3)*x(3) - x(0) - x(3)) - 10.0,
        (2.0*x(0)*x(0) + x(1)*x(1) + x(2)*x(2) + 2.0*x(0) - x(1) - x(3)) - 5.0,
        (x(0) + x(1) + x(2) + x(3) - 5.0);
      return out.allFinite();
    },
    [] (Vector const & x, SparseMatrix & out) { // Jacobian of the constraints function
      out.resize(4,4);
      std::vector<Eigen::Triplet<Real>> triplets{
        {0, 0, 2.0*x(0) + 1.0},
        {0, 1, 2.0*x(1) - 1.0},
        {0, 2, 2.0*x(2) + 1.0},
        {0, 3, 2.0*x(3) - 1.0},
        
        {1, 0, 2.0*x(0) - 1.0},
        {1, 1, 4.0*x(1)},
        {1, 2, 2.0*x(2)},
        {1, 3, 4.0*x(3) - 1.0},
        
        {2, 0, 4.0*x(0) + 2.0},
        {2, 1, 2.0*x(1) - 1.0},
        {2, 2, 2.0*x(2)},
        {2, 3, -1.0},
        
        {3, 0, 1},
        {3, 1, 1},
        {3, 2, 1},
        {3, 3, 1}
      };
      out.setFromTriplets(triplets.begin(), triplets.end());
      Eigen::Map<Vector> vec( out.valuePtr(), out.nonZeros() );
      return vec.allFinite();
    },
    [] (Vector const &, Vector const & z, SparseMatrix & out) -> bool { // Hessian of the Lagrangian
      out.resize(4,4);
      std::vector<Eigen::Triplet<Real>> triplets{
        {0, 0, 2 + z(0)*2 + z(1)*2 + z(2)*4},
        {1, 1, 2 + z(0)*2 + z(1)*4 + z(2)*2},
        {2, 2, 4 + z(0)*2 + z(1)*2 + z(2)*2},
        {3, 3, 2 + z(0)*2 + z(1)*4 + z(2)*0}
      };
      out.setFromTriplets(triplets.begin(), triplets.end());
      Eigen::Map<Vector> vec( out.valuePtr(), out.nonZeros() );
      return vec.allFinite();
    },
    [] (Vector & out) {out.setConstant(4, -INFINITY); out(0) = -3000.0; return true;}, // Lower bounds on the primal variables
    [] (Vector & out) {out.setConstant(4, INFINITY); return true;}, // Upper bounds on the primal variables
    [] (Vector & out) {out.setConstant(4, -INFINITY); out(3) = 0.0; return true;}, // Lower bounds on the constraints
    [] (Vector & out) {out.setConstant(4, 0.0); return true;}  // Upper bounds on the constraints
  );
  solver.algorithm(Pipal::Algorithm::ADAPTIVE);
  solver.verbose_mode(VERBOSE);
  solver.tolerance(SOLVER_TOLERANCE);
  solver.max_iterations(MAX_ITERATIONS);
  Vector x_sol(4), x_guess(4), x_opt(4);
  x_guess << -4000.0, 1.0, 1.0, 1.0;
  x_opt << 0.287054, 1.44787, 2.16978, 1.09530;
  EXPECT_TRUE(solver.optimize(x_guess, x_sol));
  EXPECT_TRUE(x_sol.isApprox(x_opt, APPROX_TOLERANCE));
}

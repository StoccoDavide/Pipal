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
  Pipal::Solver<Real> solver("test_rosenbrock_box",
    [] (Vector const & x, Real & out) { // Objective function
      out = 100.0*std::pow(x(1) - x(0)*x(0), 2.0) + std::pow(1 - x(0), 2.0);
      return std::isfinite(out);
    },
    [] (Vector const & x, Vector & out) { // Gradient of the objective function
      out.resize(2);
      out << -400.0*x(0)*(x(1)-x(0)*x(0)) - 2.0*(1 - x(0)), 200.0*(x(1) - x(0)*x(0));
      return out.allFinite();
    },
    [] (Vector const &, Vector & out) { // Constraints function
      out.resize(0);
      return true;
    },
    [] (Vector const &, SparseMatrix & out) { // Jacobian of the constraints function
      out.resize(0,0);
      return true;
    },
    [] (Vector const & x, Vector const &, SparseMatrix & out) ->bool { // Hessian of the Lagrangian
      out.resize(2,2);
      std::vector<Eigen::Triplet<Real>> triplets{
        {0, 0, 1200.0*x(0)*x(0) - 400.0*x(1) + 2},
        {0, 1, -400.0*x(0)},
        {1, 0, -400.0*x(0)},
        {1, 1, 200}
      };
      out.setFromTriplets(triplets.begin(), triplets.end());
      Eigen::Map<Vector> vec( out.valuePtr(), out.nonZeros() );
      return vec.allFinite();
    },
    [] (Vector & out) {out.setConstant(2, -1.0); return true;}, // Lower bounds on the primal variables
    [] (Vector & out) {out.setConstant(2, +1.0); return true;}, // Upper bounds on the primal variables
    [] (Vector & out) {out.resize(0); return true;}, // Lower bounds on the constraints
    [] (Vector & out) {out.resize(0); return true;}  // Upper bounds on the constraints
  );
  solver.algorithm(Pipal::Algorithm::CONSERVATIVE);
  solver.verbose_mode(VERBOSE);
  solver.tolerance(SOLVER_TOLERANCE);
  solver.max_iterations(MAX_ITERATIONS);
  Vector x_sol(2), x_guess(2), x_opt(2);
  x_guess.setZero();
  x_opt << 1.0, 1.0;
  EXPECT_TRUE(solver.optimize(x_guess, x_sol));
  EXPECT_TRUE(x_sol.isApprox(x_opt, APPROX_TOLERANCE));
}

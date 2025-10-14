/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Copyright (c) 2025, Davide Stocco and Enrico Bertolazzi.                                      *
 *                                                                                               *
 * The Pipal project is distributed under the MIT License.                                       *
 *                                                                                               *
 * Davide Stocco                                                               Enrico Bertolazzi *
 * University of Trento                                                     University of Trento *
 * e-mail: davide.stocco@unitn.it                             e-mail: enrico.bertolazzi@unitn.it *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#pragma once

#ifndef INCLUDE_PIPAL_PROBLEM_HH
#define INCLUDE_PIPAL_PROBLEM_HH

// Standard libraries
#include <functional>
#include <type_traits>

// Eigen library
#include <Eigen/Dense>

namespace Pipal {

  /**
   * \brief Problem class for the Pipal library.
   *
   * The Problem class serves as a base class for defining optimization problems in the Pipal library.
   * It provides a structure for encapsulating the problem's objective function, constraints, and
   * other necessary components.
   * \tparam Real The floating-point type.
   * \tparam N The size of the primal variable vector.
   * \tparam M The size of the dual variable vector.
   */
  template<typename Real, Integer N, Integer M>
  class Problem
  {
    static_assert(std::is_floating_point<Real>::value,
      "Pipal::Problem: template argument 'Real' must be a floating-point type.");
    static_assert(N > 0,
      "Pipal::Problem: template argument 'N' must be positive.");
    static_assert(M > 0,
      "Pipal::Problem: template argument 'M' must be positive.");

  public:
    using VectorN = Eigen::Vector<Real, N>;
    using VectorM = Eigen::Vector<Real, M>;
    using MatrixH = Eigen::Matrix<Real, N, N>;
    using MatrixJ = Eigen::Matrix<Real, M, N>;

    /**
     * \brief Default constructor.
     *
     * Initializes the problem with default values for the objective, gradient, hessian, constraints
     * and Jacobian functions.
     */
    Problem() {};

    /**
     * \brief Default copy constructor.
     */
    Problem(const Problem &) = default;

    /**
     * \brief Default move constructor.
     */
    Problem(Problem &&) = default;

    /**
     * \brief Default destructor.
     */
    virtual ~Problem() = default;

    /**
     * \brief Evaluate the objective function.
     * \param[in] x Primal variable vector.
     * \return The objective function.
     */
    virtual Real objective(VectorN const & x) const = 0;

    /**
     * \brief Evaluate the gradient of the objective function.
     * \param[in] x Primal variable vector.
     * \return The gradient of the objective function.
     */
    virtual VectorN objective_gradient(VectorN const & x) const = 0;

    /**
     * \brief Evaluate the Hessian of the objective function.
     * \param[in] x Primal variable vector.
     * \return The Hessian matrix of the objective function.
     */
    virtual MatrixH objective_hessian(VectorN const & x) const = 0;

    /**
     * \brief Evaluate the constraints function.
     * \param[in] x Primal variable vector.
     * \return The value of the constraints function.
     */
    virtual VectorM constraints(VectorN const & x) const = 0;

    /**
     * \brief Evaluate the Jacobian of the constraints function with respect to the primal variables.
     * \param[in] x Primal variable vector.
     * \param[in] z Dual variable vector.
     * \return The Jacobian matrix of the constraints function.
     */
    virtual MatrixJ constraints_jacobian(VectorN const & x, VectorM const & z) const = 0;

    /**
     * \brief Evaluate the Hessian of the Lagrangian function with respect to the primal variables.
     * \param[in] x Primal variable vector.
     * \param[in] z Dual variable vector.
     * \return The Hessian matrix of the Lagrangian function.
     */
    virtual MatrixH lagrangian_hessian(VectorN const & x, VectorM const & z) const = 0;

  }; // class Problem


  /**
  * \brief Wrapper class for the Problem class.
  *
  * The ProblemWrapper class is a simple wrapper around the Problem class. It inherits from the
  * Problem class and can be used to define optimization problems through function handles.
  * \tparam Real The floating-point type.
  * \tparam N The size of the primal variable vector.
  * \tparam M The size of the dual variable vector.
  */
  template<typename Real, Integer N, Integer M>
  class ProblemWrapper : public Problem<Real, N, M>
  {
  public:
    using VectorN = typename Problem<Real, N, M>::VectorN;
    using VectorM = typename Problem<Real, N, M>::VectorM;
    using MatrixH = typename Problem<Real, N, M>::MatrixH;
    using MatrixJ = typename Problem<Real, N, M>::MatrixJ;

    using ObjectiveFunc           = std::function<Real(VectorN const &)>;
    using ObjectiveGradientFunc   = std::function<VectorN(VectorN const &)>;
    using ObjectiveHessianFunc    = std::function<MatrixH(VectorN const &)>;
    using ConstraintsFunc         = std::function<VectorM(VectorN const &)>;
    using ConstraintsJacobianFunc = std::function<MatrixJ(VectorN const &, VectorM const &)>;
    using LagrangianHessianFunc   = std::function<MatrixH(VectorN const &, VectorM const &)>;

  private:
    ObjectiveFunc           m_objective{nullptr};            /*!< Objective function \f$ f(\mathbf{x}) \f$ */
    ObjectiveGradientFunc   m_objective_gradient{nullptr};   /*!< Gradient of the objective function \f$ \nabla f(\mathbf{x}) \f$ */
    ObjectiveHessianFunc    m_objective_hessian{nullptr};    /*!< Hessian of the objective function \f$ \nabla^2 f(\mathbf{x}) \f$ */
    ConstraintsFunc         m_constraints{nullptr};          /*!< Constraints function \f$ g(\mathbf{x}) \f$ */
    ConstraintsJacobianFunc m_constraints_jacobian{nullptr}; /*!< Jacobian of the constraints \f$ J(\mathbf{x}) \f$ */
    LagrangianHessianFunc   m_lagrangian_hessian{nullptr};   /*!< Hessian of the Lagrangian \f$ W(\mathbf{x}, \mathbf{z}) \f$ */

  public:
    /**
     * \brief Constructor for the ProblemWrapper class (without the Hessian of the Lagrangian).
     *
     * Initializes the problem with the provided objective, gradient, constraints, and Jacobian
     * functions.
     * \param[in] objective Objective function handle.
     * \param[in] objective_gradient Gradient of the objective function handle.
     * \param[in] constraints Constraints function handle.
     * \param[in] constraints_jacobian Jacobian of the constraints function handle.
     * \param[in] lagrangian_hessian Hessian of the Lagrangian function handle.
     */
    ProblemWrapper(ObjectiveFunc const & objective, ObjectiveGradientFunc const & objective_gradient,
      ConstraintsFunc const & constraints, ConstraintsJacobianFunc const & constraints_jacobian,
      LagrangianHessianFunc const & lagrangian_hessian)
      : m_objective(objective), m_objective_gradient(objective_gradient), m_constraints(constraints),
        m_constraints_jacobian(constraints_jacobian), m_lagrangian_hessian(lagrangian_hessian) {}

    /**
     * \brief Constructor for the ProblemWrapper class (with the Hessian of the Lagrangian).
     *
     * Initializes the problem with the provided objective, gradient, constraints, Jacobian, and
     * Hessian functions.
     * \param[in] objective Objective function handle.
     * \param[in] objective_gradient Gradient of the objective function handle.
     * \param[in] objective_hessian Hessian of the objective function handle.
     * \param[in] constraints Constraints function handle.
     * \param[in] constraints_jacobian Jacobian of the constraints function handle.
     * \param[in] lagrangian_hessian Hessian of the Lagrangian function handle.
     */
    ProblemWrapper(ObjectiveFunc const & objective, ObjectiveGradientFunc const & objective_gradient,
      ObjectiveHessianFunc const & objective_hessian, ConstraintsFunc const & constraints,
      ConstraintsJacobianFunc const & constraints_jacobian, LagrangianHessianFunc const & lagrangian_hessian)
      : m_objective(objective), m_objective_gradient(objective_gradient),
        m_objective_hessian(objective_hessian), m_constraints(constraints),
        m_constraints_jacobian(constraints_jacobian), m_lagrangian_hessian(lagrangian_hessian) {}

    /**
     * \brief Default destructor for the ProblemWrapper class.
     */
    ~ProblemWrapper() {};

    /**
     * \brief Get the objective function.
     * \return The objective function.
     */
    ObjectiveFunc objective() const
    {
      return this->m_objective;
    }

    /**
     * \brief Set the objective function.
     * \param[in] objective The objective function to set.
     */
    void objective(ObjectiveFunc const & objective)
    {
      this->m_objective = objective;
    }

    /**
     * \brief Get the gradient of the objective function.
     * \return The gradient of the objective function.
     */
    ObjectiveGradientFunc objective_gradient() const
    {
      return this->m_objective_gradient;
    }

    /**
     * \brief Set the gradient of the objective function.
     * \param[in] objective_gradient The gradient of the objective function to set.
     */
    void objective_gradient(ObjectiveGradientFunc const & objective_gradient)
    {
      this->m_objective_gradient = objective_gradient;
    }

    /**
     * \brief Get the Hessian of the objective function.
     * \return The Hessian of the objective function.
     */
    ObjectiveHessianFunc objective_hessian() const
    {
      return this->m_objective_hessian;
    }

    /**
     * \brief Set the Hessian of the objective function.
     * \param[in] objective_hessian The Hessian of the objective function to set.
     */
    void objective_hessian(ObjectiveHessianFunc const & objective_hessian)
    {
      this->m_objective_hessian = objective_hessian;
    }

    /**
     * \brief Get the constraints function.
     * \return The constraints function.
     */
    ConstraintsFunc constraints() const
    {
      return this->m_constraints;
    }

    /**
     * \brief Set the constraints function.
     * \param[in] constraints The constraints function to set.
     */
    void constraints(ConstraintsFunc const & constraints)
    {
      this->m_constraints = constraints;
    }

    /**
     * \brief Get the Jacobian of the constraints function.
     * \return The Jacobian of the constraints function.
     */
    ConstraintsJacobianFunc constraints_jacobian() const
    {
      return this->m_constraints_jacobian;
    }

    /**
     * \brief Set the Jacobian of the constraints function.
     * \param[in] constraints_jacobian The Jacobian of the constraints function to set.
     */
    void constraints_jacobian(ConstraintsJacobianFunc const & constraints_jacobian)
    {
      this->m_constraints_jacobian = constraints_jacobian;
    }

    /**
     * \brief Get the Hessian of the Lagrangian function.
     * \return The Hessian of the Lagrangian function.
     */
    LagrangianHessianFunc lagrangian_hessian() const
    {
      return this->m_lagrangian_hessian;
    }

    /**
     * \brief Set the Hessian of the Lagrangian function.
     * \param[in] lagrangian_hessian The Hessian of the Lagrangian function to set.
     */
    void lagrangian_hessian(LagrangianHessianFunc const & lagrangian_hessian)
    {
      this->m_lagrangian_hessian = lagrangian_hessian;
    }

    /**
     * \brief Evaluate the objective function.
     * \param[in] x Primal variable vector.
     * \return The objective function.
     */
    Real objective(VectorN const & x) const override
    {
      return this->m_objective(x);
    }

    /**
     * \brief Evaluate the gradient of the objective function.
     * \param[in] x Primal variable vector.
     * \return The gradient of the objective function.
     */
    VectorN objective_gradient(VectorN const & x) const override
    {
      return this->m_objective_gradient(x);
    }

    /**
     * \brief Evaluate the Hessian of the objective function.
     * \param[in] x Primal variable vector.
     * \return The Hessian matrix of the objective function.
     */
    MatrixH objective_hessian(VectorN const & x) const override
    {
      return this->m_objective_hessian(x);
    }

    /**
     * \brief Evaluate the constraints function.
     * \param[in] x Primal variable vector.
     * \return The value of the constraints function.
     */
    VectorM constraints(VectorN const & x) const override
    {
      return this->m_constraints(x);
    }

    /**
     * \brief Evaluate the Jacobian of the constraints function.
     * \param[in] x Primal variable vector.
     * \param[in] z Dual variable vector.
     * \return The Jacobian matrix of the constraints function.
     */
    MatrixJ constraints_jacobian(VectorN const & x, VectorM const & z) const override
    {
      return this->m_constraints_jacobian(x, z);
    }

    /**
     * \brief Evaluate the Hessian of the Lagrangian function.
     * \param[in] x Primal variable vector.
     * \param[in] z Dual variable vector.
     * \return The Hessian matrix of the Lagrangian function.
     */
    MatrixH lagrangian_hessian(VectorN const & x, VectorM const & z) const override
    {
      return this->m_lagrangian_hessian(x, z);
    }

  }; // class ProblemWrapper

} // namespace Pipal

#endif /* INCLUDE_PIPAL_PROBLEM_HH */

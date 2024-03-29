#ifndef __STAN__PROB__DISTRIBUTIONS__MULTIVARIATE__CONTINUOUS__MULTI_NORMAL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__MULTIVARIATE__CONTINUOUS__MULTI_NORMAL_HPP__

#include <stan/math/matrix_error_handling.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/traits.hpp>
#include <stan/agrad/agrad.hpp>
#include <stan/meta/traits.hpp>
#include <stan/agrad/matrix.hpp>

namespace stan {
  namespace prob {
    /**
     * The log of the multivariate normal density for the given y, mu, and
     * a Cholesky factor L of the variance matrix.
     * Sigma = LL', a square, semi-positive definite matrix.
     *
     *
     * @param y A scalar vector
     * @param mu The mean vector of the multivariate normal distribution.
     * @param L The Cholesky decomposition of a variance matrix 
     * of the multivariate normal distribution
     * @return The log of the multivariate normal density.
     * @throw std::domain_error if LL' is not square, not symmetric, 
     * or not semi-positive definite.
     * @tparam T_y Type of scalar.
     * @tparam T_loc Type of location.
     * @tparam T_covar Type of scale.
     */
    template <bool propto,
              typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                  const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                  const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L,
                  const Policy&) {
      static const char* function = "stan::prob::multi_normal_cholesky_log(%1%)";

      using stan::math::mdivide_left_tri_low;
      using stan::math::dot_self;
      using stan::math::multiply;
      using stan::math::subtract;
      using stan::math::sum;
      
      using stan::math::check_size_match;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
      using stan::math::check_cov_matrix;
      using boost::math::tools::promote_args;

      typename promote_args<T_y,T_loc,T_covar>::type lp(0.0);

      if (!check_size_match(function, 
			    y.size(), "Size of random variable",
			    mu.size(), "size of location parameter",
			    &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
			    y.size(), "Size of random variable",
			    L.rows(), "rows of covariance parameter",
			    &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
			    y.size(), "Size of random variable",
			    L.cols(), "columns of covariance parameter",
			    &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;

      if (y.rows() == 0)
        return lp;
      
      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.rows();
      
      if (include_summand<propto,T_covar>::value) {
        Eigen::Matrix<T_covar,Eigen::Dynamic,1> L_log_diag = L.diagonal().array().log().matrix();
        lp -= sum(L_log_diag);
      }

      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<typename 
		      boost::math::tools::promote_args<T_y,T_loc>::type,
		      Eigen::Dynamic, 1> y_minus_mu(y.size());
        for (int i = 0; i < y.size(); i++)
          y_minus_mu(i) = y(i)-mu(i);
        Eigen::Matrix<typename 
		      boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
		      Eigen::Dynamic, 1> 
            half(mdivide_left_tri_low(L,y_minus_mu));
        // FIXME: this code does not compile. revert after fixing subtract()
        // Eigen::Matrix<typename 
        //               boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
        //               Eigen::Dynamic, 1> 
        //   half(mdivide_left_tri_low(L,subtract(y,mu)));
        lp -= 0.5 * dot_self(half);
      }
      return lp;
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
              const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
              const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L) {
      return multi_normal_cholesky_log<propto>(y,mu,L,
                                               stan::math::default_policy());
    }

    template <typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                  const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                  const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L,
                  const Policy&) {
      return multi_normal_cholesky_log<false>(y,mu,L,Policy());
    }

    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
              const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
              const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L) {
      return multi_normal_cholesky_log<false>(y,mu,L,
                                              stan::math::default_policy());
    }

    /** y can have multiple rows (observations) and columns (on variables)
     */
    template <bool propto,
              typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                  const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                  const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L,
                  const Policy&) {
      static const char* function = "stan::prob::multi_normal_cholesky_log(%1%)";

      using stan::math::mdivide_left_tri_low;
      using stan::math::columns_dot_self;
      using stan::math::multiply;
      using stan::math::subtract;
      using stan::math::sum;
      using stan::math::log;
      
      using stan::math::check_size_match;
      using stan::math::check_finite;
      using stan::math::check_not_nan;
      using stan::math::check_cov_matrix;
      using boost::math::tools::promote_args;

      typename promote_args<T_y,T_loc,T_covar>::type lp(0.0);

      if (!check_size_match(function, 
			    y.cols(), "Columns of random variable",
			    mu.rows(), "rows of location parameter",
			    &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
			    y.cols(), "Columns of random variable",
			    L.rows(), "rows of covariance parameter",
			    &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
			    y.cols(), "Columns of random variable",
			    L.cols(), "columns of covariance parameter",
			    &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;

      if (y.cols() == 0)
        return lp;

      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.cols() * y.rows();

      if (include_summand<propto,T_covar>::value) {
        Eigen::Matrix<T_covar,Eigen::Dynamic,1> L_log_diag = L.diagonal().array().log().matrix();
        lp -= sum(L_log_diag) * y.rows();
      }

      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic> MU(y.rows(),y.cols());
        for(typename Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic>::size_type i = 0; i < y.rows(); i++)
          MU.row(i) = mu;
	
        Eigen::Matrix<typename
                    boost::math::tools::promote_args<T_loc,T_y>::type,
                    Eigen::Dynamic,Eigen::Dynamic>
          y_minus_MU(y.rows(), y.cols());
        for (int i = 0; i < y.size(); i++)
          y_minus_MU(i) = y(i)-MU(i);

        Eigen::Matrix<typename 
                    boost::math::tools::promote_args<T_loc,T_y>::type,
                    Eigen::Dynamic,Eigen::Dynamic> 
          z(y_minus_MU.transpose()); // was = 
        
        // FIXME: revert this code when subtract() is fixed.
        // Eigen::Matrix<typename 
        //               boost::math::tools::promote_args<T_loc,T_y>::type,
        //               Eigen::Dynamic,Eigen::Dynamic> 
        //   z(subtract(y,MU).transpose()); // was = 
                
        Eigen::Matrix<typename 
                      boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
                      Eigen::Dynamic,Eigen::Dynamic> 
          half(mdivide_left_tri_low(L,z));
          
        lp -= 0.5 * sum(columns_dot_self(half));
      }
      return lp;
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
              const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
              const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L) {
      return multi_normal_cholesky_log<propto>(y,mu,L,
                                               stan::math::default_policy());
    }

    template <typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                  const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                  const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L,
                  const Policy&) {
      return multi_normal_cholesky_log<false>(y,mu,L,Policy());
    }

    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_cholesky_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
              const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
              const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& L) {
      return multi_normal_cholesky_log<false>(y,mu,L,
                                              stan::math::default_policy());
    }
    
    // MultiNormal(y|mu,Sigma)   [y.rows() = mu.rows() = Sigma.rows();
    //                            y.cols() = mu.cols() = 0;
    //                            Sigma symmetric, non-negative, definite]
    /**
     * The log of the multivariate normal density for the given y, mu, and
     * variance matrix. 
     * The variance matrix, Sigma, must be size d x d, symmetric,
     * and semi-positive definite. Dimension, d, is implicit.
     *
     * @param y A scalar vector
     * @param mu The mean vector of the multivariate normal distribution.
     * @param Sigma The variance matrix of the multivariate normal distribution
     * @return The log of the multivariate normal density.
     * @throw std::domain_error if Sigma is not square, not symmetric, 
     * or not semi-positive definite.
     * @tparam T_y Type of scalar.
     * @tparam T_loc Type of location.
     * @tparam T_covar Type of scale.
     */
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                     const Policy&) {
      static const char* function = "stan::prob::multi_normal_log(%1%)";
      typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type lp(0.0);
      
      using stan::math::check_not_nan;
      using stan::math::check_size_match;
      using stan::math::check_positive;
      using stan::math::check_pos_definite;
      using stan::math::check_finite;
      using stan::math::check_symmetric;
      using stan::math::dot_product;
      using stan::math::mdivide_left;
      using stan::math::log_determinant;
      
      if (!check_size_match(function, 
                            Sigma.rows(), "Rows of covariance parameter",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_positive(function, Sigma.rows(), "Covariance matrix rows", &lp, Policy()))
        return lp;
      if (!check_symmetric(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_pos_definite(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            mu.size(), "size of location parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            Sigma.rows(), "rows of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;
      
      if (y.rows() == 0)
        return lp;
      
      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.rows();
      
      if (include_summand<propto,T_covar>::value) {
        lp -= 0.5 * log_determinant(Sigma);
      }

      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_y,T_loc>::type,
            Eigen::Dynamic, 1> y_minus_mu(y.size());
        for (int i = 0; i < y.size(); i++)
          y_minus_mu(i) = y(i)-mu(i);
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
            Eigen::Dynamic, 1> Sinv_y_minus_mu(mdivide_left(Sigma,y_minus_mu));
        lp -= 0.5 * dot_product(y_minus_mu,Sinv_y_minus_mu);
      }
      return lp;
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
         const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
         const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_log<propto>(y,mu,Sigma,stan::math::default_policy());
    }


    template <typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
             const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
             const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
             const Policy&){
      return multi_normal_log<false>(y,mu,Sigma,Policy());
    }


    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
         const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
         const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_log<false>(y,mu,Sigma,stan::math::default_policy());
    }
    
    /**
     * y can have multiple rows (observations) and columns (on variables)
     */
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                     const Policy&) {
      static const char* function = "stan::prob::multi_normal_log(%1%)";
      typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type lp(0.0);
      
      using stan::math::check_size_match;
      using stan::math::check_positive;
      using stan::math::check_pos_definite;
      using stan::math::check_finite;
      using stan::math::check_symmetric;
      using stan::math::check_not_nan;
      using stan::math::sum;
      using stan::math::mdivide_left;
      using stan::math::log_determinant;
      using stan::math::columns_dot_product;
      
      if (!check_size_match(function, 
                            Sigma.rows(), "Rows of covariance matrix",
                            Sigma.cols(), "columns of covariance matrix",
                            &lp, Policy()))
        return lp;
      if (!check_positive(function, Sigma.rows(), "Covariance matrix rows", &lp, Policy()))
        return lp;
      if (!check_symmetric(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_pos_definite(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            mu.rows(), "rows of location parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            Sigma.rows(), "rows of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;
      
      if (y.cols() == 0)
        return lp;
      
      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.cols() * y.rows();
      
      if (include_summand<propto,T_covar>::value) {
        lp -= 0.5 * log_determinant(Sigma) * y.rows();
      }
      
      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic> MU(y.rows(),y.cols());
        for(typename Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic>::size_type i = 0; i < y.rows(); i++)
          MU.row(i) = mu;
        
        Eigen::Matrix<typename
            boost::math::tools::promote_args<T_loc,T_y>::type,
            Eigen::Dynamic,Eigen::Dynamic> y_minus_MU(y.rows(), y.cols());

        for (int i = 0; i < y.size(); i++)
          y_minus_MU(i) = y(i)-MU(i);
        
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_loc,T_y>::type,
            Eigen::Dynamic,Eigen::Dynamic> z(y_minus_MU.transpose()); // was = 
        
        // FIXME: revert this code when subtract() is fixed.
        // Eigen::Matrix<typename 
        //               boost::math::tools::promote_args<T_loc,T_y>::type,
        //               Eigen::Dynamic,Eigen::Dynamic> 
        //   z(subtract(y,MU).transpose()); // was = 
        
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
            Eigen::Dynamic,Eigen::Dynamic> Sinv_y_minus_mu(mdivide_left(Sigma,z));
        
        lp -= 0.5 * sum(columns_dot_product(z,Sinv_y_minus_mu));
      }
      return lp;      
    }

    template <bool propto,
              typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
         const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
         const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_log<propto>(y,mu,Sigma,stan::math::default_policy());
    }


    template <typename T_y, typename T_loc, typename T_covar, 
              class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
             const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
             const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
             const Policy&){
      return multi_normal_log<false>(y,mu,Sigma,Policy());
    }

    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
         const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
         const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_log<false>(y,mu,Sigma,stan::math::default_policy());
    }
    
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                          const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                          const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                          const Policy&) {
      static const char* function = "stan::prob::multi_normal_prec_log(%1%)";
      typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type lp(0.0);
      
      using stan::math::check_not_nan;
      using stan::math::check_symmetric;
      using stan::math::check_size_match;
      using stan::math::check_positive;
      using stan::math::check_pos_definite;
      using stan::math::check_finite;
      using stan::math::log_determinant;
      using stan::math::sum;
      using stan::math::dot_product;
      using stan::math::multiply;
      
      if (!check_size_match(function, 
                            Sigma.rows(), "Rows of covariance parameter",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_positive(function, Sigma.rows(), "Covariance matrix rows", &lp, Policy()))
        return lp;
      if (!check_symmetric(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_pos_definite(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            mu.size(), "size of location parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            Sigma.rows(), "rows of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.size(), "Size of random variable",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;
      
      if (y.rows() == 0)
        return lp;
      
      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.rows();
      
      if (include_summand<propto,T_covar>::value)
        lp += log_determinant(Sigma);
      
      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_y,T_loc>::type,
            Eigen::Dynamic, 1> y_minus_mu(y.size());
        for (int i = 0; i < y.size(); i++)
          y_minus_mu(i) = y(i)-mu(i);
        Eigen::Matrix<typename 
            boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
            Eigen::Dynamic, 1> Sinv_y_minus_mu(multiply(Sigma,y_minus_mu));
        lp -= 0.5 * dot_product(y_minus_mu,Sinv_y_minus_mu);
      }
      return lp;
    }
    
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_prec_log<propto>(y,mu,Sigma,stan::math::default_policy());
    }
    
    
    template <typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                     const Policy&){
      return multi_normal_prec_log<false>(y,mu,Sigma,Policy());
    }
    
    
    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,1>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_prec_log<false>(y,mu,Sigma,stan::math::default_policy());
    }

    /**
     * y can have multiple rows (observations) and columns (on variables)
     */
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                     const Policy&) {
      static const char* function = "stan::prob::multi_normal_prec_log(%1%)";
      typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type lp(0.0);
      
      using stan::math::check_not_nan;
      using stan::math::check_symmetric;
      using stan::math::check_size_match;
      using stan::math::check_positive;
      using stan::math::check_pos_definite;
      using stan::math::check_finite;
      using stan::math::log_determinant;
      using stan::math::sum;
      using stan::math::columns_dot_product;
      using stan::math::multiply;
      
      if (!check_size_match(function, 
                            Sigma.rows(), "Rows of covariance matrix",
                            Sigma.cols(), "columns of covariance matrix",
                            &lp, Policy()))
        return lp;
      if (!check_positive(function, Sigma.rows(), "Covariance matrix rows", &lp, Policy()))
        return lp;
      if (!check_symmetric(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_pos_definite(function, Sigma, "Covariance matrix", &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            mu.rows(), "rows of location parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            Sigma.rows(), "rows of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_size_match(function, 
                            y.cols(), "Columns of random variable",
                            Sigma.cols(), "columns of covariance parameter",
                            &lp, Policy()))
        return lp;
      if (!check_finite(function, mu, "Location parameter", &lp, Policy())) 
        return lp;
      if (!check_not_nan(function, y, "Random variable", &lp, Policy())) 
        return lp;
      
      if (y.cols() == 0)
        return lp;
      
      if (include_summand<propto>::value) 
        lp += NEG_LOG_SQRT_TWO_PI * y.cols() * y.rows();
      
      if (include_summand<propto,T_covar>::value) {
        lp += log_determinant(Sigma) * y.rows();
      }
      
      if (include_summand<propto,T_y,T_loc,T_covar>::value) {
        Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic> MU(y.rows(),y.cols());
        for(typename Eigen::Matrix<T_loc, Eigen::Dynamic, Eigen::Dynamic>::size_type i = 0; i < y.rows(); i++)
          MU.row(i) = mu;
        
        Eigen::Matrix<typename
        boost::math::tools::promote_args<T_loc,T_y>::type,
        Eigen::Dynamic,Eigen::Dynamic> y_minus_MU(y.rows(), y.cols());
        
        for (int i = 0; i < y.size(); i++)
          y_minus_MU(i) = y(i)-MU(i);
        
        Eigen::Matrix<typename 
        boost::math::tools::promote_args<T_loc,T_y>::type,
        Eigen::Dynamic,Eigen::Dynamic> z(y_minus_MU.transpose()); // was = 
        
        // FIXME: revert this code when subtract() is fixed.
        // Eigen::Matrix<typename 
        //               boost::math::tools::promote_args<T_loc,T_y>::type,
        //               Eigen::Dynamic,Eigen::Dynamic> 
        //   z(subtract(y,MU).transpose()); // was = 
        
        Eigen::Matrix<typename 
        boost::math::tools::promote_args<T_covar,T_loc,T_y>::type,
        Eigen::Dynamic,Eigen::Dynamic> Sinv_y_minus_mu(multiply(Sigma,z));
        
        lp -= 0.5 * sum(columns_dot_product(z,Sinv_y_minus_mu));
      }
      return lp;      
    }
    
    template <bool propto,
    typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_prec_log<propto>(y,mu,Sigma,stan::math::default_policy());
    }
    
    
    template <typename T_y, typename T_loc, typename T_covar, 
    class Policy>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma,
                     const Policy&){
      return multi_normal_prec_log<false>(y,mu,Sigma,Policy());
    }
    
    template <typename T_y, typename T_loc, typename T_covar>
    inline
    typename boost::math::tools::promote_args<T_y,T_loc,T_covar>::type
    multi_normal_prec_log(const Eigen::Matrix<T_y,Eigen::Dynamic,Eigen::Dynamic>& y,
                     const Eigen::Matrix<T_loc,Eigen::Dynamic,1>& mu,
                     const Eigen::Matrix<T_covar,Eigen::Dynamic,Eigen::Dynamic>& Sigma) {
      return multi_normal_prec_log<false>(y,mu,Sigma,stan::math::default_policy());
    }
    
  }
}

#endif

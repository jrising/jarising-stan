#ifndef __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__DISCRETE__NEG_BINOMIAL_HPP__
#define __STAN__PROB__DISTRIBUTIONS__UNIVARIATE__DISCRETE__NEG_BINOMIAL_HPP__

#include <boost/math/special_functions/digamma.hpp>
#include <stan/agrad.hpp>
#include <stan/math/error_handling.hpp>
#include <stan/math/special_functions.hpp>
#include <stan/meta/traits.hpp>
#include <stan/prob/traits.hpp>
#include <stan/prob/constants.hpp>
#include <stan/prob/internal_math.hpp>

namespace stan {

  namespace prob {

    // NegBinomial(n|alpha,beta)  [alpha > 0;  beta > 0;  n >= 0]
    template <bool propto,
              typename T_n,
              typename T_shape, typename T_inv_scale, 
              class Policy>
    typename return_type<T_shape, T_inv_scale>::type
    neg_binomial_log(const T_n& n, 
                     const T_shape& alpha, 
                     const T_inv_scale& beta, 
                     const Policy&) {

      static const char* function = "stan::prob::neg_binomial_log(%1%)";

      using stan::math::check_finite;      
      using stan::math::check_nonnegative;
      using stan::math::check_positive;
      using stan::math::value_of;
      using stan::math::check_consistent_sizes;
      using stan::prob::include_summand;
      
      // check if any vectors are zero length
      if (!(stan::length(n)
            && stan::length(alpha)
            && stan::length(beta)))
        return 0.0;
      
      double logp(0.0);
      if (!check_nonnegative(function, n, "Failures variable", &logp, Policy()))
        return logp;
      if (!check_finite(function, alpha, "Shape parameter", &logp, Policy()))
        return logp;
      if (!check_positive(function, alpha, "Shape parameter", &logp, Policy()))
        return logp;
      if (!check_finite(function, beta, "Inverse scale parameter",
                        &logp, Policy()))
        return logp;
      if (!check_positive(function, beta, "Inverse scale parameter", 
                          &logp, Policy()))
        return logp;
      if (!(check_consistent_sizes(function,
                                   n,alpha,beta,
                                   "Failures variable",
                                   "Shape parameter","Inverse scale parameter",
                                   &logp, Policy())))
        return logp;

      // check if no variables are involved and prop-to
      if (!include_summand<propto,T_shape,T_inv_scale>::value)
        return 0.0;

      using stan::math::multiply_log;
      using stan::math::binomial_coefficient_log;
      using boost::math::digamma;
      
      // set up template expressions wrapping scalars into vector views
      VectorView<const T_n> n_vec(n);
      VectorView<const T_shape> alpha_vec(alpha);
      VectorView<const T_inv_scale> beta_vec(beta);
      size_t size = max_size(n, alpha, beta);

      agrad::OperandsAndPartials<T_shape,T_inv_scale> operands_and_partials(alpha,beta);

      size_t len_ab = max_size(alpha,beta);
      DoubleVectorView<true,(is_vector<T_shape>::value || is_vector<T_inv_scale>::value)>
        lambda(len_ab);
      for (size_t i = 0; i < len_ab; ++i) 
        lambda[i] = value_of(alpha_vec[i]) / value_of(beta_vec[i]);

      DoubleVectorView<true,is_vector<T_inv_scale>::value>
        log1p_beta(length(beta));
      for (size_t i = 0; i < length(beta); ++i)
        log1p_beta[i] = log1p(value_of(beta_vec[i]));

      DoubleVectorView<true,is_vector<T_inv_scale>::value>
        log_beta_m_log1p_beta(length(beta));
      for (size_t i = 0; i < length(beta); ++i)
        log_beta_m_log1p_beta[i] = log(value_of(beta_vec[i])) - log1p_beta[i];

      DoubleVectorView<true,(is_vector<T_inv_scale>::value || is_vector<T_shape>::value)>
        alpha_times_log_beta_over_1p_beta(len_ab);
      for (size_t i = 0; i < len_ab; ++i)
        alpha_times_log_beta_over_1p_beta[i] 
          = value_of(alpha_vec[i])
          * log(value_of(beta_vec[i]) 
                / (1.0 + value_of(beta_vec[i])));

      DoubleVectorView<!is_constant_struct<T_shape>::value, is_vector<T_shape>::value>
        digamma_alpha(length(alpha));
      if (!is_constant_struct<T_shape>::value)
        for (size_t i = 0; i < length(alpha); ++i)
          digamma_alpha[i] = digamma(value_of(alpha_vec[i]));

      DoubleVectorView<!is_constant_struct<T_shape>::value, is_vector<T_inv_scale>::value>
        log_beta(length(beta));
      if (!is_constant_struct<T_shape>::value)
        for (size_t i = 0; i < length(beta); ++i)
          log_beta[i] = log(value_of(beta_vec[i]));

      DoubleVectorView<!is_constant_struct<T_inv_scale>::value, 
                       (is_vector<T_shape>::value || is_vector<T_inv_scale>::value)>
        lambda_m_alpha_over_1p_beta(len_ab);
      if (!is_constant_struct<T_inv_scale>::value)
        for (size_t i = 0; i < len_ab; ++i)
          lambda_m_alpha_over_1p_beta[i] =
            lambda[i]
            - ( value_of(alpha_vec[i]) 
                / (1.0 + value_of(beta_vec[i])) );

      for (size_t i = 0; i < size; i++) {
        if (alpha_vec[i] > 1e10) { // reduces numerically to Poisson
          if (include_summand<propto>::value)
             logp -= lgamma(n_vec[i] + 1.0);
          if (include_summand<propto,T_shape,T_inv_scale>::value)
            logp += multiply_log(n_vec[i], lambda[i]) - lambda[i];

          if (!is_constant_struct<T_shape>::value)
            operands_and_partials.d_x1[i]
              += n_vec[i] / value_of(alpha_vec[i]) 
              - 1.0 / value_of(beta_vec[i]);
          if (!is_constant_struct<T_inv_scale>::value)
            operands_and_partials.d_x2[i]
              += (lambda[i] - n_vec[i]) / value_of(beta_vec[i]) ;
        } else { // standard density definition
          if (include_summand<propto,T_shape>::value)
            if (n_vec[i] != 0)
              logp += binomial_coefficient_log<double>(n_vec[i] 
                                                       + value_of(alpha_vec[i]) - 1.0, 
                                                       n_vec[i]);
          if (include_summand<propto,T_shape,T_inv_scale>::value)
            logp += 
              alpha_times_log_beta_over_1p_beta[i] 
              - n_vec[i] * log1p_beta[i];

          if (!is_constant_struct<T_shape>::value)
            operands_and_partials.d_x1[i]
              += digamma(value_of(alpha_vec[i]) + n_vec[i])
              - digamma_alpha[i]
              + log_beta_m_log1p_beta[i];
          if (!is_constant_struct<T_inv_scale>::value)
            operands_and_partials.d_x2[i]
              += lambda_m_alpha_over_1p_beta[i]
              - n_vec[i]  / (value_of(beta_vec[i]) + 1.0);
        }
      }
      return operands_and_partials.to_var(logp);
    }

    template <bool propto,
              typename T_n,
              typename T_shape, typename T_inv_scale>
    inline
    typename return_type<T_shape, T_inv_scale>::type
    neg_binomial_log(const T_n& n, 
                     const T_shape& alpha, 
                     const T_inv_scale& beta) {
      return neg_binomial_log<propto>(n,alpha,beta,
                                      stan::math::default_policy());
    }

    template <typename T_n,
              typename T_shape, typename T_inv_scale, 
              class Policy>
    inline
    typename return_type<T_shape, T_inv_scale>::type
    neg_binomial_log(const T_n& n, 
                     const T_shape& alpha, 
                     const T_inv_scale& beta, 
                     const Policy&) {
      return neg_binomial_log<false>(n,alpha,beta,Policy());
    }

    template <typename T_n, 
              typename T_shape, typename T_inv_scale>
    inline
    typename return_type<T_shape, T_inv_scale>::type
    neg_binomial_log(const T_n& n, 
                     const T_shape& alpha, 
                     const T_inv_scale& beta) {
      return neg_binomial_log<false>(n,alpha,beta,
                                      stan::math::default_policy());
    }

      // Negative Binomial CDF
      template <bool propto, typename T_n, typename T_shape, typename T_inv_scale, 
                class Policy>
      typename return_type<T_shape, T_inv_scale>::type
      neg_binomial_cdf(const T_n& n, const T_shape& alpha, const T_inv_scale& beta, 
                       const Policy&) {
          
          static const char* function = "stan::prob::neg_binomial_cdf(%1%)";
          
          using stan::math::check_finite;      
          using stan::math::check_nonnegative;
          using stan::math::check_positive;
          using stan::math::check_consistent_sizes;
          using stan::prob::include_summand;
          
          // Ensure non-zero arugment lengths
          if (!(stan::length(n) && stan::length(alpha) && stan::length(beta)))
              return 0.0;
          
          double P(1.0);
          
          // Validate arguments
          if (!check_nonnegative(function, n, "Failures variable", &P, Policy()))
              return P;
          
          if (!check_finite(function, alpha, "Shape parameter", &P, Policy()))
              return P;
          
          if (!check_positive(function, alpha, "Shape parameter", &P, Policy()))
              return P;
          
          if (!check_finite(function, beta, "Inverse scale parameter",
                            &P, Policy()))
              return P;
          
          if (!check_positive(function, beta, "Inverse scale parameter", 
                              &P, Policy()))
              return P;
          
          if (!(check_consistent_sizes(function,
                                       n, alpha, beta,
                                       "Failures variable",
                                       "Shape parameter","Inverse scale parameter",
                                       &P, Policy())))
              return P;
          
          // Return if everything is constant and only proportionality is required
          if (!include_summand<propto,T_shape,T_inv_scale>::value)
              return 0.0;
          
          // Wrap arguments in vector views
          VectorView<const T_n> n_vec(n);
          VectorView<const T_shape> alpha_vec(alpha);
          VectorView<const T_inv_scale> beta_vec(beta);
          size_t size = max_size(n, alpha, beta);
          
          // Compute vectorized CDF and gradient
          using stan::math::value_of;
          using boost::math::ibetac;
          using boost::math::ibeta_derivative;
          
          using boost::math::digamma;
          
          // Cache a few expensive function calls if alpha is a parameter
          DoubleVectorView<!is_constant_struct<T_shape>::value,
                           is_vector<T_shape>::value> digammaN_vec(stan::length(alpha));
          DoubleVectorView<!is_constant_struct<T_shape>::value, 
                           is_vector<T_shape>::value> digammaAlpha_vec(stan::length(alpha));
          DoubleVectorView<!is_constant_struct<T_shape>::value, 
                           is_vector<T_shape>::value> digammaSum_vec(stan::length(alpha));
          DoubleVectorView<!is_constant_struct<T_shape>::value, 
                           is_vector<T_shape>::value> betaFunc_vec(stan::length(alpha));
          
          if (!is_constant_struct<T_shape>::value) {
              for (size_t i = 0; i < stan::length(alpha); i++) {
                  const double n_dbl = value_of(n_vec[i]);
                  const double alpha_dbl = value_of(alpha_vec[i]);
                  
                  digammaN_vec[i] = digamma(n_dbl);
                  digammaAlpha_vec[i] = digamma(alpha_dbl + 1);
                  digammaSum_vec[i] = digamma(n_dbl + alpha_dbl + 1);
                  betaFunc_vec[i] = boost::math::beta(n_dbl, alpha_dbl + 1);
              }
          }

          agrad::OperandsAndPartials<T_shape, T_inv_scale> 
            operands_and_partials(alpha, beta);
          
          std::fill(operands_and_partials.all_partials,
                    operands_and_partials.all_partials + operands_and_partials.nvaris,
                    0.0);
          
          for (size_t i = 0; i < size; i++) {
              const double n_dbl = value_of(n_vec[i]);
              const double alpha_dbl = value_of(alpha_vec[i]);
              const double beta_dbl = value_of(beta_vec[i]);
              
              const double p_dbl = beta_dbl / (1.0 + beta_dbl);
              const double d_dbl = 1.0 / ( (1.0 + beta_dbl) * (1.0 + beta_dbl) );
              
              const double Pi = ibetac(alpha_dbl + 1, n_dbl, p_dbl);
              
              P *= Pi;
              
              if (!is_constant_struct<T_shape>::value) {
                  double g1 = 0;
                  double g2 = 0;
                  
                  stan::math::gradRegIncBeta(g1, g2, alpha_dbl + 1, n_dbl, p_dbl, 
                                             digammaAlpha_vec[i], digammaN_vec[i], 
                                             digammaSum_vec[i], betaFunc_vec[i]);
                  
                  operands_and_partials.d_x1[n] 
                    += - g1 / Pi;
              }
              
              if (!is_constant_struct<T_inv_scale>::value)
                  operands_and_partials.d_x2[i] 
                    += - d_dbl * ibeta_derivative(alpha_dbl + 1, n_dbl, p_dbl) / Pi;
              
          }
          
          for (size_t i = 0; i < size; i++) {
              if (!is_constant_struct<T_shape>::value)
                  operands_and_partials.d_x1[i] *= P;
              if (!is_constant_struct<T_inv_scale>::value)
                  operands_and_partials.d_x2[i] *= P;
          }
          
          return P;
      }
      
      template <bool propto, typename T_n, typename T_shape, typename T_inv_scale>
      inline typename return_type<T_shape, T_inv_scale>::type
      neg_binomial_cdf(const T_n& n, const T_shape& alpha, const T_inv_scale& beta) {
          return neg_binomial_cdf<propto>(n, alpha, beta, stan::math::default_policy());
      }
      
      template <typename T_n, typename T_shape, typename T_inv_scale, class Policy>
      inline typename return_type<T_shape, T_inv_scale>::type
      neg_binomial_cdf(const T_n& n, const T_shape& alpha, const T_inv_scale& beta, 
                       const Policy&) {
          return neg_binomial_cdf<false>(n, alpha, beta, Policy());
      }
      
      template <typename T_n, typename T_shape, typename T_inv_scale>
      inline typename return_type<T_shape, T_inv_scale>::type
      neg_binomial_cdf(const T_n& n, const T_shape& alpha, const T_inv_scale& beta) {
          return neg_binomial_cdf<false>(n, alpha, beta, stan::math::default_policy());
      }


  }
}
#endif

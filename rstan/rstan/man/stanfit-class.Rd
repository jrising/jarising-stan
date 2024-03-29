
\name{stanfit-class} 
\Rdversion{1.1}

\docType{class}
\alias{stanfit-class}
\alias{summary,stanfit-method}
%% \alias{extract,stanfit-method}
\alias{show,stanfit-method}
% \alias{plot,stanfit-method}
\alias{get_cppo_mode} 
\alias{get_cppo_mode,stanfit-method} 
\alias{get_stancode}
\alias{get_stancode,stanfit-method}
\alias{get_stanmodel}
\alias{get_stanmodel,stanfit-method}
\alias{get_seed}
\alias{get_seed,stanfit-method}
\alias{get_seeds}
\alias{get_seeds,stanfit-method}
\alias{get_inits}
\alias{get_posterior_mean}
\alias{get_posterior_mean,stanfit-method}
\alias{get_inits,stanfit-method}
\alias{get_logposterior} 
\alias{get_logposterior,stanfit,logical-method}
\alias{get_adaptation_info} 
\alias{get_adaptation_info,stanfit-method}
\alias{get_sampler_params}
\alias{get_sampler_params,stanfit,logical-method}
%% \alias{traceplot,stanfit-method}

\title{Class \code{stanfit}: fitted Stan model}

\description{
  The output derived from fitting a Stan model, including the samples, 
  as returned 
  by the top-level function \code{stan} or the lower-level
  sampling method \code{sampling} defined on class \code{stanmodel}.
  \code{print} and \code{plot} and other methods are provided
  for the summaries of the fitted results.  
  Access methods allow the underlying data making up a fit
  to be retrieved. There are three modes; sampling mode, 
  test gradient mode, error mode (no samples included). 
} 

\section{Objects from the Class}{
  Objects should be created by either calling function \code{stan} or
  \code{sampling} method in S4 class \code{stan_model}.
}


\section{Slots}{
  \describe{
    \item{\code{model_name}:}{The model name, object of type \code{character}} 
    \item{\code{model_pars}:}{The names of parameters (or transformed
      parameters, derived quantities), object of type \code{character}} 
    \item{\code{par_dims}:}{The dimensions for all parameters, 
       object of type \code{list}}
    \item{\code{mode}:}{The mode of the fitted model, object of type 
       \code{integer}. \code{0} indicates sampling mode; \code{1}
       indicates test gradient mode for which no sampling is done;
       \code{2}; error occurred before sampling. Most of methods for 
       \code{stanfit} are useful only for \code{mode=1}}
    \item{\code{sim}:}{Simulation results including samples for the model,
       object of type \code{list}} 
    \item{\code{inits}:}{The initial values either specified or generated
      randomly for all chains, object of type \code{list} 
      containing named lists corresponding to initial values in the chains.}
    \item{\code{stan_args}:}{The arguments used for sampling all chains,
       object of type \code{list}}
    \item{\code{stanmodel}:}{The instance of S4 class \code{stanmodel}} 
    \item{\code{date}:}{The date the object is created} 
    \item{\code{.MISC}:}{Miscellaneous helper information used for the fitted model, 
      object of type \code{environment}}
  }
}

\section{Methods}{
  \describe{
    \item{\code{show}}{\code{signature(object = "stanfit")}: print the default summary
      for the model.}
    \item{\code{plot}}{\code{signature(x = "stanfit", y = "missing")}: plot an overview of summaries 
      for all parameters (see \code{\link{plot}}).
    }
    \item{\code{summary}}{\code{signature(object = "stanfit")}: summarizes the 
      distributions of quantities using the samples: the quantiles 
      (for example, 2.5\%, 50\%, 97.5\%, which can be specified by 
      using parameter \code{probs}), 
      the mean, the standard deviation (sd), the effective sample size (n_eff),
      and the split Rhat (i.e., potential scale reduction derived from all
      chains after splitting each chain in half and treating the halves as chains).
      Returned is a named list with elements such as \code{summary} and \code{c_summary}, 
      summaries for all chains merged without warmup and individual chains.
      For the summary of all chains merged, we also have \code{se_mean},
      the standard error of the mean. In addition to parameters, the log-posterior
      (\code{lp__}) is also a quantity of interest in the summary.  
      To specify parameters of interest, use parameter \code{pars}. The 
      default for \code{pars} is all the parameters saved in the fitted
      results as well as the log-posterior. 
      Another argument for \code{summary} is \code{use_cache}, which defaults
      to \code{TRUE}. When \code{use_cache=TRUE}, the summary quantities for
      all parameters are computed and cached for future use. 
      So \code{use_cache=FALSE} can be used to avoid the computing of all
      parameters if \code{pars} is given as some specific parameters. 
    }
    \item{\code{extract}}{\code{signature(object = "stanfit")}: 
      get the samples for all chains for all (or specified) parameters. 
      (see \code{\link{extract}}). Also see \link{as.array.stanfit} 
      for coercing samples without warmup to arrays or matrices. 
    }
    \item{\code{traceplot}}{\code{signature(object = "stanfit")}:
      plot the trace of chains (see \code{\link{traceplot}}). 
    } 
    \item{\code{get_posterior_mean}}{\code{signature(object = "stanfit")}:
      get the posterior mean for parameters of interest (using \code{pars}
      to specify) among \emph{all} parameters.} 
    \item{\code{get_stancode}}{\code{signature(object = "stanfit")}:
      get the Stan code for the fitted model.}
    \item{\code{get_cppo_mode}}{\code{signature(object = "stanfit")}:
      get the optimization mode used for compiling the model associated 
      with this fitted results. The returned string is 
      one of \code{"fast"}, \code{"presentation2"}, 
      \code{"presentation1"}, and \code{"debug"}.} 
    \item{\code{get_stanmodel}}{\code{signature(object = "stanfit")}: 
      get the object of S4 class \code{stanmodel} of the fitted 
      model.} 
    \item{\code{get_inits}}{\code{signature(object = "stanfit")}:
      get the initial values for parameters used in sampling all chains.
      For \code{mode=2}, it returns an empty list.} 
    \item{\code{get_seed}}{\code{signature(object = "stanfit")}: 
      get the seed that used for sampling. When the fitted object 
      is empty (\code{mode=2}), \code{NULL} might be returned.
      In the case the seeds for all chains are different, use 
      \code{get_seeds}.} 
    \item{\code{get_seeds}}{\code{signature(object = "stanfit")}: 
      get the seeds that used for all chains. When the fitted object 
      is empty (\code{mode=2}), \code{NULL} might be returned.} 
    \item{\code{get_logposterior}}{\code{signature(object = "stanfit")}: 
      get the log-posterior (up to an additive constant, which is up to
      a multiplicative constant on the linear scale) for all chains. 
      Each element of the returned \code{list} is the log-posterior 
      for a chain. Optional parameter \code{inc_warmup} indicates whether to include
      the warmup period.} 
    \item{\code{get_adaptation_info}}{\code{signature(object = "stanfit")}:
      obtain the adaptation information for 
      sampler, which now only NUTS2 has. The results are returned 
      as a list, each element of which is a character string
      for a chain.} 
    \item{\code{get_sampler_params}}{\code{signature(obtain = "stanfit")}: 
      obtain the parameters used for the sampler such as 
      \code{stepsize} and \code{treedepth}. The results are returned
      as a list, each element of which is an array for a chain.
      The array has number of columns corresponding to the number
      of parameters used in the sampler and its column names 
      provide the parameter names. Optional parameter \code{inc_warmup}
      indicates whether to include the warmup period.} 
  }
}

\references{
  The Stan Development Team (2012) 
  \emph{Stan Modeling Language User's Guide and Reference Manual}. 
  \url{http://mc-stan.org}. 
}
\seealso{
  \code{\link{stan}} and \code{\linkS4class{stanmodel}} 
}
\examples{\dontrun{
showClass("stanfit")
fit <- stan(model_code = 'parameters {real y[2];} model {y ~ normal(0,1);}')
fit2 <- stan(fit = fit)
print(fit)
plot(fit)
traceplot(fit)
ainfo <- get_adaptation_info(fit)
cat(ainfo[[1]])
seed <- get_seed(fit)
sp <- get_sampler_params(fit)
sp2 <- get_sampler_params(fit, inc_warmup = FALSE)
head(sp[[1]])
}}
\keyword{classes}

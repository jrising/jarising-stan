\name{stan}
\alias{stan}
\title{Fit a model using Stan}
\description{
  Fit a model defined in the Stan modeling language and  
  return the fitted result as an instance of \code{stanfit}. 
}
\usage{
stan(file, model_name = "anon_model", model_code = "", 
  fit = NA, data = list(), pars = NA, chains = 4,
  iter = 2000, warmup = floor(iter/2), thin = 1, 
  init = "random", seed = sample.int(.Machine$integer.max, 1), 
  sample_file, save_dso = TRUE, 
  verbose = FALSE, ..., 
  boost_lib = NULL,
  eigen_lib = NULL)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{file}{A character string file name or a connection that \R supports 
    containing the text of a model specification in the Stan modeling 
    language; a model may also be specified directly
    as a character string using parameter \code{model_code} or through
    a previous fit using parameter \code{fit}. 
    When \code{fit} is specified, parameter \code{file} is ignored.}
  \item{model_name}{A character string naming the model; defaults 
    to \code{"anon_model"}. However, the model name would be derived from 
    \code{file} or \code{model_code} (if \code{model_code} is the name
    of a character string object) if \code{model_name} is not specified.}
  \item{model_code}{A character string either containing the model definition
    or the name of a character string object in the workspace. This 
    parameter is used only if parameter \code{file} is not specified. 
    When \code{fit} is specified, the model compiled previously
    is used so specifying \code{model_code} is ignored.}
  \item{fit}{An instance of S4 class \code{stanfit} derived from
             a previous fit;  defaults to \code{NA}. 
    If \code{fit} is not \code{NA}, the compiled model associated with the fitted result 
    is re-used; thus the time that would otherwise be spent 
    recompiling the C++ code for the model can be saved.}
  \item{data}{An object of class \code{list}, \code{environment}
    providing the data for the model, or a vector of character strings 
    for all the names of objects used as data in the working space. 
    See the notes below.}    
  \item{pars}{A vector of character string specifying parameters of interest; defaults
    to \code{NA} indicating all parameters in the model. Only samples for parameters
    given in \code{pars} are stored in the fitted results.}
  \item{chains}{A positive integer specifying number of chains; defaults to 4.} 
  \item{iter}{A positive integer specifying how many iterations for each 
    chain (including warmup). The default is 2000.}
  \item{warmup}{A positive integer specifying number of warmup (aka burnin)
     iterations.  This also specifies the number of iterations used for step-size
     adaptation, so warmup samples should not be used for inference.
     The default is \code{iter/2}.}
  \item{thin}{A positive integer specifying the period for saving sample; defaults to 1.} 
  \item{seed}{The seed, a positive integer, for random number generation. The
    default is generated from 1 to the maximum integer supported by \R on the
    machine. When multiple chains are used, only one seed is needed, with other
    chains' seeds being generated from the first chain's seed to prevent
    dependency among the random number streams for the chains. When a seed is 
    specified by a number, \code{as.integer} will be applied to it. 
    If \code{as.integer} produces \code{NA}, the seed is generated randomly. 
    We can also specify a seed using a character string of digits, such as
    \code{"12345"}, which is converted to integer.}
  \item{init}{One of digit \code{0}, string \code{"0"} or \code{"random"}, 
    a function that returns a list, or a list of initial parameter values with
    which to indicate how the initial values of parameters are specified. 
    \code{"0"}: initialize all to be zero on the unconstrained support; 
    \code{"random"}: randomly generated;
    \code{list}: a list of lists equal in length to the number of chains 
     (parameter \code{chains}), where each list in the list of lists specifies the initial
      values of parameters by name for the corresponding chain.
    \code{function}: a function that returns a list for specifying 
      the initial values of parameters for a chain. The function
      can take an optional parameter \code{chain_id}. 
  }
  \item{sample_file}{A character string of file name for specifying where to 
    write samples for \emph{all} parameters and other saved quantities. 
    If not provided, files are not created. When the folder specified 
    is not writable, \code{tempdir()} is used. 
    When there are multiple chains, an underscore and chain number are appended
    to the file name.}

  \item{save_dso}{Logical, with default \code{TRUE}, indicating whether the 
    dynamic shared object (DSO) compiled from the C++ code for the model 
    will be saved or not. If \code{TRUE}, we can draw samples from
    the same model in another \R session using the saved DSO (i.e., 
    without compiling the C++ code again).
    This parameter only takes effect if \code{fit} is not used; with  
    \code{fit} defined, the DSO from the previous run is used.
    When \code{save_dso=TRUE}, the fitted object can be loaded from 
    what is saved previously and used for sampling, if the compiling is
    done on the same platform, that is, same operating system and same 
    architecture (32bits or 64bits). 
}

  \item{verbose}{\code{TRUE} or \code{FALSE}: flag indicating whether 
     to print intermediate output from Stan on the console, which might
     be helpful for model debugging.}

  \item{\dots}{Other optional parameters for the sampler: 
    \enumerate{
      \item \code{chain_id} (\code{integer})
      \item \code{leapfrog_steps} (\code{integer})
      \item \code{epsilon} (\code{double})
      \item \code{gamma} (\code{double}) 
      \item \code{delta} (\code{double})
      \item \code{equal_step_sizes} (\code{TRUE} or \code{FALSE})
      \item \code{max_treedepth} (\code{integer})
    }

    More details can be 
    found in Stan's manual. By default, 
    \code{leapfrog_steps} is \code{-1} and 
    \code{equal_step_sizes} is \code{FALSE} so that
    NUTS2 sampler, the default sampler, is used.

    \code{chain_id} can be a vector to specify the chain_id for all 
    chains or an integer. For the former case, they should be unique. 
    For the latter, the sequence of integers starting from the given
    \code{chain_id} are used for all chains. 

    In addition, \code{refresh} (\code{integer}) can be used to
    control how to indicate the progress during sampling (i.e. 
    show the progress every \code{refresh} iterations). 
    By default, \code{refresh = max(iter/10, 1)}. 
    The progress indicator is turned off if \code{refresh <= 0}.

    Another parameter is \code{test_grad} (\code{TRUE} or \code{FALSE}).  
    If \code{test_grad=TRUE}, Stan will not do any sampling. Instead,
    the gradient calculation is tested and printed out and the fitted
    \code{stanfit} object is in test gradient mode.  By default, it is
    \code{FALSE}. 
 
    When a new model is fitted starting from Stan model code, 
    \code{\dots} is passed to \code{stan_model} and thus \code{stanc}.
  } 

  \item{boost_lib}{The path for an alternative version of the Boost C++ 
     library to the one supplied with RStan.}

  \item{eigen_lib}{The path for an alternative version of the Eigen C++ 
     library to the one in \pkg{RcppEigen}.} 
}
\details{
  \code{stan} does all of the work of fitting a
  Stan model and returning the results as an instance of \code{stanfit}. 
  First, it translates the Stan model to C++ code.  Second, the 
  C++ code is compiled into a binary shared object, which
  is loaded into the current \R session (an object
  of S4 class \code{stanmodel} is created). Finally, sample
  are drawn and wrapped in an object of S4 class \code{stanfit}, 
  which provides functions such as \code{print}, \code{summary}, and \code{plot}
  to inspect and retrieve the results of the fitted model. 

  \code{stan} can also be used to sample again from a fitted model under
  different settings (e.g., different \code{iter}) by providing 
  argument \code{fit}. In this case, the compiled C++ code 
  for the model is reused. 
}
\value{
  Fitted results as an object of S4 class \code{stanfit}. If error occurs 
  before or during sampling, and if \code{test_grad = TRUE}, the returned
  object would not contain samples. But the compiled binary object for the 
  model is still included, so we can reuse the returned object for another
  sampling. 
}

\note{
  The data passed to \code{stan} are preprocessed before
  passing to Stan. In general, each element of data should be either a numeric
  vector (including special case 'scalar') or a numeric array (matrix). 
  An element can also be a data frame or a specially structured list (see
  details below), both of which will be converted into arrays in the
  preprocessing.  Using a specially structured list is not
  encouraged though it might be convenient sometimes; and when in doubt, just
  use arrays.  

  This preprocessing for each element mainly includes the following:
  \enumerate{
    \item Change the data of type from \code{double} 
      to \code{integer} if no accuracy is lost. The main
      reason is that by default, \R uses \code{double}
      as data type such as in \code{a <- 3}. But Stan 
      will not read data of type \code{int} from \code{real} 
      and it reads data from \code{int} if the data 
      type is declared as \code{real}. 
   
   \item Check if there is \code{NA} in the data. 
     Unlike BUGS, Stan does not allow missing data. Any \code{NA} values
     in supplied data will cause the function to stop and report an error.
  
   \item Check data types.  Stan allows only numeric data, that is,
     doubles, integers, and arrays of these.  Data of other types (for
     examples, characters) are not passed to Stan. 

   \item Check whether there are objects in the data list with duplicated names.
     Duplicated names, if found, will cause the function to stop and
     report an error. 

   \item Check whether the names of objects in the data list are legal 
     Stan names. If illegal names are found, it will stop and 
     report an error. See Stan's manual for the rules of variable names. 

   \item When an element is of type \code{data.frame}, it will be converted to
     \code{matrix} by function \code{data.matrix}. 

   \item When an element is of type \code{list}, it is supposed to make it
     easier to pass data for those declared in Stan code such as 
     \code{"vector[J] y1[I]"} and \code{"matrix[J,K] y2[I]"}. Using the latter
     as an example, we can use a list for \code{y2} if the list has "I" elements,
     each of which is an array (matrix) of dimension "J*K". However, it is 
     not possible to pass a list for data declared such as 
     \code{"vector[K] y3[I,J]"}; the only way for it is to use an array with
     dimension "I*J*K". In addition, technically a data frame in R is 
     also a list, but it should not be used for the purpose here since a data frame
     will be converted to a matrix as described above.  
  } 

  Stan treats a vector of length 1 in R as a scalar.  So technically 
  if, for example, \code{"real y[1];"} is defined in the data block, an array 
  such as \code{"y = array(1.0, dim = 1)"} in R should be used. 

  The returned S4 class \code{stanfit} includes the compiled model
  if option \code{save_dso} is \code{TRUE}, so the compiled model
  can be saved for using in future R sessions. 

  The function accepts a previously fitted instance of \code{stanfit}
  through parameter \code{fit} in order to reuse the compiled model; other
  configuration may change.

  The optimization level for compiling the C++ code generated for the 
  model can be set by \code{set_cppo}. In general, the higher the optimization
  level is set, the faster the generated binary code for the model runs. However, 
  the binary code generated for the model runs fast by using higher
  optimization level is at the cost of long time to compile the C++ code. 
}

\references{
  Stan Development Team (2012) 
  \emph{Stan Modeling Language User's Guide and Reference Manual}. 
  \url{http://mc-stan.org}. 
}
\seealso{
  \code{\link{stanc}} for translating model code in Stan modeling language to C++, 
  \code{\link{sampling}} for sampling, and \code{\linkS4class{stanfit}} for the
  fitted results.

  see \code{\link{extract}} and \code{\link{as.array.stanfit}} for extracting
  samples from \code{stanfit} objects.

  see \code{\link{set_cppo}} for setting higher optimization level for
  compiling the C++ code.
}
\examples{\dontrun{
library(rstan)
scode <- "
parameters {
  real y[2]; 
} 
model {
  y[1] ~ normal(0, 1);
  y[2] ~ double_exponential(0, 2);
} 
"
fit1 <- stan(model_code = scode, iter = 10, verbose = FALSE) 
print(fit1)
fit2 <- stan(fit = fit1, iter = 10000, verbose = FALSE) 

## extract samples as a list of arrays
e2 <- extract(fit2, permuted = TRUE)

## using as.array on the stanfit object to get samples 
a2 <- as.array(fit2)
}} 
\keyword{rstan} 

##
## Define rstan plugin for inline package.
## (original name: inline.R)
##

rstan_inc_path_fun <- function() { 
  system.file('include', package = 'rstan')
} 

rstan_libs_path_fun <- function() {
  if (nzchar(.Platform$r_arch)) {
    return(system.file('libstan', .Platform$r_arch, package = 'rstan'))
  }
  system.file('libstan', package = 'rstan')
}

# Using RcppEigen
eigen_path_fun <- function() {
  rstan_options("eigen_lib")
} 

# If included in RStan
# eigen_path_fun() <- paste0(rstan_inc_path_fun(), '/stanlib/eigen_3.1.0')

static_linking <- function() {
  # return(Rcpp:::staticLinking());
  ## not following Rcpp's link, we only have either dynamic version or static
  ## version because the libraries are big.
  ## (In Rcpp, both versions are compiled.)
  # return(.Platform$OS.type == 'windows')
  # For the time being, use static linking for libstan on all platforms. 
  TRUE
}

PKG_CPPFLAGS_env_fun <- function() {
   paste(' -I"', file.path(rstan_inc_path_fun(), '/stansrc" '),
         ' -I"', file.path(eigen_path_fun(), '" '),
         ' -I"', file.path(eigen_path_fun(), '/unsupported" '),
         ' -I"', rstan_options("boost_lib"), '"',
         ' -I"', rstan_inc_path_fun(), '"', sep = '')
}

legitimate_space_in_path <- function(path) {
  # Add preceding '\\' to spaces on non-windows (this should happen rarely,
  # and not sure it will work)
  # For windows, use the short path name (8.3 format) 
  # 
  WINDOWS <- .Platform$OS.type == "windows"
  if (WINDOWS) { 
    path2 <- utils::shortPathName(path) 
    # it is weird that the '\\' in the path name will be gone
    # when passed to cxxfunction, so chagne it to '/' 
    return(gsub('\\\\', '/', path2, perl=TRUE))
  }
  gsub("([^\\\\])(\\s+)", '\\1\\\\\\2', path, perl = TRUE)
} 

RSTAN_LIBS_fun <- function() {
  static <- static_linking() 
  rstan.libs.path <- rstan_libs_path_fun()

  # It seems that adding quotes to the path does not work well 
  # in the case there is space in the path name 
  if (grepl('[^\\\\]\\s', rstan.libs.path, perl = TRUE))
    rstan.libs.path <- legitimate_space_in_path(rstan.libs.path)

  if (static) {
    paste(' "', rstan.libs.path, '/libstan.a', '"', sep = '')
  } else {
    paste(' -L"', rstan.libs.path, '" -Wl,-rpath,"', rstan.libs.path, '" -lstan ', sep = '')
  }
}

# cat(PKG_CPPFLAGS_env, "\n")
# cat(RSTAN_LIBS_fun(), "\n")

rstanplugin <- function() {
  Rcpp_plugin <- getPlugin("Rcpp")
  rcpp_pkg_libs <- Rcpp_plugin$env$PKG_LIBS
  rcpp_pkg_path <- system.file(package = 'Rcpp')
  rcpp_pkg_path2 <- legitimate_space_in_path(rcpp_pkg_path) 
 
  # In case  we have space (typicall on windows though not necessarily)
  # in the file path of Rcpp's library. 
  
  # If rcpp_PKG_LIBS contains space without preceding '\\', add `\\'; 
  # otherwise keept it intact
  if (grepl('[^\\\\]\\s', rcpp_pkg_libs, perl = TRUE))
    rcpp_pkg_libs <- gsub(rcpp_pkg_path, rcpp_pkg_path2, rcpp_pkg_libs, fixed = TRUE) 

  list(includes = '',
       body = function(x) x,
       LinkingTo = c("Rcpp"),
       ## FIXME see if we can use LinkingTo for RcppEigen's header files
       env = list(PKG_LIBS = paste(rcpp_pkg_libs, RSTAN_LIBS_fun()),
                  PKG_CPPFLAGS = paste(Rcpp_plugin$env$PKG_CPPFLAGS, PKG_CPPFLAGS_env_fun())))
}


# inlineCxxPlugin would automatically get registered in inline's plugin list.
# Note that everytime rstan plugin is used, inlineCxxPlugin
# gets called so we can change some settings on the fly
# for example now by setting rstan_options(boost_lib=xxx)
inlineCxxPlugin <- function(...) {
  settings <- rstanplugin()
  settings
}


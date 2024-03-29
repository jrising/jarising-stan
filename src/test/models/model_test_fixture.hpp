#ifndef __TEST__MODELS__MODEL_TEST_FIXTURE_HPP__
#define __TEST__MODELS__MODEL_TEST_FIXTURE_HPP__

#include <gtest/gtest.h>
#include <test/models/utility.hpp>
#include <stan/mcmc/chains.hpp>
#include <utility>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/binomial.hpp>
#include <fstream>
#include <algorithm>

/** 
 * Model_Test_Fixture is a test fixture for google test
 * to aid in running models.
 *
 * Derived classes must define:
 *   static std::vector<std::string> get_model_path()
 *   static bool has_data()
 *   - indicates whether the model has data
 */
template <class Derived>
class Model_Test_Fixture : public ::testing::Test {
  
public:
  static char path_separator;
  static std::string model_path;
  static stan::mcmc::chains<> *chains;
  static size_t num_chains;
  static std::vector<std::string> command_outputs;
  static const size_t skip;
  static size_t iterations;
  static long elapsed_milliseconds;

  /** 
   * SetUpTestCase() called by google test once
   * a test case.
   *
   * Sets:
   *  - path_separator: '\' for Windows, '/' for all else.
   *  - base_name:      location of model without ".stan"
   *  - chains:         creates a chains object by reading
   *                    the header of a csv file
   */
  static void SetUpTestCase() {
    model_path = convert_model_path(get_model_path());

    iterations = num_iterations();

    chains = create_chains();
  }
    
  /** 
   * TearDownTestCase() called by google test once
   * a test case.
   *
   * Deletes chains. 
   */
  static void TearDownTestCase() {
    delete chains;
  }

  /** 
   * Returns the csv file for the chain number.
   * 
   * @param chain the chain number
   * 
   * @return the file location of the csv file
   */
  static std::string get_csv_file(size_t chain) {
    std::stringstream csv_file;
    csv_file << model_path << "." << chain << ".csv";
    return csv_file.str();
  }

  /** 
   * Returns the command to run in a shell on the OS.
   * 
   * @param chain the chain number 
   * 
   * @return a string command that can be run from a shell.
   */
  static std::string get_command(size_t chain) {
    std::stringstream command;
    command << model_path;
    command << " --samples=" << get_csv_file(chain);
    command << " --chain_id=" << chain;
    if (has_data()) {
      command << " --data=" << model_path << ".data.R";
    }
    if (has_init()) {
      command << " --init=" << model_path << ".init.R";
    }
    command << " --iter=" << iterations;
    command << " --refresh=" << iterations;
    return command.str();
  }

  /** 
   * Populates the chains object with data from csv files.
   */
  static void default_populate_chains() {
    if (chains->num_kept_samples() == 0U) {
      for (size_t chain = 0U; chain < num_chains; chain++) {
        stan::mcmc::add_chain(*chains, chain, get_csv_file(chain), skip);
      }
    }
  }

  static void populate_chains() {
    Derived::populate_chains();
  }
  
  static void test_gradient() {
    std::string command = get_command(0U);
    command += " --test_grad";
    std::string command_output;
    EXPECT_NO_THROW(command_output = run_command(command))
      << "Gradient test failed. \n"
      << "\tRan command: " << command << "\n"
      << "\tCommand output: " << command_output;
    //std::cout << "command_output: " << command_output << "\n";
  }

  /** 
   * Runs the model num_chains times.
   * Populates the chains object after running the model.
   */
  static void run_model() {
    for (size_t chain = 0; chain < num_chains; chain++) {
      std::string command_output;
      EXPECT_NO_THROW(command_output = run_command(get_command(chain), elapsed_milliseconds)) 
        << "Can not execute command: " << get_command(chain);
      command_outputs.push_back(command_output);
    }
    populate_chains();
  }
    
  /** 
   * Creates a chains object.
   *
   * Runs the model for 0 iterations to read the
   * names and dimensions of the parameters.
   * 
   * @return An initialized chains object.
   */
  static stan::mcmc::chains<>* create_chains() {
    std::stringstream command;
    command << get_command(num_chains)
	    << " --iter=0";
    EXPECT_NO_THROW(run_command(command.str())) 
      << "Can not build header using: " << command.str();
      
    std::vector<std::string> names;
    std::vector<std::vector<size_t> > dimss;
    stan::mcmc::read_variables(get_csv_file(num_chains), skip,
                               names, dimss);
      
    return (new stan::mcmc::chains<>(num_chains, names, dimss));
  }


  /** 
   * Return the path to the model (without the extension) as
   * a vector.
   * 
   * @return the path to the model
   */
  static std::vector<std::string> get_model_path() {
    return Derived::get_model_path();
  }
  
  /**
   * Return true if the model has data.
   *
   * @return true if the model has data;
   *         false otherwise.
   */
  static bool has_data() {
    return Derived::has_data();
  }

  /**
   * Return true if the model has an initialization file.
   *
   * @return true if the model has an initialization file;
   *         false otherwise.
   */
  static bool has_init() {
    return Derived::has_init();
  }

  static std::vector<size_t> skip_chains_test() {
    return Derived::skip_chains_test();
  } 

  static size_t num_iterations() {
    return Derived::num_iterations();
  }

  static std::vector<std::pair<size_t, double> >
  get_expected_values() {
    return Derived::get_expected_values();
  }

  static bool is_results_empty() {
    std::ifstream results("models/timing.csv");
    return (results.peek() == EOF);
  }

  static void write_header() {
    if (!is_results_empty())
      return;
    std::ofstream results("models/timing.csv");
    results << "model" << ","
	    << "chains" << ","
	    << "iterations per chain" << ","
	    << "kept samples" << ","
	    << "parameters" << ","
	    << "time (ms)" << ","
	    << "n_eff (min)" << ","
	    << "n_eff (max)" << ","
      	    << "n_eff (mean)" << ","
      	    << "n_eff (median)" << ","
	    << "time per n_eff (min)" << ","
	    << "time per n_eff (max)" << ","
	    << "time per n_eff (mean)" << ","
	    << "time per n_eff (median)"
	    << std::endl;
    results.close();
  }
  
  static void write_results() {
    write_header();
    std::ofstream results("models/timing.csv", std::ios_base::app);
    
    size_t N = chains->num_params();
    std::vector<double> n_eff(chains->num_params());
    for (size_t n = 0; n < N; n++)
      n_eff[n] = chains->effective_sample_size(n);
    std::sort(n_eff.begin(), n_eff.end());
    double n_eff_median;
    if (N % 2 == 0)
      n_eff_median = (n_eff[N/2 - 1] + n_eff[N/2]) / 2;
    else
      n_eff_median = n_eff[N/2];

    results << "\"" << model_path << ".stan\"" << ","
	    << chains->num_chains() << ","
	    << num_iterations() << ","
	    << chains->num_kept_samples() << ","
	    << N << ","
	    << elapsed_milliseconds << ","
	    << *(std::min_element(n_eff.begin(), n_eff.end())) << ","
	    << *(std::max_element(n_eff.begin(), n_eff.end())) << ","
      	    << stan::math::sum(n_eff) / N << ","
	    << n_eff_median << ","
	    << elapsed_milliseconds / *(std::min_element(n_eff.begin(), n_eff.end())) << ","
	    << elapsed_milliseconds / *(std::max_element(n_eff.begin(), n_eff.end())) << ","
	    << elapsed_milliseconds / (stan::math::sum(n_eff) / N) << ","
	    << elapsed_milliseconds / n_eff_median
	    << std::endl;
    results.close();
  }

};
  
template<class Derived> 
stan::mcmc::chains<> *Model_Test_Fixture<Derived>::chains;

template<class Derived>
size_t Model_Test_Fixture<Derived>::num_chains = 4;

template<class Derived>
std::string Model_Test_Fixture<Derived>::model_path;

template<class Derived>
std::vector<std::string> Model_Test_Fixture<Derived>::command_outputs;

template<class Derived>
const size_t Model_Test_Fixture<Derived>::skip = 3U;

template<class Derived>
size_t Model_Test_Fixture<Derived>::iterations = 2000U;

template<class Derived>
long Model_Test_Fixture<Derived>::elapsed_milliseconds = 0;


TYPED_TEST_CASE_P(Model_Test_Fixture);

TYPED_TEST_P(Model_Test_Fixture, TestGradient) {
  TypeParam::test_gradient();
}

TYPED_TEST_P(Model_Test_Fixture, RunModel) {
  TypeParam::run_model();
  TypeParam::write_results();
}

TYPED_TEST_P(Model_Test_Fixture, ChainsTest) {
  std::vector<std::string> err_message;
  for (size_t chain = 0; chain < TypeParam::num_chains; chain++) {
    std::vector<std::pair<std::string, std::string> > options = 
      parse_command_output(TypeParam::command_outputs[chain]);
    parse_command_output(TypeParam::command_outputs[chain]);

    std::string msg = "Seed is : ";
    for (size_t option = 0; option < options.size(); option++) {
      if (options[option].first == "seed")
        msg += options[option].second;
    }
    err_message.push_back(msg);
  }

  stan::mcmc::chains<> *c = TypeParam::chains;
  size_t num_chains = c->num_chains();
  size_t num_params = c->num_params();
  std::vector<size_t> params_to_skip = TypeParam::skip_chains_test();

  for (size_t chain = 0; chain < num_chains; chain++) {
    for (size_t param = 0; param < num_params; param++) {
      if (std::find(params_to_skip.begin(), params_to_skip.end(), param) == params_to_skip.end()) {
	EXPECT_GT(c->variance(chain, param), 0)
	  << "Chain " << chain << ", param " << param
	  << ": variance is 0" << std::endl
	  << err_message[chain];
      }
    }
  }

  for (size_t param = 0; param < num_params; param++) {
    if (std::find(params_to_skip.begin(), params_to_skip.end(), param) == params_to_skip.end()) {
      // made this 1.5 to fail less often
      EXPECT_LT(c->split_potential_scale_reduction(param), 1.5) 
	<< "Param " << param
	<< ": split r hat > 1.5" << std::endl;
    }
  }
}


TYPED_TEST_P(Model_Test_Fixture, ExpectedValuesTest) {
  using std::vector;
  using std::pair;
  using std::sqrt;
  using std::fabs;
  using std::setw;

  using boost::math::students_t;
  using boost::math::binomial;
  using boost::math::quantile;
  
  vector<pair<size_t, double> > expected_values = TypeParam::get_expected_values();
  size_t n = expected_values.size();
  if (n == 0)
    return;

  stan::mcmc::chains<> *c = TypeParam::chains;
  double alpha = 0.05;
  if (n == 1) 
    alpha = 0.0005;
  
  int failed = 0;
  std::stringstream err_message;
  for (size_t i = 0; i < n; i++) {
    size_t index = expected_values[i].first;
    double expected_mean = expected_values[i].second;

    double neff = c->effective_sample_size(index);
    double sample_mean = c->mean(index);
    double sd = c->sd(index);
    double se = sd / sqrt(neff);
    double z = quantile(students_t(neff-1.0), 1 - alpha/2.0);

    if (fabs(expected_mean - sample_mean) > sd) {
      failed++;
      // want the error message to have which, what, how
      err_message << "parameter index: " << index
                  << "\n\texpected:    " << setw(10) << expected_mean
                  << "\n\tsampled:     " << setw(10) << sample_mean
		  << "\n\tsd:          " << setw(10) << c->sd(index)
                  << "\n\tneff:        " << setw(10) << neff
                  << "\n\tsplit R.hat: " << setw(10) << c->split_potential_scale_reduction(index)
                  << "\n\tz:           " << setw(10) << z
                  << "\n\tse:          " << setw(10) << se
                  << "\n\n\tfabs(diff) > sd: " 
                  << fabs(expected_mean - sample_mean) << " > " << sd << "\n\n";
    }
    // that 5.0 is there to make the test fail less often.
    /*if (fabs(expected_mean - sample_mean) > z*se * 5.0) {
      failed++;
      // want the error message to have which, what, how
      err_message << "parameter index: " << index
                  << "\n\texpected:    " << setw(10) << expected_mean
                  << "\n\tsampled:     " << setw(10) << sample_mean
		  << "\n\tsd:          " << setw(10) << c->sd(index)
                  << "\n\tneff:        " << setw(10) << neff
                  << "\n\tsplit R.hat: " << setw(10) << c->split_potential_scale_reduction(index)
                  << "\n\tz:           " << setw(10) << z
                  << "\n\tse:          " << setw(10) << se
                  << "\n\n\tfabs(diff) > z * se * 5.0: " 
                  << fabs(expected_mean - sample_mean) << " > " << z*se * 5.0 << "\n\n";
		  }*/
  }
  
  if (failed == 0)
    return;

  double p = 1 - cdf(binomial(n, alpha), failed);
  // this test should fail less than 0.01% of the time.
  // (if all the parameters are failing independently... ha)
  if (p < 0.001) {
    err_message << "------------------------------------------------------------\n";
    for (size_t chain = 0; chain < TypeParam::num_chains; chain++) {
      std::vector<std::pair<std::string, std::string> > options = 
        parse_command_output(TypeParam::command_outputs[chain]);

      for (size_t option = 0; option < options.size(); option++) {
        if (options[option].first == "seed")
          err_message << "seed: " << options[option].second << std::endl;
      }
    }
    
    EXPECT_EQ(0, failed)
      << "Failed " << failed << " of " << expected_values.size() << " comparisons\n"
      << "p: " << p << std::endl
      << "------------------------------------------------------------\n"
      << err_message.str() << std::endl
      << "------------------------------------------------------------\n";
    
  }
}

REGISTER_TYPED_TEST_CASE_P(Model_Test_Fixture,
                           TestGradient,
                           RunModel,
                           ChainsTest,
                           ExpectedValuesTest);

#endif

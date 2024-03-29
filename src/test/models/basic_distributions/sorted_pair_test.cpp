#include <gtest/gtest.h>
#include <test/models/model_test_fixture.hpp>

class Models_BasicDistributions_SortedPair : 
  public Model_Test_Fixture<Models_BasicDistributions_SortedPair> {
protected:
  virtual void SetUp() {
  }
public:
  static std::vector<std::string> get_model_path() {
    std::vector<std::string> model_path;
    model_path.push_back("models");
    model_path.push_back("basic_distributions");
    model_path.push_back("sorted_pair");
    return model_path;
  }

  static bool has_data() {
    return false;
  }

  static bool has_init() {
    return false;
    
  }
  static size_t num_iterations() {
    return iterations;
  }

  static std::vector<size_t> skip_chains_test() {
    std::vector<size_t> params_to_skip;
    return params_to_skip;
  }

  static void populate_chains() {
    default_populate_chains();
  }

  static std::vector<std::pair<size_t, double> >
  get_expected_values() {
    std::vector<std::pair<size_t, double> > expected_values;
    return expected_values;
  }

};

INSTANTIATE_TYPED_TEST_CASE_P(Models_BasicDistributions_SortedPair,
			      Model_Test_Fixture,
			      Models_BasicDistributions_SortedPair);

TEST_F(Models_BasicDistributions_SortedPair,
       Test_Sorted_Pair) {
  populate_chains();
  using std::vector;
  vector<double> a, b;
  chains->get_samples(2U, a);
  chains->get_samples(3U, b);
  
  for (size_t n = 0; n < chains->num_samples(); n++) {
    EXPECT_GE(a[n], -1);
    EXPECT_LE(a[n], 1);
    EXPECT_GE(b[n], -1);
    EXPECT_LE(b[n], 1);
    EXPECT_GE(a[n], b[n])
      << n << ": expecting " << a[n] << " to be greater than or equal to " << b[n];
  }
}

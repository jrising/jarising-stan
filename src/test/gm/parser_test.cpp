#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <exception>
#include <stdexcept>

#include "stan/gm/ast.hpp"
#include "stan/gm/parser.hpp"
#include <stan/gm/generator.hpp>
#include <stan/gm/grammars/program_grammar.hpp>
#include <stan/gm/grammars/whitespace_grammar.hpp>
#include <stan/gm/grammars/expression_grammar.hpp>
#include <stan/gm/grammars/statement_grammar.hpp>
#include <stan/gm/grammars/var_decls_grammar.hpp>


bool is_parsable(const std::string& file_name) {
  stan::gm::program prog;
  std::ifstream fs(file_name.c_str());
  bool parsable = stan::gm::parse(0, fs, file_name, prog);
  return parsable;
}

TEST(gm_parser,eight_schools) {
  EXPECT_TRUE(is_parsable("src/models/misc/eight_schools/eight_schools.stan"));
}

TEST(gm_parser,bugs_1_kidney) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/kidney/kidney.stan"));
}
/*TEST(gm_parser,bugs_1_leuk) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/leuk/leuk.stan"));
  }*/
/*TEST(gm_parser,bugs_1_leukfr) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/leukfr/leukfr.stan"));
}*/
TEST(gm_parser,bugs_1_mice) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/mice/mice.stan"));
}
TEST(gm_parser,bugs_1_oxford) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/oxford/oxford.stan"));
}
TEST(gm_parser,bugs_1_rats) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/rats/rats.stan"));
}
TEST(gm_parser,bugs_1_salm) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/salm/salm.stan"));
}
TEST(gm_parser,bugs_1_seeds) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/seeds/seeds.stan"));
}
TEST(gm_parser,bugs_1_surgical) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol1/surgical/surgical.stan"));
}

TEST(gm_parser,bugs_2_beetles_cloglog) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/beetles/beetles_cloglog.stan"));
}
TEST(gm_parser,bugs_2_beetles_logit) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/beetles/beetles_logit.stan"));
}
TEST(gm_parser,bugs_2_beetles_probit) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/beetles/beetles_probit.stan"));
}
TEST(gm_parser,bugs_2_birats) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/birats/birats.stan"));
}
TEST(gm_parser,bugs_2_dugongs) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/dugongs/dugongs.stan"));
}
TEST(gm_parser,bugs_2_eyes) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/eyes/eyes.stan"));
}
TEST(gm_parser,bugs_2_ice) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/ice/ice.stan"));
}
/*TEST(gm_parser,bugs_2_stagnant) {
  EXPECT_TRUE(is_parsable("src/models/bugs_examples/vol2/stagnant/stagnant.stan"));
  }*/

TEST(gm_parser,good_trunc) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_trunc.stan"));
}

TEST(gm_parser,good_vec_constraints) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_trunc.stan"));
}

TEST(gm_parser,good_const) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_const.stan"));
}

TEST(gm_parser,good_matrix_ops) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_matrix_ops.stan"));
}

TEST(gm_parser,good_funs) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_funs.stan"));
}

TEST(gm_parser,triangle_lp) {
  EXPECT_TRUE(is_parsable("src/models/basic_distributions/triangle.stan"));
}

TEST(gm_parser,good_vars) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_vars.stan"));
}

TEST(gm_parser,good_intercept_var) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_intercept_var.stan"));
}


TEST(gm_parser,good_cov) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/good_cov.stan"));
}

TEST(gm_parser,parsable_test_bad1) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad1.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad2) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad2.stan"),
               std::invalid_argument);
}

TEST(gm_parser,parsable_test_bad3) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad3.stan"),
               std::invalid_argument);
}

TEST(gm_parser,parsable_test_bad4) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad4.stan"),
               std::invalid_argument);
}
// TEST(gm_parser,parsable_test_bad5) {
//    EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad5.stan"),
//                 std::invalid_argument);
// }

TEST(gm_parser,parsable_test_bad6) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad6.stan"),
               std::invalid_argument);
}

TEST(gm_parser,parsable_test_bad7) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad7.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad8) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad8.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad9) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad9.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad10) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad10.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad11) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad11.stan"),
               std::invalid_argument);
}
TEST(gm_parser,parsable_test_bad_trunc) {
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad_trunc1.stan"),
               std::invalid_argument);
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad_trunc2.stan"),
               std::invalid_argument);
  EXPECT_THROW(is_parsable("src/test/gm/model_specs/bad_trunc3.stan"),
               std::invalid_argument);
}
TEST(gm_parser,function_signatures) {
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures1.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures2.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures3.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures4.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures5.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures6.stan"));
  EXPECT_TRUE(is_parsable("src/test/gm/model_specs/compiled/function_signatures7.stan"));
}

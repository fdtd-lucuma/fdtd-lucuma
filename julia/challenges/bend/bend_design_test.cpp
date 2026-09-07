// == challenges/ceviche_designs_test/bend/bend_design_test.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "challenges/common/ceviche_design_test.hpp"

#ifndef LUCUMA_CHALLENGE_DIR
#define LUCUMA_CHALLENGE_DIR "."
#endif

int main(int argc, char **argv) {
  return lucuma::julia::RunCevicheDesignTest(LUCUMA_CHALLENGE_DIR, argc, argv);
}

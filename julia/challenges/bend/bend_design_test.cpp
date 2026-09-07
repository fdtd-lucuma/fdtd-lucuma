// == challenges/ceviche_designs_test/bend/bend_design_test.jl
// SPDX-License-Identifier: GPL-3.0-or-later

#include "challenges/common/ceviche_design_test.hpp"

#ifndef VULKAN_CHALLENGE_DIR
#define VULKAN_CHALLENGE_DIR "."
#endif

int main(int argc, char **argv) {
  return lucuma::vulkan::RunCevicheDesignTest(VULKAN_CHALLENGE_DIR, argc, argv);
}

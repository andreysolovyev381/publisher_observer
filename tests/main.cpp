//
// Created by Andrey Solovyev on 14/10/2021.
//

#include <gtest/gtest.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {

	testing::InitGoogleTest(&argc, argv);
	testing::GTEST_FLAG(color) = "yes";

	return RUN_ALL_TESTS();
}


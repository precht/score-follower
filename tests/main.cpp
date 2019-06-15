#include <gtest/gtest.h>

#include "unit_tests.h"
#include "acceptance_tests.h"
#include "performance_tests.h"
#include "integration_tests.h"

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

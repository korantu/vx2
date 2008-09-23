#include <gtest/gtest.h>

/*** Simple test: ***/
       TEST(TextCase, TestName) {};

/*** Fixture: ***/
//instance of each Fixture is created for every test
	class Fixture: public testing::Test {
	      virtual void SetUp() {/* Run before each test */ };
	      virtual void TearDown() { /* Run after each test */ };
        };

// Tests using fixtures:
TEST_F(Fixture, TestName){
};

/*** Checks: ***/


  

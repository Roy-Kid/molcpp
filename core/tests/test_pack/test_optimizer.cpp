#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "molcpp/pack/optimizer.hpp"
#include "molcpp/pack/optimizer_impl.hpp"
#include "molcpp/types.hpp"

using namespace molcpp::pack;
using namespace Catch;

TEST_CASE("Optimizer: Basic Structures", "[pack][optimizer]") {
    
    SECTION("OptimizationParams default values") {
        OptimizationParams<float> params;
        
        REQUIRE(params.max_iterations == 1000);
        REQUIRE(params.tolerance == Approx(1e-6));
        REQUIRE(params.learning_rate == Approx(0.01));
        REQUIRE(params.seed == 42);
        REQUIRE_FALSE(params.verbose);
        REQUIRE_FALSE(params.save_trajectory);
    }
    
    SECTION("OptimizationParams custom values") {
        OptimizationParams<float> params;
        params.max_iterations = 500;
        params.tolerance = 1e-8;
        params.learning_rate = 0.1;
        params.verbose = true;
        params.save_trajectory = true;
        
        REQUIRE(params.max_iterations == 500);
        REQUIRE(params.tolerance == Approx(1e-8));
        REQUIRE(params.learning_rate == Approx(0.1));
        REQUIRE(params.verbose);
        REQUIRE(params.save_trajectory);
    }
    
    SECTION("OptimizationResult structure") {
        OptimizationResult<float> result;
        
        // Test that we can set and access all fields
        result.positions = xt::zeros<float>({5, 3});
        result.final_penalty = 1.5;
        result.iterations = 100;
        result.converged = true;
        result.status = "Test status";
        result.trajectory = xt::ones<float>({5, 3});
        
        REQUIRE(result.positions.shape(0) == 5);
        REQUIRE(result.positions.shape(1) == 3);
        REQUIRE(result.final_penalty == Approx(1.5));
        REQUIRE(result.iterations == 100);
        REQUIRE(result.converged);
        REQUIRE(result.status == "Test status");
        REQUIRE(result.trajectory.has_value());
    }
}

TEST_CASE("Optimizer: GradientDescentOptimizer", "[pack][optimizer]") {
    
    SECTION("GradientDescentOptimizer creation and basic properties") {
        GradientDescentOptimizer<float> optimizer;
        
        REQUIRE(optimizer.name() == "GradientDescent");
        REQUIRE(optimizer.description() == "Basic gradient descent optimizer");
    }
    
    SECTION("GradientDescentOptimizer placeholder optimization") {
        GradientDescentOptimizer<float> optimizer;
        
        // Create simple initial positions
        CoordArrayf initial_positions = xt::ones<float>({3, 3});
        
        // Create simple penalty function (constant penalty)
        PenaltyFunction<float> penalty_fn = [](const CoordArrayf& positions) -> float {
            return 1.0;  // Constant penalty
        };
        
        // Create simple gradient function (zero gradient)
        GradientFunction<float> gradient_fn = [](const CoordArrayf& positions) -> CoordArrayf {
            return xt::zeros<float>(positions.shape());
        };
        
        OptimizationParams<float> params;
        params.verbose = false;  // Don't print during tests
        
        auto result = optimizer.optimize(initial_positions, penalty_fn, gradient_fn, params);
        
        // Check that result has expected structure (placeholder implementation)
        REQUIRE(result.positions.shape(0) == 3);
        REQUIRE(result.positions.shape(1) == 3);
        REQUIRE(result.final_penalty == Approx(1.0));
        REQUIRE(result.iterations == 0);  // Placeholder doesn't actually iterate
        REQUIRE_FALSE(result.converged);
        REQUIRE(result.status.find("Placeholder") != std::string::npos);
    }
    
    SECTION("GradientDescentOptimizer with trajectory saving") {
        GradientDescentOptimizer<float> optimizer;
        
        CoordArrayf initial_positions = xt::ones<float>({2, 3});
        
        PenaltyFunction<float> penalty_fn = [](const CoordArrayf& positions) -> float {
            return 0.5;
        };
        
        GradientFunction<float> gradient_fn = [](const CoordArrayf& positions) -> CoordArrayf {
            return xt::zeros<float>(positions.shape());
        };
        
        OptimizationParams<float> params;
        params.save_trajectory = true;
        params.verbose = false;
        
        auto result = optimizer.optimize(initial_positions, penalty_fn, gradient_fn, params);
        
        REQUIRE(result.trajectory.has_value());
        REQUIRE(result.trajectory->shape(0) == 2);
        REQUIRE(result.trajectory->shape(1) == 3);
    }
}

TEST_CASE("Optimizer: LBFGSOptimizer", "[pack][optimizer]") {
    
    SECTION("LBFGSOptimizer creation and basic properties") {
        LBFGSOptimizer<float> optimizer;
        
        REQUIRE(optimizer.name() == "L-BFGS");
        REQUIRE(optimizer.description() == "Limited-memory BFGS optimizer");
    }
    
    SECTION("LBFGSOptimizer placeholder optimization") {
        LBFGSOptimizer<float> optimizer;
        
        CoordArrayf initial_positions = xt::zeros<float>({4, 3});
        
        PenaltyFunction<float> penalty_fn = [](const CoordArrayf& positions) -> float {
            return 2.0;
        };
        
        GradientFunction<float> gradient_fn = [](const CoordArrayf& positions) -> CoordArrayf {
            return xt::ones<float>(positions.shape());
        };
        
        OptimizationParams<float> params;
        params.verbose = false;
        
        auto result = optimizer.optimize(initial_positions, penalty_fn, gradient_fn, params);
        
        REQUIRE(result.positions.shape(0) == 4);
        REQUIRE(result.positions.shape(1) == 3);
        REQUIRE(result.final_penalty == Approx(2.0));
        REQUIRE(result.iterations == 0);
        REQUIRE_FALSE(result.converged);
        REQUIRE(result.status.find("Placeholder") != std::string::npos);
    }
}

TEST_CASE("Optimizer: Factory Function", "[pack][optimizer]") {
    
    SECTION("make_optimizer creates GradientDescentOptimizer") {
        auto optimizer = make_optimizer<float>("gradient_descent");
        
        REQUIRE(optimizer != nullptr);
        REQUIRE(optimizer->name() == "GradientDescent");
        
        // Test short form
        auto optimizer_short = make_optimizer<float>("gd");
        REQUIRE(optimizer_short != nullptr);
        REQUIRE(optimizer_short->name() == "GradientDescent");
    }
    
    SECTION("make_optimizer creates LBFGSOptimizer") {
        auto optimizer = make_optimizer<float>("lbfgs");
        
        REQUIRE(optimizer != nullptr);
        REQUIRE(optimizer->name() == "L-BFGS");
        
        // Test alternative name
        auto optimizer_alt = make_optimizer<float>("L-BFGS");
        REQUIRE(optimizer_alt != nullptr);
        REQUIRE(optimizer_alt->name() == "L-BFGS");
    }
    
    SECTION("make_optimizer throws for unknown type") {
        REQUIRE_THROWS_AS(make_optimizer<float>("unknown_optimizer"), std::invalid_argument);
    }
}

TEST_CASE("Optimizer: Template Type Support", "[pack][optimizer]") {
    
    SECTION("Float precision optimizer") {
        GradientDescentOptimizer<float> optimizer;
        
        CoordArray<float> initial_positions = xt::ones<float>({2, 3});
        
        PenaltyFunction<float> penalty_fn = [](const CoordArray<float>& positions) -> float {
            return 1.5f;
        };
        
        GradientFunction<float> gradient_fn = [](const CoordArray<float>& positions) -> CoordArray<float> {
            return xt::zeros<float>(positions.shape());
        };
        
        OptimizationParams<float> params;
        params.verbose = false;
        
        auto result = optimizer.optimize(initial_positions, penalty_fn, gradient_fn, params);
        
        REQUIRE(result.final_penalty == Approx(1.5f));
        REQUIRE(result.positions.shape(0) == 2);
        REQUIRE(result.positions.shape(1) == 3);
    }
    
    SECTION("Double precision optimizer") {
        LBFGSOptimizer<double> optimizer;
        
        CoordArray<double> initial_positions = xt::ones<double>({3, 3});
        
        PenaltyFunction<double> penalty_fn = [](const CoordArray<double>& positions) -> double {
            return 2.5;
        };
        
        GradientFunction<double> gradient_fn = [](const CoordArray<double>& positions) -> CoordArray<double> {
            return xt::zeros<double>(positions.shape());
        };
        
        OptimizationParams<double> params;
        params.verbose = false;
        
        auto result = optimizer.optimize(initial_positions, penalty_fn, gradient_fn, params);
        
        REQUIRE(result.final_penalty == Approx(2.5));
        REQUIRE(result.positions.shape(0) == 3);
        REQUIRE(result.positions.shape(1) == 3);
    }
    
    SECTION("Factory with explicit types") {
        auto float_optimizer = make_optimizer<float>("gradient_descent");
        auto double_optimizer = make_optimizer<double>("lbfgs");
        
        REQUIRE(float_optimizer != nullptr);
        REQUIRE(double_optimizer != nullptr);
        REQUIRE(float_optimizer->name() == "GradientDescent");
        REQUIRE(double_optimizer->name() == "L-BFGS");
    }
}


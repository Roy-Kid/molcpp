#include "molcpp/pack/optimizer.hpp"
#include "molcpp/pack/optimizer_impl.hpp"

// Explicit instantiation for common types
template class molcpp::pack::GradientDescentOptimizer<float>;
template class molcpp::pack::GradientDescentOptimizer<double>;

template class molcpp::pack::LBFGSOptimizer<float>;
template class molcpp::pack::LBFGSOptimizer<double>;

template std::unique_ptr<molcpp::pack::Optimizer<float>> molcpp::pack::make_optimizer<float>(const std::string& type);
template std::unique_ptr<molcpp::pack::Optimizer<double>> molcpp::pack::make_optimizer<double>(const std::string& type);


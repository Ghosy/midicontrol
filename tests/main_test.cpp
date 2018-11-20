#define CATCH_CONFIG_RUNNER
#include<catch2/catch.hpp>
#include<spdlog/sinks/stdout_color_sinks.h>

int main(int argc, char* argv[]) {
	// Lie that there is a logger
	auto null_sink = spdlog::stdout_color_mt("multi_sink");
	null_sink->set_level(spdlog::level::off);

	int result = Catch::Session().run(argc, argv);

	return result;
}

#include <log.hpp>
#include <manager.hpp>
#include <options.hpp>
#include <processor.hpp>

constexpr static auto CONFIG_NAME = "flower.conf";

static void
load_config() {
  namespace fs = std::filesystem;

  const auto *home = std::getenv("HOME");
  auto home_config = std::string{home} + "/." + CONFIG_NAME;
  if (fs::is_regular_file(home_config)) {
    Options::load_file(home_config);
  }

  if (fs::is_regular_file(CONFIG_NAME)) {
    Options::load_file(CONFIG_NAME);
  }
}

static void
init_plugin_manager(Plugins::Manager& manager, const std::string& path) {
  namespace fs = std::filesystem;

  if (fs::is_directory(Options::SYSTEM_PLUGINS_DIR)) {
    Log::info("Loading plugins from %s\n", Options::SYSTEM_PLUGINS_DIR);
    manager.load_from_folder(Options::SYSTEM_PLUGINS_DIR);
  }

  if (fs::is_directory(path)) {
    Log::info("Loading plugins from %s\n", path.c_str());
    manager.load_from_folder(path);
  }
}

int
main(int argc, char **argv) {
  load_config();
  Options::parse_args(argc, argv);

  auto plugin_manager = Plugins::Manager{};
  init_plugin_manager(plugin_manager, Options::plugins_dir);

  switch (Options::mode) {
  /* List plugins */
  case Options::Mode::PRINT_PLUGINS:
    std::printf("Input plugins:\n");
    for (const auto &[_, p] : plugin_manager.inputs()) {
      std::printf("----------------\n");
      std::printf("%s\n", p.info().name);
      std::printf("----------------\n");
      std::printf("%s\n", p.info().description);
      std::printf("\n");
    }
    break;

  /* Print help */
  case Options::Mode::PRINT_HELP:
    Options::print_help(argv[0]);
    break;

  /* Print version */
  case Options::Mode::PRINT_VERSION:
    std::printf("version 1.0.0\n");
    break;

  /* Main process */
  case Options::Mode::PROCESS:
    try {
      auto input = plugin_manager.create_input(Options::input_plugin,
                                               Options::argument.c_str());
      Flow::start_processor(std::move(input));
    } catch (const std::exception &e) {
      Log::error("Error: %s\n", e.what());
      return 2;
    }
    break;
  }

  return 0;
}

#include <filesystem>

#include <log.hpp>
#include <manager.hpp>
#include <options.hpp>
#include <processor.hpp>

int
main(int argc, char **argv) {
  const auto *home = std::getenv("HOME");
  auto home_config = std::string{home} + "/." + Options::CONFIG_NAME;

  /* Load config */
  Options::merge_file(home_config);
  Options::merge_file(Options::CONFIG_NAME);

  /* Parse arguments */
  Options::merge_args(argc, argv);

  const auto& options = Options::options();

  /* Load plugins */
  Plugins::load_plugins(Options::SYSTEM_PLUGINS_DIR);
  Plugins::load_plugins(options.plugins_dir);

  switch (options.mode) {
  /* List plugins */
  case Options::Mode::PRINT_PLUGINS:
    Plugins::print_plugins();
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
      Flow::Processor().start();
    } catch (const std::exception &e) {
      Log::error("Error: %s\n", e.what());
      return 2;
    }
    break;
  }

  return 0;
}

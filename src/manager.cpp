#include <manager.hpp>

#include <cstdio>
#include <filesystem>
#include <unordered_map>

#include <log.hpp>
#include <plugin.hpp>

namespace Plugins {

static std::unordered_map<std::string, Plugin> inputs;

void
load_plugins(const std::string& dir)
{
  Log::debug("Loading plugins from %s\n", dir.c_str());

  using namespace std::filesystem;

  if (!is_directory(dir))
    return;

  for (const auto& file : directory_iterator(dir)) {
    if (file.path().extension() != PLUGIN_EXTENSION)
      continue;

    auto plugin = Plugin{file.path()};

    if (plugin.info().type != INPUT_PLUGIN)
      continue;

    inputs.emplace(plugin.info().name, std::move(plugin));
  }
}

Input
create_input(const std::string& name, const char* arg)
{
  Log::debug("Creating plugin %s\n", name.c_str());

  auto search = inputs.find(name);
  if (search == inputs.end())
    throw std::runtime_error{"Input is not loaded"};
  
  auto input = Input{std::move(search->second), arg};
  inputs.erase(search);

  return input;
}

void
print_plugins()
{
  for (const auto& [_, plugin] : inputs) {
    std::printf("----------------\n");
    std::printf("%s\n", plugin.info().name);
    std::printf("----------------\n");
    std::printf("%s\n", plugin.info().description);
    std::printf("\n");
  }
}

} // namespace Plugins

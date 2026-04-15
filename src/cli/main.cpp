#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

#include "cli/action.h"
#include "cli/control/register.h"
#include "cli/config/register.h"
#include "cli/utils/cli_context.h"
#include "cli/utils/formatter.h"
#include "common/i18n.h"
#include "config.h"

int main(int argc, char *argv[]) {
  vinput::i18n::Init();

  CLI::App app{_("vinput - Voice input manager")};
  auto cli_formatter = std::make_shared<CLI::Formatter>();
  cli_formatter->label("Usage", _("Usage"));
  cli_formatter->label("OPTIONS", _("OPTIONS"));
  cli_formatter->label("SUBCOMMAND", _("SUBCOMMAND"));
  cli_formatter->label("SUBCOMMANDS", _("SUBCOMMANDS"));
  cli_formatter->label("Positionals", _("Positionals"));
  cli_formatter->label("REQUIRED", _("REQUIRED"));
  cli_formatter->label("Env", _("Env"));
  cli_formatter->label("Needs", _("Needs"));
  cli_formatter->label("Excludes", _("Excludes"));
  app.formatter(cli_formatter);
  app.require_subcommand(0, 1);
  app.set_help_flag("-h,--help", _("Print this help message and exit"))
      ->group(_("OPTIONS"));
  app.set_version_flag("-v,--version", VINPUT_VERSION,
                       _("Display program version information and exit"))
      ->group(_("OPTIONS"));

  bool json_output = false;
  app.add_flag("-j,--json", json_output, _("Output in JSON format"))
      ->group(_("OPTIONS"));

  vinput::cli::CliAction action;
  vinput::cli::config::RegisterConfigCli(app, &action);
  vinput::cli::control::RegisterControlCli(app, &action);

  const auto localize_help_groups = [](auto &&self, CLI::App *node) -> void {
    for (auto *option : node->get_options()) {
      if (option->get_group() == "Options") {
        option->group(_("OPTIONS"));
      }
    }
    for (auto *subcommand : node->get_subcommands([](CLI::App *) { return true; })) {
      if (subcommand->get_group() == "Subcommands") {
        subcommand->group(_("SUBCOMMANDS"));
      }
      self(self, subcommand);
    }
  };
  localize_help_groups(localize_help_groups, &app);

  CLI11_PARSE(app, argc, argv);

  if (argc == 1) {
    std::cout << app.help();
    return 0;
  }

  CliContext ctx;
  ctx.json_output = json_output;
  ctx.is_tty = (isatty(STDOUT_FILENO) == 1);

  auto fmt = CreateFormatter(ctx);
  if (action) {
    return action(*fmt, ctx);
  }
  return 0;
}

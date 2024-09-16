<?php
/**
 * @file
 * Command(s) for displaying help information.
 */

/**
 * Implements hook_bee_command().
 */
function help_bee_command() {
  return array(
    'help' => array(
      'description' => bt("Provide help and examples for 'bee' and its commands."),
      'callback' => 'help_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => FALSE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'information',
      'arguments' => array(
        'command' => bt("The command to display help for. Leave blank to display help for 'bee' itself."),
      ),
      'optional_arguments' => array('command'),
      'examples' => array(
        'bee help status' => bt("Display help for the 'status' command."),
        'bee help' => bt("Display help for 'bee' itself."),
      ),
    ),
  );
}

/**
 * Command callback: Provide help and examples for 'bee' and its commands.
 */
function help_bee_callback($arguments, $options) {
  global $_bee_global_options, $_bee_command_aliases;

  // Display help for the given command.
  if (isset($arguments['command'])) {
    $command = $arguments['command'];

    // Get all commands and sort them alphabetically.
    $commands = bee_all_commands();
    ksort($commands);

    // If the given command is an alias, get the actual command name.
    if (isset($_bee_command_aliases[$command])) {
      $command = $_bee_command_aliases[$command];
    }

    // Make sure the command exists.
    if (!isset($commands[$command])) {
      bee_message(bt("Cannot display help for '!command' as it doesn't exist.", array(
        '!command' => $command,
      )), 'error');
      return;
    }

    // Add the name of the command so it can be retrieved later.
    $commands[$command]['command'] = $command;

    return help_bee_command_help($commands[$command]);
  }

  // Otherwise display help for 'bee' itself.
  $option_rows = array();
  $command_rows = array();

  // Display Bee with the symbol.
  $output = array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => BEE_SYMBOL_BEE . " " . bt('Bee'),
        '#bold' => TRUE,
      ),
    ),
  );

  // Display the usage pattern.
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => bt('Usage: bee [global-options] <command> [options] [arguments]') . "\n",
    ),
  );

  // Display the global options.
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => bt('Global Options:'),
      '#bold' => TRUE,
    ),
  );

  foreach ($_bee_global_options as $option => $descriptor) {
    $value = '--' . $option;
    if (isset($descriptor['short'])) {
      $value .= ', -' . $descriptor['short'];
    }
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => " $value",
        '#color' => 'blue',
        '#bold' => TRUE,
      ),
    );
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => " " . $descriptor['description'] . "\n",
      ),
    );
  }

  // Add the Commands.
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => "\n" . bt('Commands:'),
      '#bold' => TRUE,
    ),
  );

  // Get all commands in groups.
  $commands = bee_all_commands(TRUE);
  ksort($commands);

  // Move the 'Advanced' group to the end.
  $advanced = $commands['advanced'];
  unset($commands['advanced']);
  $commands['advanced'] = $advanced;

  foreach ($commands as $group => $grouped_commands) {
    ksort($grouped_commands);

    // Output the Group heading.
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => " " . strtoupper(str_replace('_', ' ', $group)),
        '#bold' => TRUE,
      ),
    );

    // Loop through each command in the group.
    foreach ($grouped_commands as $command => $descriptor) {
      // Output the command.
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => "  " . $command,
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
      );

      // Output aliases if they exist.
      $aliases = array_keys($_bee_command_aliases, $command);
      if (!empty($aliases)) {
        $output[] = array(
          'type' => 'text',
          'variables' => array(
            'value' => '   ' . implode(', ', $aliases),
            '#color' => 'magenta',
            '#bold' => TRUE,
          ),
        );
      }

      // Output the command description.
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => "   " . $descriptor['description'] . "\n",
        ),
      );
    }
  }
  return $output;
}

/**
 * Parse a command descriptor to provide help information about the command.
 *
 * @param array $descriptor
 *   A command descriptor. See the documentation for the return value of
 *   HOOK_bee_command() for more details.
 */
function help_bee_command_help(array $descriptor) {
  global $_bee_command_aliases;
  $aliases = array_keys($_bee_command_aliases, $descriptor['command']);

  // Command name.
  $output = array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => $descriptor['command'],
        '#bold' => TRUE,
      ),
    ),
  );
  if (!empty($aliases)) {
    // Aliases.
    $output[0]['newline'] = FALSE;
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => ', ' . implode(', ', $aliases),
      ),
    );
  }

  // Description.
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => $descriptor['description'] . "\n",
    ),
  );

  // Arguments.
  if (!empty($descriptor['arguments'])) {
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Arguments:'),
        '#bold' => TRUE,
      ),
    );

    foreach ($descriptor['arguments'] as $argument_name => $argument_description) {
      if (!empty($descriptor['optional_arguments']) && in_array($argument_name, $descriptor['optional_arguments'])) {
        $argument_description = bt('(optional) ') . $argument_description;
      }

      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " $argument_name",
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
      );
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " $argument_description\n",
        ),
      );
    }
  }

  // Options.
  if (!empty($descriptor['options'])) {
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Options:'),
        '#bold' => TRUE,
      ),
    );

    foreach ($descriptor['options'] as $option_name => $option) {
      $value = !empty($option['value']) ? '=' . strtoupper($option['value']) : '';
      if (isset($option['short'])) {
        $option_name .= ', -' . $option['short'];
      }
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " --$option_name" . $value,
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
      );
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " " . $option['description'] . "\n",
        ),
      );
    }
  }

  // Examples.
  if (!empty($descriptor['examples'])) {
    $output[] = array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Examples:'),
        '#bold' => TRUE,
      ),
    );

    foreach ($descriptor['examples'] as $example => $description) {
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " $example",
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
      );
      $output[] = array(
        'type' => 'text',
        'variables' => array(
          'value' => " $description\n",
        ),
      );
    }
  }

  return $output;
}

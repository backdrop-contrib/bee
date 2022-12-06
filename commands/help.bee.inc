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

  // Get all commands and sort them alphabetically.
  $commands = bee_all_commands();
  ksort($commands);

  // Display help for the given command.
  if (isset($arguments['command'])) {
    $command = $arguments['command'];

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

  foreach ($_bee_global_options as $option => $descriptor) {
    $value = '--' . $option;
    if (isset($descriptor['short'])) {
      $value .= ', -' . $descriptor['short'];
    }
    $option_rows[] = array(
      array(
        'value' => $value,
        '#color' => 'blue',
        '#bold' => TRUE,
      ),
      array('value' => $descriptor['description']),
    );

    $option_rows[] = array();
  }

  foreach ($commands as $command => $descriptor) {
    $command_rows[] = array(
      array(
        'value' => $command,
        '#color' => 'blue',
        '#bold' => TRUE,
      ),
      array('value' => $descriptor['description']),
    );

    $aliases = array_keys($_bee_command_aliases, $command);
    if (!empty($aliases)) {
      $command_rows[] = array(
        array(
          'value' => implode(', ', $aliases),
          '#color' => 'magenta',
          '#bold' => TRUE,
        ),
        array('value' => ''),
      );
    }

    $command_rows[] = array();
  }

  return array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Bee'),
        '#bold' => TRUE,
      ),
    ),
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Usage: bee [global-options] <command> [options] [arguments]') . "\n",
      ),
    ),
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Global Options:'),
        '#bold' => TRUE,
      ),
    ),
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $option_rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    ),
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Commands:'),
        '#bold' => TRUE,
      ),
    ),
    array(
      'type' => 'table',
      'variables' => array(
        'rows' => $command_rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    ),
  );
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
      'newline' => TRUE,
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

    $rows = array();
    foreach ($descriptor['arguments'] as $argument_name => $argument_description) {
      if (!empty($descriptor['optional_arguments']) && in_array($argument_name, $descriptor['optional_arguments'])) {
        $argument_description = bt('(optional) ') . $argument_description;
      }

      $rows[] = array(
        array(
          'value' => $argument_name,
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
        array('value' => $argument_description),
      );
    }

    $output[] = array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    );
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

    $rows = array();
    foreach ($descriptor['options'] as $option_name => $option) {
      $value = !empty($option['value']) ? '=' . strtoupper($option['value']) : '';

      $rows[] = array(
        array(
          'value' => '--' . $option_name . $value,
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
        array('value' => $option['description']),
      );
    }

    $output[] = array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    );
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

    $rows = array();
    foreach ($descriptor['examples'] as $example => $description) {
      $rows[] = array(
        array(
          'value' => $example,
          '#color' => 'blue',
          '#bold' => TRUE,
        ),
        array('value' => $description),
      );
    }

    $output[] = array(
      'type' => 'table',
      'variables' => array(
        'rows' => $rows,
        'delimiter' => '   ',
        'delimiter_left' => ' ',
        'delimiter_right' => ' ',
      ),
    );
  }

  return $output;
}

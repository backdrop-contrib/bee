<?php
/**
 * @file
 * Command(s) for displaying help information.
 */

/**
 * Implements hook_b_command().
 */
function help_b_command() {
  return array(
    'help' => array(
      'description' => bt('Provide help and examples for `b` and its commands.'),
      'callback' => 'help_b_callback',
      'arguments' => array(
        'command' => bt ('The command to display help for. Leave blank to display help for `b` itself.'),
      ),
      'optional_arguments' => array('command'),
      'examples' => array(
        'b help status' => bt('Display help for the `status` command.'),
        'b help' => bt('Display help for `b` itself.'),
      ),
    ),
  );
}

/**
 * Command callback: Provide help and examples for `b` and its commands.
 */
function help_b_callback($arguments, $options) {
  global $command_aliases;

  // Get all commands and sort them alphabetically.
  $commands = b_all_commands();
  ksort($commands);

  // Display help for the given command.
  if (isset($arguments['command'])) {
    $command = $arguments['command'];

    // If the given command is an alias, get the actual command name.
    if (isset($command_aliases[$command])) {
      $command = $command_aliases[$command];
    }

    // Make sure the command exists.
    if (!isset($commands[$command])) {
      b_message(bt("Cannot display help for `@command` as it doesn't exist.", array(
        '@command' => $command,
      )), 'error');
      return;
    }

    // Add the name of the command so it can be retrieved later.
    $commands[$command]['command'] = $command;

    return help_b_command_help($commands[$command]);
  }

  // Otherwise display help for `b` itself.
  $rows = array();
  foreach ($commands as $command => $descriptor) {
    $rows[] = array(
      array(
        'value' => $command,
        '#color' => 'blue',
        '#bold' => TRUE,
      ),
      array('value' => $descriptor['description']),
    );

    $aliases = array_keys($command_aliases, $command);
    if (!empty($aliases)) {
      $rows[] = array(
        array(
          'value' => implode(', ', $aliases),
          '#color' => 'magenta',
          '#bold' => TRUE,
        ),
        array('value' => ''),
      );
    }

    $rows[] = array();
  }

  return array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Backdrop Console'),
        '#bold' => TRUE,
      ),
    ),
    array(
      'type' => 'text',
      'variables' => array(
        'value' => bt('Usage: b [global-options] <command> [options] [arguments]') . "\n",
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
        'rows' => $rows,
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
 *   HOOK_b_command() for more details.
 */
function help_b_command_help($descriptor) {
  global $command_aliases;
  $aliases = array_keys($command_aliases, $descriptor['command']);

  // Command name.
  $output = array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => $descriptor['command'],
        '#bold' => TRUE,
      ),
      'newline' => FALSE,
    ),
  );
  if (empty($aliases)) {
    $output[0]['newline'] = TRUE;
  }
  else {
    // Aliases.
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

  return $output;
}













function help_display_one_callback($callback) {
  $output = array();

  if (isset($callback['module'])) {
    $output[] = array(
      '#type' => 'text',
      '#color' => 'bold_blue',
      'value' => bt('Extension !name', array('!name' => $callback['module'])),
    );
  }
  if (isset($callback['description'])) {
    $output[] = array(
      '#type' => 'text',
      '#color' => 'purple',
      'value' => $callback['description'],
    );
  }

  $examples = array();
  if (isset($callback['examples'])) {
    foreach ($callback['examples'] as $example => $description) {
      $examples[] = array(
        $example,
        $description,
      );
    }
  }
  if (!empty($examples)) {
    $output[] = array(
      '#type' => 'text',
      '#color' => 'bold_blue',
      'value' => "\nExamples:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $examples,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',
    );
  }
  $available_options = array();
  if (isset($callback['options'])) {
    foreach ($callback['options'] as $option => $description) {
      if (is_array($description)) {
        $available_options[] = array(
          '--' . $option,
          $description['description'],
        );
        if (isset($description['example-value'])) {
          $available_options[] = array(
            '',
            array('data' => 'example: ' . $description['example-value'], '#color' => 'purple'),
          );
        }
      }
      else {
        $available_options[] = array(
          '--' . $option,
          $description,
        );
      }
    }
  }

  if (!empty($available_options)) {
    $output[] = array(
      '#type' => 'text',
      '#color' => 'bold_blue',
      'value' => "\nOptions:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $available_options,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',
    );
  }
  $available_arguments = array();
  if (isset($callback['arguments'])) {
    foreach ($callback['arguments'] as $argument => $description) {
      if (is_array($description)) {
        $available_arguments[] = array(
          $argument,
          $description['description'],
        );
      }
      else {
        $available_arguments[] = array(
          $argument,
          $description,
        );
      }
    }
  }
  if (!empty($available_arguments)) {
    $output[] = array(
      '#type' => 'text',
      '#color' => 'bold_blue',
      'value' => "\nArguments:",
    );
    $output[] = array(
      '#type' => 'table',
      'rows' => $available_arguments,
      'delimiter' => ' : ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',
    );
  }
  if (isset($callback['aliases'])) {
    $output[] = array(
      '#type' => 'text',
      'value' => "\n" . 'Aliases: ' . implode(", ", $callback['aliases']),
    );
  }
  return $output;
}

<?php
/**
 * @file
 * Drush integration for displaying help information.
 */

/**
 * Implements hook_b_command().
 */
function drush_help_drush_command() {
  $items['help'] = array(
    'description' => 'Display drush help.',
    'arguments' => array(
      'command' => 'Command to print help',
    ),
    'callback' => 'drush_help_callback',
  );

  return $items;
}

/**
 * Callback function for hook_b_command().
 */
function drush_help_callback($command = FALSE) {
  $commands = drush_get_commands();
  ksort($commands);
  if ($command) {
    if (!isset($commands[$command])) {
      b_set_message('There is no ' . $command, 'error');
      return;
    }
    $output = drush_display_help_by_cmd($commands[$command]);
  }
  else {
    foreach ($commands as $cmd => $callbacks) {
      $first = TRUE;
      $aliases = array();
      foreach ($callbacks as $callback) {
        if (isset($callback['is_alias'])) {
          continue;
        }

        if (isset($callback['aliases'])) {
          $aliases += $callback['aliases'];
        }
        if ($first) {
          $rows[] = array(
            array('data' => $cmd, '#color' => 'bold_blue'),
            array('data' => $callback['description']),
          );
          $first = FALSE;
        }
        else {
          $rows[] = array(
            '',
            array('data' => $callback['description']),
          );
        }
      }
      if ($aliases) {
        $rows[] = array(
          '',
          array(
            'data' => 'aliases: ' . implode(", ", $aliases),
            '#color' => 'purple',
          ),
        );
      }
    }
    $output[] = array(
      '#type' => 'text',
      'value' => "\n[!] Drush commands:\n",
      '#color' => 'red',
    );

    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => '    ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',
    );
  }

  b_render($output);
}

/**
 * Display help info for the provided command.
 */
function drush_display_help_by_cmd($commands) {
  $examples = array();
  $available_options = array();
  $available_arguments = array();
  $aliases = array();
  foreach ($commands as $callback) {
    if (isset($callback['examples'])) {
      foreach ($callback['examples'] as $example => $description) {
        $examples[] = array(
          $example,
          $description,
        );
      }
    }
    if (isset($callback['options'])) {
      foreach ($callback['options'] as $option => $description) {
        $available_options[] = array(
          '--' . $option,
          $description,
        );
      }
    }
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
    if (isset($callback['aliases'])) {
      $aliases += $callback['aliases'];
    }
  }
  $output = array();
  $output[] = array(
    '#type' => 'text',
    '#color' => 'purple',
    'value' => $callback['description'],
  );

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
  if (!empty($aliases)) {
    $output[] = array(
      '#type' => 'text',
      'value' => "\n" . 'Aliases: ' . implode(", ", $aliases),
    );
  }

  return $output;
}

<?php
/**
 * Implementation of hook_b_command().
 */
function help_b_command() {
  $items['help'] = array(
    'description' => 'Display help.',
    'arguments' => array(
      'command' => 'Command to print help',
    ),
    'callback' => 'help_b_callback',
  );
  return $items;
}


/**
 * Callback function for hook_b_command().
 * @param string $arguments
 * @param $options
 *  todo: are we using this?
 * @return array $output
 *  Table of help for the command.
 */
function help_b_callback($arguments, $options) {
  if (defined('BACKDROP_ROOT')) {
    b_bootstrap(B_BOOTSTRAP_FULL);
  }
  $commands = b_get_commands();
  ksort($commands);
  $rows = array();
  $answer = array();
  if (isset($arguments['command'])) {
    $cmd = $arguments['command'];
    if (!isset($commands[$cmd])) {
      b_set_message('There is no ' . $cmd, 'error');
      return;
    }
    $output = display_help_by_cmd($commands[$cmd]);
  }
  else {
    foreach($commands as $cmd => $callbacks) {
      $first = TRUE;
      $aliases = array();
      foreach($callbacks as $callback) {
        if (isset($callback['is_alias'])) {
          continue;
        }
        if (isset($callback['aliases'])) {
          $aliases += $callback['aliases'];
        }
        if ($first) {
          $rows[] = array(
            array('data' => $cmd, '#color' => 'bold_blue'),
            array('data' => $callback['description'])
          );
          $first = FALSE;
        }
        else {
          $rows[] = array(
            '',
            array('data' => $callback['description'])
          );
        }
      }
      if ($aliases) {
        $rows[] = array(
          '',
          array('data' => 'aliases: ' . implode(", ", $aliases), '#color' => 'purple')
        );
      }
    }
    $output[] = array(
      '#type' => 'text',
      'value' => "\nBackdrop Console commands:\n",
    );
    
    $output[] = array(
      '#type' => 'table',
      'rows' => $rows,
      'delimiter' => '    ',
      'delimiter_left' => ' ',
      'delimiter_right' => '',    
    );
  }
  return $output;
}

/**
 * @param array $commands
 * @return array $output
 *  Formatted help on each command to the stdout.
 */
function display_help_by_cmd($commands){
  $output = array();
  foreach($commands as $cmd => $callback){
    $output[] = help_display_one_callback($callback);
  }
  return $output;
}

/**
 * @param string $callback
 *  Backdrop command to return help about.
 * @return array $output
 *  Array of help for the Backdrop command.
 */
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
    foreach($callback['examples'] as $example => $description){
      $examples[] = array(
        $example,
        $description
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
    foreach($callback['options'] as $option => $description) {
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
          $description
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
    foreach($callback['arguments'] as $argument => $description) {
      if (is_array($description)) {
        $available_arguments[] = array(
          $argument,
          $description['description'],
        );
      }
      else {
        $available_arguments[] = array(
          $argument,
          $description
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
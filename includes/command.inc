<?php
/**
 * @file
 * Functions to handle command dispatching.
 */

/**
 * Parse the input to extract the command, options and arguments.
 */
function b_command_parse_input() {
  global $argv, $command, $options, $arguments;

  // These variables should already be empty (since this function should only be
  // run initially via `b_init()`), but we reset them here just in case.
  $command = NULL;
  $options = array();
  $arguments = array();

  foreach ($argv as $id => $arg) {
    // Skip the first argument (the script name).
    if ($id == 0) {
      continue;
    }

    // Get any long options.
    if (preg_match('#^--(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];
      $opt_value = TRUE;

      // Get the value, if given.
      if (strpos($opt_name, '=') !== FALSE) {
        list($opt_name, $opt_value) = explode('=', $opt_name);
      }

      $options[$opt_name] = $opt_value;
      continue;
    }

    // Get any short options.
    if (preg_match('#^-(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];

      $options[$opt_name] = TRUE;
      continue;
    }

    // The first non-option argument is the command.
    if (empty($command)) {
      $command = $arg;
      continue;
    }

    // Everything else is an argument.
    $arguments[] = $arg;
  }
}

/**
 * Get the callback function for the command and process it.
 */
function b_command_process() {
  global $command, $options, $arguments, $elements;

  // Display help text if no command is given.
  if (empty($command)) {
    $command = 'help';
  }

  // Get all commands and make sure the given command is valid.
  $commands = b_command_all_commands();
  if (!isset($commands[$command])) {
    b_set_message(bt('There is no `@command` command.', array(
      '@command' => $command,
    )), 'error');
    return;
  }

  // Process the command.
  $descriptor = $commands[$command];
  $callback = $descriptor['callback'];

  if (function_exists($callback)) {
    $ready = TRUE;
    if (isset($descriptor['bootstrap'])) {
      $ready = b_bootstrap($descriptor['bootstrap']);
    }

    if ($ready) {
      if (b_command_validate($descriptor)) {
        $elements[] = $callback($arguments, $options);
      }
    }
    else {
      b_set_message(bt('Required bootstrap level for @module is not ready.', array(
        '@module' => $descriptor['module'],
      )), 'error');
    }
  }
}

/**
 * Get a list of all possible commands.
 *
 * @return array
 *   Associative array of command descriptors.
 */
function b_command_all_commands() {
  $all_commands = array();
  if (function_exists('backdrop_static')) {
    $all_commands = &backdrop_static(__FUNCTION__);
  }

  if (empty($all_commands)) {
    // Get list of command files.
    $list = b_command_file_list();

    foreach ($list as $command_file => $path) {
      require_once $path;

      // Make sure the command file's hook function exists.
      $function = $command_file . '_b_command';
      if (!function_exists($function)) {
        b_set_message(bt('`@function()` does not exist in @file. Skipping.', array(
          '@function' => $function,
          '@file' => $path,
        )), 'log');

        continue;
      }

      // Get the command descriptor(s) from the hook function.
      $result = $function();
      foreach ((array) $result as $command => $descriptor) {
        $descriptor['module'] = $command_file;
        $all_commands[$command] = $descriptor;

        // For each alias, store a copy of the descriptor.
        if (!empty($command['aliases'])) {
          foreach ($command['aliases'] as $alias) {
            $descriptor['is_alias'] = TRUE;
            $all_commands[$alias] = $descriptor;
          }
        }
      }
    }
  }

  return $all_commands;
}

/**
 * Compile a list of all available `b` command files.
 *
 * A `b` command file is a file that matches: `*.b.inc`.
 *
 * The following places are searched for command files:
 * - The `commands` directory within the `b` installation.
 * - The `.b` folder in the user's HOME directory.
 * - Enabled modules in the current Backdrop site.
 *
 * @return array
 *   An associative array whose keys are the name of the command file and whose
 *   values are the path to the command file.
 */
function b_command_file_list() {
  $list = array();
  if (function_exists('backdrop_static')) {
    $list = &backdrop_static(__FUNCTION__);
  }

  if (empty($list)) {
    // Get list of paths to search.
    $search_paths = b_command_file_search_paths();

    foreach ($search_paths as $path) {
      // Find all `.b.inc` files.
      $files = b_file_scan_directory($path, '#\.b\.inc$#');

      foreach ($files as $file) {
        $command_file = basename($file->filename, '.b.inc');
        $list[$command_file] = $file->uri;
      }
    }
  }

  return $list;
}

/**
 * Get a list of paths to search for command files.
 *
 * @return array
 *   A list of paths to directories where command files can be found.
 */
function b_command_file_search_paths() {
  $search_paths = array();
  $home = getenv('HOME') . '/.b';

  // Add `b/commands` directory.
  $search_paths[] = dirname(__DIR__) . '/commands';

  // Add `.b` folder in user's HOME directory.
  if (is_dir($home)) {
    $search_paths[] = $home;
  }

  // Add enabled modules' directories.
  if (b_backdrop_installed()) {
    $filenames = db_query('SELECT filename FROM {system} WHERE type = :type AND status = :status', array(
      ':type' => 'module',
      ':status' => 1,
    ))->fetchCol();

    foreach ($filenames as $filename) {
      $search_paths[] = dirname($filename);
    }
  }

  return $search_paths;
}

/**
 * Validate the command input (e.g. make sure required arguments and options
 * have been given).
 *
 * @param array $descriptor
 *   An associative array describing a command, its arguments and options.
 *
 * @return boolean
 *   Whether or not the command input is valid.
 */
function b_command_validate($descriptor) {
  global $options, $arguments;

  // Check for required options.
  if (isset($descriptor['options'])) {
    foreach ($descriptor['options'] as $name => $option) {
      if (isset($option['required']) && $option['required'] == TRUE) {
        if (!isset($options[$name])) {
          b_set_message(bt('Option @name is required.', array(
            '@name' => $name,
          )), 'error');

          return FALSE;
        }
      }
    }
  }

  // Check for required arguments.
  if (isset($descriptor['arguments'])) {
    foreach ($descriptor['arguments'] as $name => $argument) {
      if (isset($argument['required']) && $argument['required'] == TRUE) {
        if (!isset($arguments[$name])) {
          b_set_message(bt('Argument @name is required.', array(
            '@name' => $name,
          )), 'error');

          return FALSE;
        }
      }
    }
  }

  return TRUE;
}

<?php
/**
 * @file
 * Process and validate commands.
 */

/**
 * Parse the input to extract the command, options and arguments.
 */
function b_parse_input() {
  global $argv, $command, $arguments, $options;

  foreach ($argv as $id => $arg) {
    // Skip the first argument (the script name).
    if ($id == 0) {
      continue;
    }

    // Get any long options.
    if (preg_match('#^--(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];
      $opt_value = TRUE;

      // Get the value, if any.
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
function b_process_command() {
  global $command, $arguments, $options, $command_aliases, $output;

  // Display help text if no command is given.
  if (empty($command)) {
    $command = 'help';
  }

  // Get all commands and convert any aliases.
  $commands = b_all_commands();
  if (isset($command_aliases[$command])) {
    $command = $command_aliases[$command];
  }

  // Make sure the given command exists.
  if (!isset($commands[$command])) {
    b_message(bt('There is no `@command` command.', array(
      '@command' => $command,
    )), 'error');
    return;
  }

  // Process the command.
  $descriptor = $commands[$command];
  if (b_validate_command($descriptor)) {
    $callback = $descriptor['callback'];
    if (function_exists($callback)) {
      $ready = TRUE;
      if (isset($descriptor['bootstrap'])) {
        $ready = b_bootstrap($descriptor['bootstrap']);
      }

      if ($ready) {
        $output = $callback($arguments, $options);
      }
      else {
        b_message(bt('The required bootstrap level for `@command` is not ready.', array(
          '@command' => $command,
        )), 'error');
      }
    }
  }
}

/**
 * Get a list of all possible commands.
 *
 * @return array
 *   Associative array of command descriptors.
 */
function b_all_commands() {
  static $all_commands = array();

  if (empty($all_commands)) {
    global $command_aliases;

    // Get list of command files.
    $list = b_command_file_list();

    foreach ($list as $command_file => $path) {
      require_once $path;

      // Make sure the command file's hook function exists.
      $function = $command_file . '_b_command';
      if (!function_exists($function)) {
        b_message(bt('`@function()` does not exist in @file. Skipping.', array(
          '@function' => $function,
          '@file' => $path,
        )), 'log');
        continue;
      }

      // Get the command descriptor(s) from the hook function.
      $descriptors = (array) $function();
      foreach ($descriptors as $command => $descriptor) {
        $all_commands[$command] = $descriptor;

        // Map aliases to their commands.
        if (!empty($descriptor['aliases'])) {
          foreach ($descriptor['aliases'] as $alias) {
            $command_aliases[$alias] = $command;
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
 * A `b` command file is a file that matches `*.b.inc` in one of the supported
 * paths (@see b_command_file_paths()).
 *
 * @return array
 *   An associative array whose keys are the name of the command file and whose
 *   values are the path to the command file.
 */
function b_command_file_list() {
  static $list = array();

  if (empty($list)) {
    // Get list of paths to search.
    $search_paths = b_command_file_paths();

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
 * The following places are searched:
 * - The `commands` directory within the `b` installation.
 * - The `.b` folder in the user's HOME directory.
 * - Enabled modules in the current Backdrop site.
 *
 * @return array
 *   A list of paths to directories where command files can be found.
 */
function b_command_file_paths() {
  static $search_paths = array();

  if (empty($search_paths)) {
    global $backdrop_installed;

    // Add `b/commands` directory.
    $search_paths[] = dirname(__DIR__) . '/commands';

    // Add `.b` folder in user's HOME directory.
    $home = getenv('HOME') . '/.b';
    if (is_dir($home)) {
      $search_paths[] = $home;
    }

    // Add enabled modules' directories.
    if ($backdrop_installed) {
      $filenames = db_query('SELECT filename FROM {system} WHERE type = :type AND status = :status', array(
        ':type' => 'module',
        ':status' => 1,
      ))->fetchCol();

      foreach ($filenames as $filename) {
        $search_paths[] = dirname($filename);
      }
    }
  }

  return $search_paths;
}

/**
 * Validate the command input (e.g. make sure required arguments have been
 * given, and remove any arguments/options that aren't part of the given
 * command).
 *
 * @param array $descriptor
 *   An associative array describing a command, its arguments and options.
 *
 * @return bool
 *   Whether or not the command input is valid.
 */
function b_validate_command($descriptor) {
  global $arguments, $options;

  // Validate arguments.
  if (isset($descriptor['arguments'])) {
    $grouped = array();
    $argument_names = array_keys($descriptor['arguments']);
    $multiple = isset($descriptor['multiple_argument']) ? $descriptor['multiple_argument'] : NULL;
    $optional = isset($descriptor['optional_arguments']) ? $descriptor['optional_arguments'] : array();

    // Group provided arguments by their argument name.
    foreach ($argument_names as $name) {
      if ($name != $multiple) {
        $grouped[$name] = array_shift($arguments);
      }
      else {
        $grouped[$name] = $arguments;
        $arguments = array();
      }

      // Make sure all required arguments have a value.
      if (empty($grouped[$name]) && !in_array($name, $optional)) {
        b_message(bt('Argument `@name` is required.', array(
          '@name' => $name,
        )), 'error');
        return FALSE;
      }
    }
    $arguments = $grouped;
  }
  else {
    $arguments = array();
  }

  // Validate options.
  if (isset($descriptor['options'])) {
    // Remove any provided options that aren't used by the command.
    foreach ($options as $name => $value) {
      if (!isset($descriptor['options'][$name])) {
        unset($options[$name]);
      }
    }
  }
  else {
    $options = array();
  }

  return TRUE;
}

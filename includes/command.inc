<?php
/**
 * @file
 * Process and validate commands.
 */

/**
 * Parse the input to extract the command, options and arguments.
 */
function bee_parse_input() {
  global $argv, $_bee_command, $_bee_arguments, $_bee_options;

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
      if (($pos_equals = strpos($arg, '=')) !== FALSE) {
        // Extract option name from before '=' and ignoring the preceding '--'.
        $opt_name = substr($arg, 2, $pos_equals - 2);
        // Extract option value by taking everything from after '='.
        $opt_value = substr($arg, $pos_equals + 1);
      }

      $_bee_options[$opt_name] = $opt_value;
      continue;
    }

    // Get any short options.
    if (preg_match('#^-(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];

      $_bee_options[$opt_name] = TRUE;
      continue;
    }

    // The first non-option argument is the command.
    if (empty($_bee_command)) {
      $_bee_command = $arg;
      continue;
    }

    // Everything else is an argument.
    $_bee_arguments[] = $arg;
  }
}

/**
 * Get the callback function for the command and process it.
 */
function bee_process_command() {
  global $_bee_command, $_bee_arguments, $_bee_options, $_bee_command_aliases;
  global $_bee_output, $_bee_yes_mode, $_bee_debug_mode, $_bee_bootstrap_level;

  // Display help text if no command is given.
  if (empty($_bee_command)) {
    $_bee_command = 'help';
  }

  // Get all commands and convert any aliases.
  $commands = bee_all_commands();
  if (isset($_bee_command_aliases[$_bee_command])) {
    $_bee_command = $_bee_command_aliases[$_bee_command];
  }

  // Make sure the given command exists.
  if (!isset($commands[$_bee_command])) {
    bee_message(bt("There is no '!command' command.", array(
      '!command' => $_bee_command,
    )), 'error');
    return;
  }

  // Process the command.
  $descriptor = $commands[$_bee_command];
  if (bee_validate_command($descriptor)) {
    $callback = $descriptor['callback'];
    if (function_exists($callback)) {
      $ready = TRUE;
      if (isset($descriptor['bootstrap'])) {
        $_bee_bootstrap_level = $descriptor['bootstrap'];
        $ready = bee_bootstrap($_bee_bootstrap_level);
      }

      if ($ready) {
        // Notify about Yes mode and Debug mode if enabled. This must be done
        // after Backdrop has been bootstrapped, if applicable.
        if ($_bee_yes_mode) {
          bee_instant_message(bt("'Yes' mode enabled."));
        }
        if ($_bee_debug_mode) {
          bee_instant_message(bt("'Debug' mode enabled."));
        }

        // Send telemetry if enabled and if interval has passed since the last
        // time telemetry was sent.
        bee_invoke_telemetry();

        // Invoke the command.
        $_bee_output = $callback($_bee_arguments, $_bee_options);
      }
      else {
        bee_message(bt("The required bootstrap level for '!command' is not ready.", array(
          '!command' => $_bee_command,
        )), 'error');
      }
    }
  }
}

/**
 * Get a list of all possible commands.
 *
 * @param bool $group
 *   Whether or not to group the commands. Defaults to FALSE.
 *
 * @return array
 *   Associative array of command descriptors.
 */
function bee_all_commands($group = FALSE) {
  static $all_commands = array();
  static $grouped_commands = array();

  if (empty($all_commands) || ($group && empty($grouped_commands))) {
    global $_bee_command_aliases;

    // Get list of command files.
    $list = bee_command_file_list();

    foreach ($list as $command_file => $path) {
      require_once $path;

      // Make sure the command file's hook function exists.
      $function = $command_file . '_bee_command';
      if (!function_exists($function)) {
        bee_message(bt("'!function()' does not exist in !file. Skipping.", array(
          '!function' => $function,
          '!file' => $path,
        )), 'log');
        continue;
      }

      // Get the command descriptor(s) from the hook function.
      $descriptors = (array) $function();
      foreach ($descriptors as $command => $descriptor) {
        if ($group) {
          $group_name = (!empty($descriptor['group'])) ? $descriptor['group'] : 'miscellaneous';
          $grouped_commands[$group_name][$command] = $descriptor;
        }
        else {
          $all_commands[$command] = $descriptor;
        }

        // Map aliases to their commands.
        if (!empty($descriptor['aliases'])) {
          foreach ($descriptor['aliases'] as $alias) {
            $_bee_command_aliases[$alias] = $command;
          }
        }
      }
    }
  }

  if ($group) {
    return $grouped_commands;
  }
  else {
    return $all_commands;
  }
}

/**
 * Compile a list of all available `bee` command files.
 *
 * A `bee` command file is a file that matches `*.bee.inc` in one of the
 * supported paths (@see bee_command_file_paths()).
 *
 * @return array
 *   An associative array whose keys are the name of the command file and whose
 *   values are the path to the command file.
 */
function bee_command_file_list() {
  static $list = array();

  if (empty($list)) {
    // Get list of paths to search.
    $search_paths = bee_command_file_paths();

    foreach ($search_paths as $path) {
      // Find all `.bee.inc` files.
      $files = bee_file_scan_directory($path, '#\.bee\.inc$#');

      foreach ($files as $file) {
        $command_file = basename($file->filename, '.bee.inc');
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
 * - The `commands` directory within the `bee` installation.
 * - The `.bee` folder in the user's HOME directory.
 * - Enabled modules in the current Backdrop site.
 *
 * @return array
 *   A list of paths to directories where command files can be found.
 */
function bee_command_file_paths() {
  static $search_paths = array();

  if (empty($search_paths)) {
    global $_bee_backdrop_installed, $_bee_backdrop_root;

    // Add `bee/commands` directory.
    $search_paths[] = dirname(__DIR__) . '/commands';

    // Add `.bee` folder in user's HOME directory.
    $home = getenv('HOME') . '/.bee';
    if (is_dir($home)) {
      $search_paths[] = $home;
    }

    // Add `.bee` folder in the folder above the Backdrop root.
    $parent = dirname($_bee_backdrop_root) . '/.bee';
    if (is_dir($parent)) {
      $search_paths[] = $parent;
    }

    // Add enabled modules' directories.
    if ($_bee_backdrop_installed) {
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
function bee_validate_command(array $descriptor) {
  global $_bee_arguments, $_bee_options;

  // Validate arguments.
  if (isset($descriptor['arguments'])) {
    $grouped = array();
    $argument_names = array_keys($descriptor['arguments']);
    $multiple = isset($descriptor['multiple_argument']) ? $descriptor['multiple_argument'] : NULL;
    $optional = isset($descriptor['optional_arguments']) ? $descriptor['optional_arguments'] : array();

    // Group provided arguments by their argument name.
    foreach ($argument_names as $name) {
      if ($name != $multiple) {
        $grouped[$name] = array_shift($_bee_arguments);
      }
      else {
        $grouped[$name] = $_bee_arguments;
        $_bee_arguments = array();
      }

      // Make sure all required arguments have a value.
      if (!isset($grouped[$name]) && !in_array($name, $optional)) {
        bee_message(bt("Argument '!name' is required.", array(
          '!name' => $name,
        )), 'error');
        bee_command_show_help();
        return FALSE;
      }
    }
    $_bee_arguments = $grouped;
  }
  else {
    $_bee_arguments = array();
  }

  // Validate options.
  if (isset($descriptor['options'])) {
    // Remove any provided options that aren't used by the command.
    foreach ($_bee_options as $name => $value) {
      // Check for long form command options.
      if (!isset($descriptor['options'][$name])) {
        // Check for short form command options.
        foreach ($descriptor['options'] as $command_option_key => $command_option) {
          // Check if short option exists and it matches the provided option.
          if (isset($command_option['short']) && $command_option['short'] == $name) {
            // If it does, set the long form command option.
            $_bee_options[$command_option_key] = TRUE;
          }
        }
        // Unset the option if it doesn't match long form options now that all
        // short form options have been converted to their long form
        // equivalent.
        unset($_bee_options[$name]);
      }
    }
  }
  else {
    $_bee_options = array();
  }

  return TRUE;
}

/**
 * Show the help text for a command.
 */
function bee_command_show_help() {
  global $_bee_command, $_bee_output;

  // Show a message to give context to the following help.
  bee_message(bt('To help you get it right, here is the help text for this command:'), 'info');

  // Get the command descriptor.
  $commands = bee_all_commands();
  $command_descriptor = $commands[$_bee_command];

  // The help function we call expects the command key to exist within the
  // descriptor containing the command.
  $command_descriptor['command'] = $_bee_command;

  // Set the output to be the help text for the command.
  $_bee_output = help_bee_command_help($command_descriptor);
}

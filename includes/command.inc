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

  // These arrays should already be empty (since this function should only be
  // run initially via `b_init()`), but we reset them just in case to avoid
  // duplicate values.
  $options = array();
  $arguments = array();

  foreach ($argv as $id => $arg) {
    // Skip the first argument (the script name).
    if ($id == 0) {
      continue;
    }

    // The second argument is the command.
    if ($id == 1) {
      $command = $arg;
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

    // Everything else is an argument.
    $arguments[] = $arg;
  }
}

/**
 * Get the callback function for the command and process it.
 */
function b_command_process() {
  global $command, $options, $arguments;

  // Display help text if no command is given.
  if (empty($command)) {
    $command = 'help';
  }

  // Get all commands and make sure the given command is valid.
  $commands = b_command_all_commands();
  if (!isset($commands[$command])) {
    b_set_message(bt('There is no `@command` command.', array('@command' => $command)), 'error');
    return;
  }

  // Process the command.
  foreach ($commands[$command] as $cmd) {
    $callback = $cmd['callback'];
    if (function_exists($callback)) {
      $arguments = array();
      $options = array();
      $ready = TRUE;
      if (isset($command['bootstrap'])) {
        $ready = b_bootstrap($command['bootstrap']);
      }
      if ($ready) {
        b_get_command_args_options($arguments, $options, $command);
        if (b_check_for_required($arguments, $options, $command)) {
          $elements[] = $command_callback($arguments, $options);
        }
      }
      else {
        b_set_message("Required bootstrap level for " . $command['module'] . " is not ready", 'error');
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
      // TODO: Up to here.
      $function = $command_file . '_b_command';
      if (!function_exists($function)) {
        b_set_message(dt("Skip !function. It does not exists.", array('!function' => $function)), 'log');
        continue;
      }
      $result = $function();

      foreach ((array) $result as $key => $command) {
        if (!isset($commands[$key])) {
          $commands[$key] = array();
        }
        $command['module'] = $command_file;
        $commands[$key][] = $command;
        // For every alias, make a copy of the command and store it in the
        // command list using the alias as a key.
        if (isset($command['aliases']) && count($command['aliases'])) {
          foreach ($command['aliases'] as $alias) {
            $command['is_alias'] = TRUE;
            if (!isset($commands[$alias])) {
              $commands[$alias] = array();
            }
            $commands[$alias][] = $command;
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
 * Check for required options and arguments.
 */
function b_check_for_required(&$arguments, &$options, $command) {
  if (isset($command['options'])) {
    foreach ($command['options'] as $name => $option) {
      if (is_array($option)) {
        if (isset($option['required']) && $option['required'] == TRUE) {
          if (!isset($options[$name])) {
            b_set_message(bt('Option !name is required.', array('!name' => $name)), 'error');
            return FALSE;
          }
        }
      }
    }
  }

  if (isset($command['arguments'])) {
    foreach ($command['arguments'] as $name => $argument) {
      if (is_array($argument)) {
        if (isset($argument['required']) && $argument['required'] == TRUE) {
          if (!isset($arguments[$name])) {
            b_set_message(bt('Argument !name is required.', array('!name' => $name)), 'error');
            return FALSE;
          }
        }
      }
    }
  }

  return TRUE;
}

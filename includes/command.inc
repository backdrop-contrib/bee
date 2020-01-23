<?php
/**
 * @file
 * Functions to handle command dispatching.
 */

/**
 * Assign $arguments and $options.
 */
function b_command_get_options() {
  global $global_options, $argv;
  $options = array();

  foreach ($argv as $id => $arg) {
    // Skip the first argument (the script name).
    if ($id == 0) {
      continue;
    }

    // Find long options.
    if (preg_match('#^--(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];
      $opt_value = TRUE;

      if (strpos($opt_name, '=') !== FALSE) {
        list($opt_name, $opt_value) = explode('=', $opt_name);
      }

      if (isset($global_options[$opt_name])) {
        $options[$opt_name] = $opt_value;
      }

      continue;
    }

    // Find short options.
    if (preg_match('#^-(\S+)#', $arg, $matches)) {
      $opt_name = $matches[1];

      foreach ($global_options as $global_option) {
        if (isset($global_option['short']) && $global_option['short'] == $opt_name) {
          $options[$opt_name] = TRUE;
        }
      }

      continue;
    }




    if ($multiple_option) {
      $arguments[$arg_name][] = $argv[$i];
      continue;
    }

    if (empty($command_name)) {
      $command_name = $argv[$i];
      continue;
    }

    if (!empty($command_args)) {
      if (!$multiple_option) {
        $arg_name = array_shift($command_args);
      }
      if (is_array($command['arguments'][$arg_name])
        && isset($command['arguments'][$arg_name]['multiple'])
        && $command['arguments'][$arg_name]['multiple']) {

        $arguments[$arg_name][] = $argv[$i];
        $multiple_option = TRUE;
      }
      else {
        $arguments[$arg_name] = $argv[$i];
      }
    }
  }

  return $options;
}

/**
 * Search for all callbacks on command and process it.
 */
function b_process_command() {
  // @codingStandardsIgnoreLine -- Not sure of the repercussions of renaming these...
  global $argv, $elements;

  $cmd = b_get_command_name();
  if (empty($cmd)) {
    $cmd = 'help';
  }
  $commands = b_get_commands();
  if (!isset($commands[$cmd])) {
    b_set_message('There is no ' . $cmd, 'error');
    return;
  }
  foreach ($commands[$cmd] as $command) {
    $command_callback = $command['callback'];
    if (function_exists($command_callback)) {
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
 * Search for first not option value.
 */
function b_get_command_name() {
  global $argc, $argv;
  for ($i = 1; $i < $argc; $i++) {
    if (preg_match('/^--(\S+)/', $argv[$i], $matches)) {
      continue;
    }

    if (preg_match('/^-(\S+)/', $argv[$i], $matches)) {
      continue;
    }

    return $argv[$i];
  }

  return FALSE;
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

/**
 * Get a list of all implemented commands.
 *
 * @return array
 *   Associative array of currently active command descriptors.
 */
function b_get_commands() {
  $commands = array();
  if (function_exists('backdrop_static')) {
    $commands = &backdrop_static(__FUNCTION__);
  }
  if (empty($commands)) {
    $list = b_commandfile_list();
    foreach ($list as $command_file => $path) {
      require_once $path;
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
  return $commands;
}

/**
 * Collect a list of all available b command files.
 *
 * Scans the following paths for drush command files:
 *
 * - The "/path/to/b/commands" folder.
 * - The ".b" folder in the user's HOME folder.
 * - Folders belonging to enabled modules in the current Backdrop site.
 *
 * A B command file is a file that matches "*.b.inc".
 *
 * @see file_scan_directory()
 *
 * @return array
 *   An array whose values are all available command files.
 */
function b_commandfile_list() {
  if (function_exists('backdrop_static')) {
    $list = &backdrop_static(__FUNCTION__);
  }
  if (empty($list)) {
    $list = array();
    $search_path = b_commandfile_searchpaths();
    foreach ($search_path as $path) {
      $files = b_file_scan_directory($path, "/\.b\.inc$/");
      foreach ($files as $file) {
        $command_file = basename($file->filename, '.b.inc');
        $list[$command_file] = $file->uri;
      }
    }
  }
  return $list;
}

/**
 * Get paths to search for command files.
 *
 * @return array
 *   A list of paths to directories where command files can be found.
 */
function b_commandfile_searchpaths() {
  $search_path = array();
  $home = getenv("HOME");

  if (is_dir($home . '/.b')) {
    $search_path[] = $home . '/.b';
  }

  // Add commands files.
  $search_path[] = dirname(__DIR__) . '/commands/';

  if (b_backdrop_installed()) {
    $modules = db_select('system', 's')
      ->fields('s')
      ->condition('status', 1)
      ->condition('type', 'module')
      ->execute()
      ->fetchAll();
    foreach ($modules as $module) {
      $pathinfo = pathinfo($module->filename);
      $search_path[] = $pathinfo['dirname'];
    }
  }
  return $search_path;
}

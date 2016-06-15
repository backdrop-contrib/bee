<?php
/*
 * @file
 * Functions handle command dispatching.
 */

/**
 * Get a list of all implemented commands.
 * This invokes hook_drush_command().
 *
 * @return
 *   Associative array of currently active command descriptors.
 *
 */
function b_get_commands() {
  $commands = &backdrop_static(__FUNCTION__);
  if ($commands) {
    return $commands;
  }
  
  $list = b_commandfile_list();
  foreach ($list as $command_file => $path) {
    require_once($path);
    $function = $command_file . '_b_command';
    function_exists($function);
    $result = $function();
    
    foreach ((array)$result as $key => $command) {
      $commands[$key] = $command;
      // For every alias, make a copy of the command and store it in the command list
      // using the alias as a key
      if (isset($command['aliases']) && count($command['aliases'])) {
        foreach ($command['aliases'] as $alias) {
          $commands[$alias] = $command;
          $commands[$alias]['is_alias'] = TRUE;
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
 * @return
 *   An array whose values are all available command files.
 */
function b_commandfile_list() {
  $list = &backdrop_static(__FUNCTION__);
  if(empty($list)) {
    $list = array();
    $search_path = b_commandfile_searchpaths();
    foreach($search_path as $path) {    
      $files = file_scan_directory($path, "/\.b\.inc$/");
      foreach ($files as $file) {
        $command_file = basename($file->uri);
        $list[$command_file] = $file->uri;
      }
    }
  }
  return $list;
}

/**
 * @return array of strings - paths to directories where command files
 * can be found.
 */
function b_commandfile_searchpaths() {
  $search_path = array();
  $home = getenv("HOME");
  
  if(is_dir($home . '/.b')) {
    $search_path[] = $home . '/.b';
  }
  
  // Add commands files
  $search_path[] = dirname(__DIR__) . '/commands/';
  
  if(function_exists('module_list') && function_exists('db_select')) {
    $modules = db_select('system', 's')
      ->fields('s')
      ->condition('status', 1)
      ->condition('type', 'module')
      ->execute()
      ->fetchAll();
    foreach($modules as $module) {
      $pathinfo = pathinfo($module->filename);
      $search_path[] = $pathinfo['dirname'];
    }
  }
  return $search_path;
}
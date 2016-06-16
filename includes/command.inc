<?php
/*
 * @file
 * Functions handle command dispatching.
 */

/**
 * Search for all callbacks on command and process it
 */

function b_process_command(){
  global $argv, $elements;
  if(!isset($argv[1])) {
    $cmd = 'help';
  }
  else{
    $cmd = $argv[1];  
  }
  $commands = b_get_commands();
  if(!isset($commands[$cmd])){
    b_set_message('There is no ' . $cmd , 'error');
    return;
  }
  foreach($commands[$cmd] as $command) {
    $command_callback = $command['callback'];
    if(function_exists($command_callback)){ 
      $arguments = array();
      $options = array();
      $ready = TRUE;
      if(isset($command['bootstrap'])) {
        $ready = b_bootstrap($command['bootstrap']);
      }
      if($ready) {
        b_get_command_args_options($arguments, $options, $command);
        $elements[] = $command_callback($arguments, $options);
      }
      else{
        b_set_message("Required bootstrap level for " . $command['module']. " is not ready", 'error');
      }
    }
  }
}

function b_bootstrap($bootstrap_level){
  if(defined('BACKDROP_ROOT')) {
    require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
    if(function_exists('backdrop_bootstrap')) {
      chdir(BACKDROP_ROOT);
      $status = backdrop_bootstrap($bootstrap_level);
      set_error_handler('b_errorHandler');
      return TRUE;
    }
  }
  return FALSE;
}
/**
 * Assign $arguments and $options based on 
 * $command['arguments'] & $command[options]
 *
 */
function b_get_command_args_options(&$arguments, &$options, $command) {
  global $argc, $argv;
  
  $command_args = array();
  
  if(isset($command['arguments'])){
    $command_args = array_keys($command['arguments']);
  }
  for($i=2;$i < $argc; $i++) {
    if (preg_match('/--(\S+)/', $argv[$i], $matches)) {
      $opt_name = $matches[1];
      if(FALSE !== strpos($opt_name, '=')) {
        list($opt_name, $opt_value) = explode('=', $opt_name);
      }
      if(empty($opt_value)){
        $opt_value = TRUE;
      }
      if(isset($command['options'][$opt_name])) {
        $options[$opt_name] = $opt_value;
      }
      continue;
    }
    if(!empty($command_args)) {
      $arg_name = array_shift($command_args);
      $arguments[$arg_name] = $argv[$i];
    }
  }
}

/**
 * Get a list of all implemented commands.
 *
 * @return
 *   Associative array of currently active command descriptors.
 *
 */
function b_get_commands() {
  if(function_exists('backdrop_static')) {
    $commands = &backdrop_static(__FUNCTION__);
  }
  if (empty($commands)) {
    $list = b_commandfile_list();
    foreach ($list as $command_file => $path) {
      require_once($path);
      $function = $command_file . '_b_command';
      if(!function_exists($function)) {
        echo " skip  $function \n";
        continue;
      }
      $result = $function();
      
      foreach ((array)$result as $key => $command) {
        if(!isset($commands[$key])){
          $commands[$key] = array();
        }
        $command['module'] = $command_file;
        $commands[$key][] = $command;
        // For every alias, make a copy of the command and store it in the command list
        // using the alias as a key
        if (isset($command['aliases']) && count($command['aliases'])) {
          foreach ($command['aliases'] as $alias) {
            $command['is_alias'] = TRUE;
            if(!isset($commands[$alias])){
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
 * @return
 *   An array whose values are all available command files.
 */
function b_commandfile_list() {
  if(function_exists('backdrop_static')) {
    $list = &backdrop_static(__FUNCTION__);
  }
  if(empty($list)) {
    $list = array();
    $search_path = b_commandfile_searchpaths();
    foreach($search_path as $path) {    
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
<?php
/*
 * @file
 * Functions for drush wrapper.
 */

/**
 * Final bootstrap phase: Backdrop is fully loaded; validate and fix input data.
 */
define('DRUSH_BOOTSTRAP_DRUPAL_FULL', 8);

include_once('drush_wrapper/output.inc');
include_once('drush_wrapper/drush.inc');


function drush_process_command() {
  global $argv, $elements;
  $cmd = b_get_command_name();
  if(empty($cmd)) {
    $cmd = 'help';
  }
  
  $commands = drush_get_commands();
  if(!isset($commands[$cmd])){
    b_set_message(dt('There is no !cmd', array('!cmd' => $cmd)) , 'error');
    return;
  }
  foreach($commands[$cmd] as $command) {
    
    if(!$command_callback = drush_get_command($cmd, $command)){
      b_set_message(dt("There is no callback"), 'error');
      continue;
    }
    if(function_exists($command_callback)){
      $arguments = drush_get_arguments();
      $ready = TRUE;
      if(isset($command['bootstrap'])) {
        $ready = b_bootstrap($command['bootstrap']);
      }
      if($ready) {
        call_user_func_array($command_callback, $arguments);
      }
      else{
        b_set_message(dt("Required bootstrap level for !module is not ready", array('!module' => $command['module'])), 'error');
      }
    }
    else{
      b_set_message(dt("There is no function !callback", array('!callback' => $command_callback)), 'error');
    }
  }
}

function drush_get_command($cmd, $command){
  if(isset($command['callback'])) {
    return $command['callback'];
  }
  
  
  if(isset($command['is_alias'])) {
    $cmd = $command['original'];
  }
  
  $hook = str_replace("-", "_", $cmd);
  $func = 'drush_' . $hook;

  if(function_exists($func)) {
    return $func;
  }
  
  $func = 'drush_' . $command['module'] . '_' . $hook. "\n";

  if(function_exists($func)) {
    return $func;
  }

}

/**
 * search for first not option value
 */
function drush_get_arguments(){
  global $argc, $argv;
  $arguments = array();
  for($i=1;$i < $argc; $i++) {
    if (preg_match('/--(\S+)/', $argv[$i], $matches)) {
      continue;
    }
    
    if(preg_match('/-(\S+)/', $argv[$i], $matches)) {
      continue;
    }
    
    $arguments[] = $argv[$i];
  }
  array_shift($arguments);
  return $arguments;
}

/**
 * Get a list of all implemented commands.
 *
 * @return
 *   Associative array of currently active command descriptors.
 *
 */
function drush_get_commands() {
  if(function_exists('backdrop_static')) {
    $commands = &backdrop_static(__FUNCTION__);
  }
  if (empty($commands)) {
    $list = _drush_commandfile_list();
    foreach ($list as $command_file => $path) {
      require_once($path);
      $function = $command_file . '_drush_command';
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
            $command['original'] = $key;
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
 * Collect a list of all available drush command files.
 *
 * Scans the following paths for drush command files:
 *
 * - The ".drush" folder in the user's HOME folder.
 * - Folders belonging to enabled modules in the current Backdrop site.
 *
 * A Drush command file is a file that matches "*.drush.inc".
 *
 * @see file_scan_directory()
 *
 * @return
 *   An associative array whose keys and values are the names of all available
 *   command files.
 */
function _drush_commandfile_list() {
  if(function_exists('backdrop_static')) {  
    $list = &backdrop_static(__FUNCTION__);
  }
  if(empty($list)) {
    $list = array();
    $search_path = _drush_commandfile_searchpaths();
    foreach($search_path as $path) {        
      $files = file_scan_directory($path, "/\.drush\.inc$/");
      foreach ($files as $file) {
        $command_file = basename($file->name, '.drush');
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
function _drush_commandfile_searchpaths() {
  $search_path = array();
  $home = getenv("HOME");
  
  if(is_dir($home . '/.drush')) {
    $search_path[] = $home . '/.drush';
  }
  // Add commands files
  $search_path[] = dirname(__DIR__) . '/commands/';

  b_bootstrap(B_BOOTSTRAP_FULL);
  
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

/**
 * @see t()
 */
function dt($string, $args = array()){
  return bt($string,$args);
}



/**
 * Get the value for an option and return it as a list.  If the
 * option in question is passed on the command line, its value should
 * be a comma-separated list (e.g. --flag=1,2,3).  If the option
 * was set in a drushrc.php file, then its value may be either a
 * comma-separated list or an array of values (e.g. $option['flag'] = array('1', '2', '3')).
 *
 * @param option
 *   The name of the option to get
 * @param default
 *   Optional. The value to return if the option has not been set
 * @param context
 *   Optional. The context to check for the option. If this is set, only this context will be searched.
 */
function drush_get_option_list($name, $default = array(), $context = NULL){
  global $argc, $argv;
  
  $result = array();
  for($i=2;$i < $argc; $i++) {
    if (preg_match('/^--(\S+)/', $argv[$i], $matches)) {
      $opt_name = $matches[1];
      if(FALSE !== strpos($opt_name, '=')) {
        list($opt_name, $opt_value) = explode('=', $opt_name);
      }
      if(empty($opt_value)){
        $opt_value = TRUE;
      }
    }
    
    if(preg_match('/^-(\S+)/', $argv[$i], $matches)) {
      $opt_name = $matches[1];
      $opt_value = TRUE;
    }
    
    if($opt_name == $name) {
      $result[] = $opt_value;
    }
  }
  if(empty($result) && !empty($default)) {
    return $default;
  }
  return $result;
}

/**
 * Get the value for an option.
 *
 * If the first argument is an array, then it checks whether one of the options
 * exists and return the value of the first one found. Useful for allowing both
 * -h and --host-name
 *
 * @param option
 *   The name of the option to get
 * @param default
 *   Optional. The value to return if the option has not been set
 * @param context
 *   Optional. The context to check for the option. If this is set, only this context will be searched.
 */
function drush_get_option($option, $default = NULL, $context = NULL){
  $options = drush_get_option_list($option);
  if(!empty($options)) {
    return $options[0];
  }
  return $default;
}

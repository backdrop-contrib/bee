<?php
/*
 * @file
 * Functions for drush wrapper.
 */
 
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
function _drush_commandfile_list($path) {
  $list = &backdrop_static(__FUNCTION__);
  if(empty($list)) {
    $list = array();
    $search_path = _drush_commandfile_searchpaths();
    foreach($search_path as $path) {        
      $files = file_scan_directory($path, "/\.drush(7|)\.inc$/");
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
function _drush_commandfile_searchpaths() {
  $search_path = array();
  $home = getenv("HOME");
  
  if(is_dir($home . '/.drush')) {
    $search_path[] = $home . '/.drush';
  }
  
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
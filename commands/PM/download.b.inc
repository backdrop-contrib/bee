<?php

define('DOWNLOAD_VIA_GIT', 2);
/**
 * Implements of hook_b_command().
 */
function download_b_command() {
  $items['pm-download'] = array(
    'description' => 'Download contrib package.',
    'arguments' => array(
      'packages' => array(
        'description' => 'Package to download.',
        'multiple' => TRUE,
      ),
    ),
    'options' => array(
      'root' => 'Backdrop ROOT directory',
    ),
    'callback' => 'download_b_callback',
    'aliases' => array('dl'),
  );
  return $items;
}

/**
 * dl callback. Download project.
 */
function download_b_callback($arguments, $options) {

  if (!isset($arguments['packages'])) {
    b_set_message('Please provide package name(s)', 'error');
    return;
  }
  foreach($arguments['packages'] as $package) {
    _download_project($package, $options);
  }
}

/**
 * Internal function to download project by name.
 */
function _download_project($project, $options) {
  $tmp_folder = b_tmp_folder();
  if (!$tmp_folder) {
    b_set_message('Failed to generate tmp directory!', 'error');
    return;
  }
  if ($project == 'backdrop') {
    $owner = 'backdrop';
    $link = get_project_link('backdrop', $project);
  }else{
    $owner = 'backdrop-contrib';
    if(!$link = get_project_link('backdrop-contrib', $project)){
      return FALSE;
    }
  }

  if(DOWNLOAD_VIA_GIT == $link) {
    exec('git clone https://github.com/' . $owner . '/' . $project . ' ' . $tmp_folder . '/' . $project);
  }
  else{
    exec('wget -q --directory-prefix ' . $tmp_folder . ' ' . $link);
    exec('unzip ' . $tmp_folder . '/' . $project . '.zip -d ' .$tmp_folder);
    unlink($tmp_folder . '/' . $project . '.zip');
  }

  if($project == 'backdrop') {
    $download_target = '.';
    if (isset($options['root'])) {
      $download_target = $options['root'];
    }
    elseif (defined('BACKDROP_ROOT')) {
      $download_target = BACKDROP_ROOT;
    }
  }
  else {
    $download_target = download_get_target($tmp_folder . '/' . $project, $options);
    if (file_exists($download_target. '/' . $project)) {
      b_set_message('Folder ' . $download_target. '/' . $project . ' already exists', 'error');
      b_file_unmanaged_delete_recursive($tmp_folder);
      return FALSE;
    }
  }

  b_recursive_copy($tmp_folder . '/' . $project, $download_target. '/' . $project);
  b_set_message($project . ' downloaded!','success');
  b_file_unmanaged_delete_recursive($tmp_folder);

}
/**
 * Based on type of the project, return directory to move project into.
 */
function download_get_target($folder, $options) {
  $download_target = FALSE;
  if(isset($options['root'])) {
    $download_target = $options['root'];
  }elseif(defined('BACKDROP_ROOT')){
    $download_target = BACKDROP_ROOT;
  }
  if(!$download_target) {
    b_set_message('Please change directory to BackdropCMS root or specify --root option.', 'error');
    return FALSE;
  }
  if($type = download_get_type($folder)){
    switch($type) {
      case 'module':
        $download_target .= '/modules/';
      break;
      case 'theme':
        $download_target .= '/themes/';
      break;
      case 'layout':
        $download_target .= '/layouts/';
      break;
    }
    if(file_exists($download_target . '/contrib')) {
      $download_target = $download_target . '/contrib';
    }
    return $download_target;
  }
  else{
    b_set_message($package . ' is unknown type!', 'error');
  }
  return FALSE;
}

/**
 * Scan for info file and return project type.
 */
function download_get_type($folder) {
  $files = b_file_scan_directory($folder, "/\.info$/");
  foreach($files as $file) {
    $content = file($file->uri);
    foreach($content as $line) {
      if(FALSE !== strpos($line, 'type')) {
        list($title, $type) = explode("=", $line);
        $type = trim($type);
        if(in_array($type, array('module', 'theme', 'layout'))){
          return $type;
        }
      }
    }
  }
  return FALSE;
}

/**
 * Return linkg to zip file from GitHub.
 */
function get_project_link($owner, $repo) {
  $context = stream_context_create(
    array(
      "http" => array(
        'method'=>"GET",
        'header'=>"Accept-language: en\r\n" .
              "User-Agent: B.project\r\n"
      )
    )
  );

  $message_variables = array('!owner' => $owner, '!repo' => $repo);
  // Check if project exists first.
  $content = file_get_contents('https://api.github.com/repos/' . $owner . '/' . $repo, false, $context);
  if(!$content) {
    b_set_message(bt('!owner/!repo not found', $message_variables), 'error');
    return FALSE;
  }

  $content = file_get_contents('https://api.github.com/repos/' . $owner . '/' . $repo .'/releases/latest', false, $context);

  if($content) {
    $json = json_decode($content, TRUE);
    if(isset($json['assets'][0]['browser_download_url'])
       && $json['assets'][0]['name'] == $repo . '.zip'){
      return $json['assets'][0]['browser_download_url'];
    }
  }

  if (!b_confirm(bt("There is no release for !owner/!repo\nDo you you want to download *git* version instead?", $message_variables))) {
    b_set_message(bt('Cancelled to download *git* version for !owner/!repo.', $message_variables));
    return FALSE;
  }
  return DOWNLOAD_VIA_GIT;
}

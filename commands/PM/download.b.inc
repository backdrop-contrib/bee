<?php
/**
 * @file download.b.inc
 *  Download Backdrop CMS project(s) (modules, themes or layouts) from github.
 */

define('DOWNLOAD_VIA_GIT', 2);

/**
 * Implements hook_b_command().
 */
function download_b_command() {
  $items['pm-download'] = array(
    'description' => bt('Download contrib package.'),
    'arguments' => array(
      'packages' => array(
        'description' => bt('Package to download.'),
        'multiple' => TRUE,
      ),
    ),
    'options' => array(
      'root' => bt('Backdrop ROOT directory'),
      'type' => bt('Fallback type if not detected by info file'),
    ),
    'callback' => 'download_b_callback',
    'aliases' => array('dl'),
  );
  return $items;
}

/**
 * dl callback. Download project.
 *
 * @param array $arguments
 * @param array $options
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
 *
 * @param string $project
 * @param array $options
 */
function _download_project($project, $options) {
  if ($project == 'backdrop') {
    $owner = 'backdrop';
    $link = get_project_link('backdrop', $project);
  }
  else {
    $owner = 'backdrop-contrib';
    if (!$link = get_project_link('backdrop-contrib', $project)) {
      return FALSE;
    }
  }

  $tmp_folder = b_tmp_folder();
  if (!$tmp_folder) {
    b_set_message('Failed to generate tmp directory!', 'error');
    return;
  }

  if(DOWNLOAD_VIA_GIT == $link) {
    exec('git clone https://github.com/' . $owner . '/' . $project . ' ' . $tmp_folder . '/' . $project);
  }
  else {
    exec('wget -q -O ' . $tmp_folder . '/' . $project . '.zip ' . $link);
    exec('unzip ' . $tmp_folder . '/' . $project . '.zip -d ' . $tmp_folder);
    unlink($tmp_folder . '/' . $project . '.zip');
  }

  if ($project == 'backdrop') {
    $download_target = '.';
    if (isset($options['root'])) {
      $download_target = $options['root'];
    }
    elseif (defined('BACKDROP_ROOT')) {
      $download_target = BACKDROP_ROOT;
    }
  }
  else {
    $download_target = download_get_target($project, $tmp_folder . '/' . $project, $options);
    if (FALSE == $download_target) {
      b_set_message('Failed to detect destination directory.', 'error');
      b_file_unmanaged_delete_recursive($tmp_folder);
      return FALSE;
    }
    if (file_exists($download_target . '/' . $project)) {
      b_set_message('Folder ' . $download_target. '/' . $project . ' already exists', 'error');
      b_file_unmanaged_delete_recursive($tmp_folder);
      return FALSE;
    }
  }

  b_recursive_copy($tmp_folder . '/' . $project, $download_target . '/' . $project);
  b_set_message($project . ' downloaded!','success');
  b_file_unmanaged_delete_recursive($tmp_folder);
}

/**
 * Based on type of the project, return directory to move project into.
 *
 * @param string $project
 * @param string $folder
 * @param array $options
 * @return $download_target or FALSE
 *  A string that is the directory path to the destination of the project or FALSE.
 */
function download_get_target($project, $folder, $options) {
  $download_target = FALSE;
  if (isset($options['root'])) {
    $download_target = $options['root'];
  }
  elseif (defined('BACKDROP_ROOT')) {
    $download_target = BACKDROP_ROOT;
  }
  if (!$download_target) {
    b_set_message('Please change directory to BackdropCMS root or specify --root option.', 'error');
    return FALSE;
  }
  $type = download_get_type($folder);
  if (FALSE == $type && isset($options['type'])) {
    $type = $options['type'];
  }
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
    default:
      b_set_message(bt('!project does not have type.', array('!project' => $project)), 'error');
      return FALSE;
  }
  if (file_exists($download_target . '/contrib')) {
    $download_target = $download_target . '/contrib';
  }
  return $download_target;
}

/**
 * Scan for info file and return project type.
 *
 * @param string $folder
 * @return $type or FALSE
 *  either string of type of project: module, theme or layout or FALSE
 */
function download_get_type($folder) {
  $files = b_file_scan_directory($folder, "/\.info$/");
  foreach($files as $file) {
    $content = file($file->uri);
    foreach($content as $line) {
      if (FALSE !== strpos($line, 'type')) {
        list($title, $type) = explode("=", $line);
        $type = trim($type);
        if (in_array($type, array('module', 'theme', 'layout'))) {
          return $type;
        }
      }
    }
  }
  return FALSE;
}

/**
 * Return link to zip file from GitHub.
 *
 * @param string $owner
 * @param string $repo
 * @return string
 *  either URL to project on github or 2.
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
  $content = file_get_contents('https://api.github.com/repos/' . $owner . '/' . $repo, FALSE, $context);
  if (!$content) {
    b_set_message(bt('!owner/!repo not found', $message_variables), 'error');
    return FALSE;
  }

  $content = file_get_contents('https://api.github.com/repos/' . $owner . '/' . $repo .'/releases/latest', FALSE, $context);

  if ($content) {
    $json = json_decode($content, TRUE);
    if (isset($json['assets'][0]['browser_download_url'])
       && $json['assets'][0]['name'] == $repo . '.zip') {
      return $json['assets'][0]['browser_download_url'];
    }
  }

  if (!b_confirm(bt("There is no release for !owner/!repo\nDo you you want to download *git* version instead?", $message_variables))) {
    b_set_message(bt('Cancelled to download *git* version for !owner/!repo.', $message_variables));
    return FALSE;
  }
  return DOWNLOAD_VIA_GIT;
}

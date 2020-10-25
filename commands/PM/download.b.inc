<?php
/**
 * @file
 * Download Backdrop CMS project(s) (modules, themes or layouts) from github.
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
      'type' => bt('Fallback type if not detected by info file'),
      'dir' => bt("(Only used when downloading Backdrop) The directory to download and extract Backdrop into. If not set, Backdrop will be downloaded to a 'backdrop' folder in the current directory."),
    ),
    'callback' => 'download_b_callback',
    'aliases' => array('dl'),
  );
  return $items;
}

/**
 * Download project. dl callback.
 *
 * @param array $arguments
 *   An array of packages do download.
 * @param array $options
 *   An array of options, i.e. type.
 */
function download_b_callback($arguments, $options) {
  if (!isset($arguments['packages'])) {
    b_message('Please provide package name(s)', 'error');
    return;
  }
  foreach ($arguments['packages'] as $package) {
    _download_project($package, $options);
  }
}

/**
 * Internal function to download project by name.
 *
 * @param string $project
 *   The project name.
 * @param array $options
 *   The options for the command.
 */
function _download_project($project, $options) {
  // Get the project owner.
  $owner = 'backdrop-contrib';
  if ($project == 'backdrop') {
    $owner = 'backdrop';
  }

  if (!$link = get_project_link($owner, $project)) {
    return FALSE;
  }

  if ($tmp_folder = b_find_temp()) {
    $tmp_folder .= '/' . hash('sha256', mt_rand());
    mkdir($tmp_folder);
  }
  else {
    b_message('Failed to generate temp directory!', 'error');
    return;
  }

  if ($link == DOWNLOAD_VIA_GIT) {
    exec('git clone https://github.com/' . $owner . '/' . $project . ' ' . $tmp_folder . '/' . $project);
  }
  else {
    exec('wget -q -O ' . $tmp_folder . '/' . $project . '.zip ' . $link);
    exec('unzip ' . $tmp_folder . '/' . $project . '.zip -d ' . $tmp_folder);
    unlink($tmp_folder . '/' . $project . '.zip');
  }

  if ($project == 'backdrop') {
    $download_target = getcwd();
    if (isset($options['dir'])) {
      if (!realpath($options['dir'])) {
        // Create the folder if it doesn't exist.
        if (!mkdir($options['dir'], 0755)) {
          b_message("Folder " . $options['dir'] . " doesn't exist and attempts to create it failed.", 'error');
          b_delete($tmp_folder);
          return FALSE;
        }
      }
      $download_target = realpath($options['dir']);
    }

    // Cannot download Backdrop into an existing installation.
    if (b_find_root($download_target)) {
      b_message('Downloading Backdrop into an existing Backdrop installation is not allowed. Please run the command outside of the installation or use the `--dir` option to specify a different destination.', 'error');
      b_delete($tmp_folder);
      return FALSE;
    }

    if (!isset($options['dir'])) {
      $download_target = $download_target . '/' . $project;
    }

    if (file_exists($download_target . '/index.php')) {
      b_message('There is already a Backdrop installation in ' . $download_target . '.', 'error');
      b_delete($tmp_folder);
      return FALSE;
    }
  }
  else {
    $download_target = download_get_target($project, $tmp_folder . '/' . $project, $options);

    if (!$download_target) {
      b_message('Failed to detect destination directory.', 'error');
      b_delete($tmp_folder);
      return FALSE;
    }

    if (file_exists($download_target . '/' . $project)) {
      b_message('Folder ' . $download_target . '/' . $project . ' already exists', 'error');
      b_delete($tmp_folder);
      return FALSE;
    }

    // Create the folder if it doesn't exist.
    if (!file_exists($download_target)) {
      if (!mkdir($download_target, 0755)) {
        b_message("Folder " . $download_target . " doesn't exist and attempts to create it failed.", 'error');
        b_delete($tmp_folder);
        return FALSE;
      }
    }

    $download_target = $download_target . '/' . $project;
  }

  b_copy($tmp_folder . '/' . $project, $download_target);
  b_message($project . ' downloaded to ' . $download_target . '!', 'success');
  b_delete($tmp_folder);
}

/**
 * Based on type of the project, return directory to move project into.
 *
 * @param string $project
 *   The machine name of the project.
 * @param string $folder
 *   The folder.
 * @param array $options
 *   Array of options for the command.
 *
 * @return string|FALSE
 *   The directory path to the destination of the project or FALSE.
 */
function download_get_target($project, $folder, $options) {
  $download_target = FALSE;
  if (defined('BACKDROP_SITE')) {
    // If BACKDROP_SITE is set, we know BACKDROP_ROOT is set too.
    $download_target = BACKDROP_ROOT . '/sites/' . BACKDROP_SITE;
  }
  elseif (defined('BACKDROP_ROOT')) {
    $download_target = BACKDROP_ROOT;
  }
  if (!$download_target) {
    b_message('Please change directory to BackdropCMS root or specify --root option.', 'error');
    return FALSE;
  }
  $type = download_get_type($folder);
  if (FALSE == $type && isset($options['type'])) {
    $type = $options['type'];
  }
  switch ($type) {
    case 'module':
      $download_target .= '/modules';
      break;

    case 'theme':
      $download_target .= '/themes';
      break;

    case 'layout':
      $download_target .= '/layouts';
      break;

    default:
      b_message(bt('!project does not have type.', array('!project' => $project)), 'error');
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
 *   The folder.
 *
 * @return string|FALSE
 *   Either string of type of project: module, theme or layout or FALSE
 */
function download_get_type($folder) {
  $files = b_file_scan_directory($folder, "/\.info$/");
  foreach ($files as $file) {
    $content = file($file->uri);
    foreach ($content as $line) {
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
 *   The owner i.e. backdrop-contrib or backdrop.
 * @param string $repo
 *   The reposoitory name.
 *
 * @return string|int
 *   Either URL to project on github or 2.
 */
function get_project_link($owner, $repo) {
  $message_variables = array('!owner' => $owner, '!repo' => $repo);

  // Make sure repo exists.
  $url = 'https://github.com/' . $owner . '/' . $repo;
  $headers = get_headers($url);
  $header_parts = explode(' ', $headers[0]);
  $response = $header_parts[1];
  if ($response[0] == '4' || $response[0] == '5') {
    b_message(bt('!owner/!repo not found', $message_variables), 'error');
    return FALSE;
  }

  // Make sure download file exists.
  $url = 'https://github.com/' . $owner . '/' . $repo . '/releases/latest/download/' . $repo . '.zip';
  $headers = get_headers($url);
  $header_parts = explode(' ', $headers[0]);
  $response = $header_parts[1];
  if ($response[0] == '4' || $response[0] == '5') {
    if (!b_confirm(bt("There is no release for !owner/!repo\nDo you you want to download the *git* version instead?", $message_variables))) {
      b_message(bt('Cancelled *git* version download for !owner/!repo.', $message_variables));
      return FALSE;
    }
    return DOWNLOAD_VIA_GIT;
  }

  return $url;
}

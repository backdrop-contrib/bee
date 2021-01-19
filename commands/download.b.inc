<?php
/**
 * @file
 * Command(s) for downloading Backdrop projects.
 */

/**
 * Implements hook_b_command().
 */
function download_b_command() {
  return array(
    'download' => array(
      'description' => bt('Download Backdrop contrib projects.'),
      'callback' => 'download_b_callback',
      'arguments' => array(
        'projects' => bt('One or more contrib projects to download.'),
      ),
      'multiple_argument' => 'projects',
      'aliases' => array('dl', 'pm-download'),
      'examples' => array(
        'b download webform' => bt('Download the Webform module.'),
        'b download simplify thesis bamboo' => bt('Download the Simplify module, Thesis theme, and Bamboo layout.'),
      ),
    ),
    'download-core' => array(
      'description' => bt('Download Backdrop core.'),
      'callback' => 'download_core_b_callback',
      'arguments' => array(
        'directory' => bt('The directory to download and extract Backdrop into. Leave blank to use the current directory.'),
      ),
      'optional_arguments' => array('directory'),
      'aliases' => array('dl-core'),
      'examples' => array(
        'b download-core ../backdrop' => bt('Download Backdrop into a `backdrop` directory in the parent folder.'),
      ),
    ),
  );
}

/**
 * Command callback: Download Backdrop contrib projects.
 */
function download_b_callback($arguments, $options) {
  global $backdrop_site, $backdrop_root;

  foreach ($arguments['projects'] as $project) {
    $info = download_b_git_info($project);

    // Get the project type.
    if (empty($info['type'])) {
      b_message(bt("The 'type' of project `@project` could not be determined.", array(
        '@project' => $project,
      )), 'error');
      return;
    }
    else {
      // Add an 's' to the end of the type name.
      $info['type'] .= 's';
    }

    // Get the directory to download the project into.
    if (!empty($backdrop_site)) {
      $destination = "$backdrop_root/sites/$backdrop_site/" . $info['type'];
    }
    elseif (!empty($backdrop_root)) {
      $destination = "$backdrop_root/" . $info['type'];
    }
    else {
      b_message(bt('The download destination could not be determined. Re-run the command from within a Backdrop installation, or set the global `--root`/`--url` options.'), 'error');
      return;
    }
    if (file_exists("$destination/contrib")) {
      $description .= '/contrib';
    }
    $destination .= "/$project";
    if (file_exists($destination)) {
      b_message(bt('`@project` already exists in `@destination`.', array(
        '@project' => $project,
        '@destination' => $destination,
      )), 'error');
      return;
    }
    elseif (!mkdir($destination, 0755)) {
      b_message(bt("The destination directory `@directory` doesn't exist and couldn't be created.", array(
        '@directory' => $destination,
      )), 'error');
      return;
    }

    // Download the project.
    if (download_b_download_project($project, $info, $destination)) {
      b_message(bt('`@project` was downloaded into `@directory`.', array(
        '@project' => $project,
        '@directory' => $destination,
      )), 'success');
    }
  }
}

/**
 * Command callback: Download Backdrop core.
 */
function download_core_b_callback($arguments, $options) {
  $info = download_b_git_info('backdrop');

  // Get or create the directory to download Backdrop into.
  $destination = !empty($arguments['directory']) ? $arguments['directory'] : getcwd();
  if (!realpath($destination)) {
    if (!mkdir($destination, 0755)) {
      b_message(bt("The destination directory `@directory` doesn't exist and couldn't be created.", array(
        '@directory' => $destination,
      )), 'error');
      return;
    }
  }
  $destination = realpath($destination);

  // Make sure the directory isn't an existing Backdrop installation.
  if (b_find_root($destination)) {
    b_message(bt('Backdrop cannot be downloaded into an existing installation. Re-run the command from outside the Backdrop installation, or set the download directory using the `directory` argument.'), 'error');
    return;
  }

  // Download Backdrop.
  if (download_b_download_project('backdrop', $info, $destination)) {
    b_message(bt('Backdrop was downloaded into `@directory`.', array(
      '@directory' => $destination,
    )), 'success');
  }
}

/**
 * Get information about a project from GitHub.
 *
 * @param string $project
 *   The name of the project.
 *
 * @return array
 *   An associative array of information about the project, possibly containing:
 *   - url: The URL to download the project.
 *   - branch: The default branch of the project.
 *   - type: The 'type' of project (e.g. module, theme or layout).
 */
function download_b_git_info($project) {
  $info = array();
  $organisation = ($project == 'backdrop') ? 'backdrop' : 'backdrop-contrib';
  $api_url = 'https://api.github.com/repos';

  // Get the download URL of the latest release.
  $url = "https://github.com/$organisation/$project/releases/latest/download/$project.zip";
  $headers = get_headers($url);
  $response = substr($headers[0], 9, 3);
  if ($response < 400) {
    $info['url'] = $url;
  }
  else {
    // Offer to get the dev version instead.
    $answer = b_confirm(bt('There is no release for `@organisation/@project`. Do you you want to download the dev version instead?', array(
      '@organisation' => $organisation,
      '@project' => $project,
    )));

    if ($answer) {
      // Get the download URL of the dev version.
      $repo_info = shell_exec("curl -sS $api_url/$organisation/$project");
      $repo_info = json_decode($repo_info, TRUE);
      $branch = $repo_info['default_branch'];
      $url = "https://github.com/$organisation/$project/archive/$branch.zip";
      $headers = get_headers($url);
      $response = substr($headers[0], 9, 3);
      if ($response < 400) {
        $info['url'] = $url;
        $info['branch'] = $branch;
      }
    }
  }

  // Get the 'type' of project.
  if ($project != 'backdrop') {
    $files = shell_exec("curl -sS $api_url/$organisation/$project/contents");
    $files = json_decode($files, TRUE);
    foreach ($files as $file) {
      if ($file['type'] == 'file' && preg_match('/\.info$/i', $file['name'])) {
        $info_file = shell_exec("curl -sSH 'Accept: application/vnd.github.v3.raw' $api_url/$organisation/$project/contents/" . $file['name']);
        $lines = explode("\n", $info_file);
        foreach ($lines as $line) {
          $values = explode('=', $line);
          if (trim($values[0]) == 'type') {
            $info['type'] = trim($values[1]);
            break 2;
          }
        }
      }
    }
  }

  return $info;
}

/**
 * Download a project.
 *
 * @param string $project
 *   The name of the project to download.
 * @param array $info
 *   An associative array of information about the project from GitHub. It
 *   should contain:
 *   - url: The URL to download the project.
 *   - branch: The default branch of the project. Needed when downloading the
 *     dev version
 *   - type: The 'type' of project (e.g. module, theme or layout). Needed for
 *     contrib projects.
 * @param string $destination
 *   The path to the destination directory.
 *
 * @return boolean
 *   TRUE if the project was downloaded successfully, FALSE if not.
 */
function download_b_download_project($project, $info, $destination) {
  // Get a temp directory.
  if (!$temp = b_get_temp($project)) {
    b_message(bt('Failed to get temp directory.'), 'error');
    return FALSE;
  }

  // Get the download URL.
  if (empty($url = $info['url'])) {
    b_message(bt('The download URL for `@project` could not be found.', array(
      '@project' => $project,
    )), 'error');
    return FALSE;
  }

  // Download & extract the project.
  $file = "$temp/$project.zip";
  $directory = !empty($info['branch']) ? "$project-$branch" : $project;
  exec("wget --show-progress -qO $file $url");
  exec("unzip $file -d $temp");
  b_copy("$temp/$directory", $destination, FALSE);
  b_delete($temp);

  return TRUE;
}

<?php
/**
 * @file
 * Command(s) for downloading Backdrop projects.
 */

/**
 * Implements hook_bee_command().
 */
function download_bee_command() {
  return array(
    'download' => array(
      'description' => bt('Download Backdrop contrib projects.'),
      'callback' => 'download_bee_callback',
      'arguments' => array(
        'projects' => bt('One or more contrib projects to download.'),
      ),
      'multiple_argument' => 'projects',
      'options' => array(
        'hide-progress' => array(
          'description' => bt('Hide the download progress bar.'),
        ),
      ),
      'aliases' => array('dl', 'pm-download'),
      'examples' => array(
        'bee download webform' => bt('Download the Webform module.'),
        'bee download simplify thesis bamboo' => bt('Download the Simplify module, Thesis theme, and Bamboo layout.'),
        'bee download devel --hide-progress' => bt("Download the Devel module, and don't show the download progress bar."),
      ),
    ),
    'download-core' => array(
      'description' => bt('Download Backdrop core.'),
      'callback' => 'download_core_bee_callback',
      'arguments' => array(
        'directory' => bt('The directory to download and extract Backdrop into. Leave blank to use the current directory.'),
      ),
      'optional_arguments' => array('directory'),
      'options' => array(
        'hide-progress' => array(
          'description' => bt('Hide the download progress bar.'),
        ),
      ),
      'aliases' => array('dl-core'),
      'examples' => array(
        'bee download-core ../backdrop' => bt("Download Backdrop into a 'backdrop' directory in the parent folder."),
        'bee download-core --hide-progress' => bt("Download Backdrop into the current directory, and don't show the download progress bar."),
      ),
    ),
  );
}

/**
 * Command callback: Download Backdrop contrib projects.
 */
function download_bee_callback($arguments, $options) {
  global $_bee_backdrop_root, $_bee_backdrop_site;

  $progress = (!empty($options['hide-progress'])) ? FALSE : TRUE;

  // Get list of all core projects so we don't try to download core dependencies
  // when we download a named project.
  if (!empty($_bee_backdrop_root)) {
    // Look in folders for core modules, themes and layouts.
    $project_types = array('module', 'theme', 'layout');
    foreach ($project_types as $project_type) {
      $core_project_path = "${_bee_backdrop_root}/core/${project_type}s";
      $core_projects_raw = array();
      $core_projects_raw = glob($core_project_path . "/*", GLOB_ONLYDIR);
      // Extract the project name and add it to a single list.
      foreach ($core_projects_raw as $core_project) {
        $core_projects[] = basename($core_project);
      }
    }
    // Provide list of core projects for debug purposes.
    bee_message(bt("Core projects are: !list", array(
      '!list' => implode(",", $core_projects),
    )), 'log');
  }

  // Iterate through the list of projects manually so any dependencies can be
  // added to the list of projects to download.
  $project_count = 0;
  while ($project_count < count($arguments['projects'])) {
    $project = $arguments['projects'][$project_count];
    // Check if the project exists by trying to get the repo homepage.
    $organisation = 'backdrop-contrib';
    $url = "https://github.com/$organisation/$project";
    $headers = get_headers($url);
    if (!$headers) {
      bee_message(bt("Unable to connect to !url.", array(
        '!url' => $url,
      )), 'error');
      continue;
    }
    $response = substr($headers[0], 9, 3);
    if ($response >= 400) {
      bee_message(bt("The '!project' project repository could not be found. Please check your spelling and try again.", array(
        '!project' => $project,
      )), 'error');
    }
    else {
      $info = download_bee_git_info($project);

      // Get the list of dependencies and add to list of projects.
      if (!empty($info['dependencies'])) {
        $dependencies = $info['dependencies'];
        foreach ($dependencies as $dependency) {
          // Remove any minimum version requirements to get just the project
          // name.
          $dependency = explode(" ", $dependency, 2);
          $dependency = $dependency[0];
          // Only add contrib dependencies to the list of projects to download.
          if (!in_array($dependency, $core_projects)) {
            $arguments['projects'][] = $dependency;
            bee_message(bt("The '!dependency' module will also be downloaded, as it is required by the '!project' module.", array(
              '!dependency' => $dependency,
              '!project' => $project,
            )), 'status');
          }

        }
      }

      // Get the project type.
      if (empty($info['type'])) {
        bee_message(bt("The 'type' of project '!project' could not be determined.", array(
          '!project' => $project,
        )), 'error');
        return;
      }
      else {
        // Add an 's' to the end of the type name.
        $info['type'] .= 's';
      }

      // Get the directory to download the project into.
      if (!empty($_bee_backdrop_site)) {
        $destination = "$_bee_backdrop_root/sites/$_bee_backdrop_site/" . $info['type'];
      }
      elseif (!empty($_bee_backdrop_root)) {
        $destination = "$_bee_backdrop_root/" . $info['type'];
      }
      else {
        bee_message(bt("The download destination could not be determined. Re-run the command from within a Backdrop installation, or set the global '--root'/'--site' options."), 'error');
        return;
      }
      if (file_exists("$destination/contrib")) {
        $destination .= '/contrib';
      }
      $destination .= "/$project";
      if (file_exists($destination)) {
        bee_message(bt("'!project' already exists in '!destination'.", array(
          '!project' => $project,
          '!destination' => $destination,
        )), 'error');
        return;
      }
      elseif (!mkdir($destination, 0755, TRUE)) {
        bee_message(bt("The destination directory '!directory' doesn't exist and couldn't be created.", array(
          '!directory' => $destination,
        )), 'error');
        return;
      }

      // Download the project.
      if (download_bee_download_project($project, $info, $destination, $progress)) {
        bee_message(bt("'!project' was downloaded into '!directory'.", array(
          '!project' => $project,
          '!directory' => $destination,
        )), 'success');
      }
    }
    $project_count++;
  }
}

/**
 * Command callback: Download Backdrop core.
 */
function download_core_bee_callback($arguments, $options) {
  $info = download_bee_git_info('backdrop');
  $progress = (!empty($options['hide-progress'])) ? FALSE : TRUE;

  // Get or create the directory to download Backdrop into.
  $destination = !empty($arguments['directory']) ? $arguments['directory'] : getcwd();
  if (!realpath($destination)) {
    if (!mkdir($destination, 0755, TRUE)) {
      bee_message(bt("The destination directory '!directory' doesn't exist and couldn't be created.", array(
        '!directory' => $destination,
      )), 'error');
      return;
    }
  }
  $destination = realpath($destination);

  // Make sure the directory isn't an existing Backdrop installation.
  if (bee_find_root($destination)) {
    bee_message(bt("Backdrop cannot be downloaded into an existing installation. Re-run the command from outside the Backdrop installation, or set the download directory using the 'directory' argument."), 'error');
    return;
  }

  // Download Backdrop.
  if (download_bee_download_project('backdrop', $info, $destination, $progress)) {
    bee_message(bt("Backdrop was downloaded into '!directory'.", array(
      '!directory' => $destination,
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
function download_bee_git_info($project) {
  $info = array();
  $organisation = ($project == 'backdrop') ? 'backdrop' : 'backdrop-contrib';
  $api_url = 'https://api.github.com/repos';

  // Get the download URL of the latest release.
  $url = "https://github.com/$organisation/$project/releases/latest/download/$project.zip";
  $headers = get_headers($url);
  if (!$headers) {
    bee_message(bt("Unable to connect to !url.", array(
      '!url' => $url,
    )), 'error');
    return;
  }
  $response = substr($headers[0], 9, 3);
  if ($response < 400) {
    $info['url'] = $url;
  }
  else {
    // Offer to get the dev version instead.
    $answer = bee_confirm(bt("There is no release for '!organisation/!project'. Do you you want to download the dev version instead?", array(
      '!organisation' => $organisation,
      '!project' => $project,
    )));

    if ($answer) {
      // Get the download URL of the dev version.
      $repo_info = shell_exec("curl -sS $api_url/$organisation/$project");
      $repo_info = json_decode($repo_info, TRUE);
      $branch = $repo_info['default_branch'];
      $url = "https://github.com/$organisation/$project/archive/$branch.zip";
      $headers = get_headers($url);
      if (!$headers) {
        bee_message(bt("Unable to connect to !url.", array(
          '!url' => $url,
        )), 'error');
        return;
      }
      $response = substr($headers[0], 9, 3);
      if ($response < 400) {
        $info['url'] = $url;
        $info['branch'] = $branch;
      }
    }
  }

  // Get the 'type' and any dependencies of project.
  if ($project != 'backdrop') {
    $files = shell_exec("curl -sS $api_url/$organisation/$project/contents");
    $files = json_decode($files, TRUE);
    foreach ($files as $file) {
      if ($file['type'] == 'file' && preg_match('/\.info$/i', $file['name'])) {
        $info_file = shell_exec("curl -sSH 'Accept: application/vnd.github.v3.raw' $api_url/$organisation/$project/contents/" . $file['name']);
        $lines = explode("\n", $info_file);
        // Declare dependencies as array.
        $info['dependencies'] = array();
        foreach ($lines as $line) {
          $values = explode('=', $line);
          // Get the type of project.
          if (trim($values[0]) == 'type') {
            $info['type'] = trim($values[1]);
          }
          // Get any dependencies of project.
          if (trim($values[0]) == 'dependencies[]') {
            $info['dependencies'][] = trim($values[1]);
          }
        }
        // Exit loop as only need to check .info file.
        break;
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
 * @param boolean $progress
 *   Whether or not the download progress bar should be displayed. Defaults to
 *   TRUE.
 *
 * @return boolean
 *   TRUE if the project was downloaded successfully, FALSE if not.
 */
function download_bee_download_project($project, array $info, $destination, $progress = TRUE) {
  // Get a temp directory.
  if (!$temp = bee_get_temp($project)) {
    bee_message(bt('Failed to get temp directory.'), 'error');
    return FALSE;
  }

  // Get the download URL.
  if (empty($url = $info['url'])) {
    bee_message(bt("The download URL for '!project' could not be found.", array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }

  // Download & extract the project.
  $file = "$temp/$project.zip";
  $directory = !empty($info['branch']) ? $project . '-' . $info['branch'] : $project;
  $wget = "wget";
  if ($progress) {
    // Check the wget version so the right "show progress" argument can be used.
    $wget_version_text = shell_exec("wget --version");
    $wget_version_number = substr((string) $wget_version_text, 9, 4);
    if (version_compare((string) $wget_version_number, "1.16", ">=")) {
      $wget_older = FALSE;
      $wget_progress_argument = " --show-progress -qO";
    }
    else {
      // Versions < 1.16 - different argument and no "quiet" argument is used.
      $wget_older = TRUE;
      $wget_progress_argument = " --progress=bar -O";
    }
    $wget .= $wget_progress_argument;
  }
  else {
    $wget .= " -qO";
  }
  $wget .= " $file $url";
  exec($wget);
  exec("unzip $file -d $temp");
  bee_copy("$temp/$directory", $destination, FALSE);
  bee_delete($temp);
  ($wget_older && $progress) ? bee_message(bt("The wget tool used by bee for downloading is version !version and does not support showing only the progress bar. Upgrade wget to version 1.16 or later; or use the '--hide-progress' argument if you wish to hide altogether.", array(
    '!version' => $wget_version_number,
  )), 'status') : '';
  return TRUE;
}

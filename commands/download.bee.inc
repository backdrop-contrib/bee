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
      'group' => 'projects',
      'arguments' => array(
        'projects' => bt('One or more contrib projects to download.'),
      ),
      'multiple_argument' => 'projects',
      'options' => array(
        'hide-progress' => array(
          'description' => bt('Deprecated, will get removed in a future version.'),
          'short' => 'h',
        ),
        'allow-multisite-copy' => array(
          'description' => bt('Override the check that would prevent the project being downloaded to a multisite site if the project exists in the shared project directory.'),
          'short' => 'f',
        ),
      ),
      'aliases' => array('dl', 'pm-download'),
      'examples' => array(
        'bee download webform' => bt('Download the Webform module.'),
        'bee download simplify thesis bamboo' => bt('Download the Simplify module, Thesis theme, and Bamboo layout.'),
        'bee --site=site_name download simplify --allow-multisite-copy' => bt('Download an additional copy of the Simplify module into the site_name multisite module folder.'),
      ),
    ),
    'download-core' => array(
      'description' => bt('Download Backdrop core.'),
      'callback' => 'download_core_bee_callback',
      'group' => 'core',
      'arguments' => array(
        'directory' => bt('The directory to download and extract Backdrop into. Leave blank to use the current directory.'),
      ),
      'optional_arguments' => array('directory'),
      'options' => array(
        'hide-progress' => array(
          'description' => bt('Deprecated, will get removed in a future version.'),
          'short' => 'h',
        ),
      ),
      'aliases' => array('dl-core'),
      'examples' => array(
        'bee download-core ../backdrop' => bt("Download Backdrop into a 'backdrop' directory in the parent folder."),
      ),
    ),
  );
}

/**
 * Command callback: Download Backdrop contrib projects.
 */
function download_bee_callback($arguments, $options) {
  global $_bee_backdrop_root, $_bee_backdrop_site;

  $allow_multisite_copy = (!empty($options['allow-multisite-copy'])) ? TRUE : FALSE;

  // Get list of all core projects so we don't try to download core dependencies
  // when we download a named project.
  if (!empty($_bee_backdrop_root)) {
    // Look in folders for core modules, themes and layouts.
    $project_types = array('module', 'theme', 'layout');
    foreach ($project_types as $project_type) {
      $core_project_path = "{$_bee_backdrop_root}/core/{$project_type}s";
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
          // Check if the dependency is a core project.
          $dependency_is_core = in_array($dependency, $core_projects);
          // Check if dependency exists in the site file system. Only
          // check contrib dependencies.
          if (!$dependency_is_core) {
            // Get information about the dependency project.
            $dependency_info = download_bee_git_info($dependency);
            // Check whether the dependency exists within the site file system.
            $dependency_existing_location = download_bee_check_project_exists($dependency, $dependency_info['type']);
            $dependency_allow_multisite_copy = download_bee_check_multisite_copy($allow_multisite_copy, $dependency_existing_location);
            if ($dependency_existing_location == FALSE || $dependency_allow_multisite_copy) {
              // If project does not exist in the site file system, or it is
              // but a multisite copy is allowed, then check whether it is
              // already in the download list.
              if (in_array($dependency, $arguments['projects'])) {
                // If project is already in the list to download, do not add to
                // the list to download again.
                bee_message(bt("The '!dependency' !dependency_type is also required by the '!project' !project_type.", array(
                  '!dependency' => $dependency,
                  '!dependency_type' => $dependency_info['type'],
                  '!project' => $project,
                  '!project_type' => $info['type'],
                )), 'status');
              }
              else {
                // If project is not already in download list, then add to list
                // and prepare message.
                $arguments['projects'][] = $dependency;
                bee_message(bt("The '!dependency' !dependency_type will also be downloaded, as it is required by the '!project' !project_type.", array(
                  '!dependency' => $dependency,
                  '!dependency_type' => $dependency_info['type'],
                  '!project' => $project,
                  '!project_type' => $info['type'],
                )), 'status');
              }
            }
            else {
              // If project does exist in the site file system, give a
              // meaningful message so user is aware of the dependency and that
              // it is already met.
              bee_message(bt("The '!dependency' !dependency_type is required by the '!project' !project_type but already exists at '!dependency_location'.", array(
                '!dependency' => $dependency,
                '!dependency_type' => $dependency_info['type'],
                '!project' => $project,
                '!project_type' => $info['type'],
                '!dependency_location' => $dependency_existing_location,
              )), 'status');
            }
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
        $type_folder = $info['type'] . 's';
      }

      // Get the directory to download the project into.
      if (!empty($_bee_backdrop_site)) {
        $destination = "$_bee_backdrop_root/sites/$_bee_backdrop_site/" . $type_folder;
      }
      elseif (!empty($_bee_backdrop_root)) {
        $destination = "$_bee_backdrop_root/" . $type_folder;
      }
      else {
        bee_message(bt("The download destination could not be determined. Re-run the command from within a Backdrop installation, or set the global '--root'/'--site' options."), 'error');
        return;
      }
      if (file_exists("$destination/contrib")) {
        $destination .= '/contrib';
      }
      $destination .= "/$project";
      // Check if the project exists within the site file system.
      $project_existing_location = download_bee_check_project_exists($project, $info['type']);
      $project_allow_multisite_copy = download_bee_check_multisite_copy($allow_multisite_copy, $project_existing_location);
      if ($project_existing_location != FALSE && !$project_allow_multisite_copy) {
        bee_message(bt("'!project' already exists in '!existing_location'.", array(
          '!project' => $project,
          '!existing_location' => $project_existing_location,
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
      if (download_bee_download_project($project, $info, $destination)) {
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
  if (download_bee_download_project('backdrop', $info, $destination)) {
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
      $repo_info = FALSE;
      if ($curl_handle = curl_init("$api_url/$organisation/$project")) {
        curl_setopt($curl_handle, CURLOPT_USERAGENT, 'Bee');// github nag @todo constant
        curl_setopt($curl_handle, CURLOPT_HEADER, 0);
        curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);
        $repo_info = curl_exec($curl_handle);
        curl_close($curl_handle);
        $repo_info = json_decode($repo_info, TRUE);
      }
      if (!is_array($repo_info)) {
        bee_message(bt('Unable to fetch repo information.'), 'error');
        return;
      }
      // Get the download URL of the dev version.
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
    $files = FALSE;
    if ($curl_handle = curl_init("$api_url/$organisation/$project/contents")) {
      curl_setopt($curl_handle, CURLOPT_USERAGENT, 'Bee');
      curl_setopt($curl_handle, CURLOPT_HEADER, 0);
      curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);
      $files = curl_exec($curl_handle);
      curl_close($curl_handle);
      $files = json_decode($files, TRUE);
    }
    if (!is_array($files)) {
      bee_message(bt('Unable to fetch file information.'), 'error');
      return;
    }
    foreach ($files as $file) {
      if ($file['type'] == 'file' && preg_match('/\.info$/i', $file['name'])) {
        $curl_handle = curl_init("$api_url/$organisation/$project/contents/" . $file['name']);
        curl_setopt($curl_handle, CURLOPT_USERAGENT, 'Bee');
        curl_setopt($curl_handle, CURLOPT_HEADER, 0);
        curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($curl_handle, CURLOPT_HTTPHEADER, array('Accept: application/vnd.github.v3.raw'));
        $info_file = curl_exec($curl_handle);
        curl_close($curl_handle);
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
 *
 * @return boolean
 *   TRUE if the project was downloaded successfully, FALSE if not.
 */
function download_bee_download_project($project, array $info, $destination) {
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

  $file_handle = fopen($file, 'w');
  $curl_handle = curl_init($url);
  curl_setopt($curl_handle, CURLOPT_FILE, $file_handle);
  curl_setopt($curl_handle, CURLOPT_USERAGENT, 'Bee');
  curl_setopt($curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
  curl_exec($curl_handle);
  curl_close($curl_handle);
  fclose($file_handle);

  $zip = new ZipArchive;
  if ($zip->open($file)) {
    $zip->extractTo($temp);
    $zip->close();
  }
  else {
    bee_message(bt('Unable to open zip file.'), 'error');
    return FALSE;
  }
  bee_copy("$temp/$directory", $destination, FALSE);
  bee_delete($temp);

  return TRUE;
}

/**
 * Get list of all projects in filesystem.
 *
 * @param string $type
 *   The type of project to list:
 *    - module
 *    - theme
 *    - layout
 *   This will find projects in both core and contrib.
 *
 * @return array
 *   An associative array of all projects with their locations, together with
 *   the location type and project type.
 */
function download_bee_find_all_projects($type) {
  global $_bee_backdrop_root, $_bee_backdrop_site;
  // Create an empty array for files.
  $files = array();
  // Set the pattern to look for .info files.
  $pattern = '#\.info$#';
  // Define the locations to search.
  $locations = array('core','contrib');
  (empty($_bee_backdrop_site)) ?: $locations[] = 'site_contrib';
  // Loop through each location.
  foreach ($locations as $location) {
    // Set the path depending on location.
    switch ($location) {
      case 'core':
        $path = "{$_bee_backdrop_root}/core/{$type}s";
        break;
      case 'contrib':
        $path = "{$_bee_backdrop_root}/{$type}s";
        break;
      case 'site_contrib':
        $path = "{$_bee_backdrop_root}/sites/{$_bee_backdrop_site}/{$type}s";
        break;
      default:
        $path = $location;
        break;
    }
    // Scan the location recursively for projects.
    $options = array(
      'key' => 'name',
    );
    $files = bee_file_scan_directory($path, $pattern, $options);
    // Modify each record to meet our needs.
    $projects[] = array();
    foreach ($files as &$file) {
      // Add the revised project record to a common list of projects.
      $key = $file->name;
      $projects[$key] = array(
        'name' => $file->name,
        'uri' => $file->uri,
        'location_category' => $location,
        'type' => $type,
      );
    }
  }
  return $projects;
}

/**
 * Check if a project exists in the site filesystem.
 *
 * @param string $project
 *   The name of the project.
 * @param string $type
 *   The type of the project.
 *
 * @return string|false
 *   The project location, or FALSE if not found.
 */
function download_bee_check_project_exists($project, $type) {
  // Get the list of all projects for the type.
  $projects = download_bee_find_all_projects($type);
  if (array_key_exists($project, $projects)) {
    // If project exists, return the path where it is located.
    return dirname($projects[$project]['uri']);
  }
  else {
    // If project does not exist, return FALSE.
    return FALSE;
  }
}

/**
 * Check if download for multisite can be downloaded if it exists in the
 * root directory for the same project type.
 *
 * @param boolean $allow_multisite_copy
 *   Whether or not to allow multisite copy.
 * @param string $existing_location_path
 *   The path of the existing project.
 *
 * @return boolean
 *   TRUE if the download for multisite can be downloaded. FALSE if not.
 */
function download_bee_check_multisite_copy($allow_multisite_copy, $existing_location_path) {
  global $_bee_backdrop_site;
  // Check if site is defined for this download.
  if (!empty($_bee_backdrop_site)) {
    // If site defined then check if existing location is in site folder.
    $existing_in_site_folder = stripos($existing_location_path, $_bee_backdrop_site) > 0;
    if (!$existing_in_site_folder && $allow_multisite_copy) {
      // If existing is in root directory and the option to allow multisite
      // copy is included, then we can download to site directory.
      return TRUE;
    }
    else {
      // If either the existing copy of the project is in the site directory OR
      // the option to allow multisite copy is NOT included, then we should not
      // allow a download to the site directory.
      return FALSE;
    }
  }
  else {
    // If no site defined then this check should always return FALSE.
    return FALSE;
  }
}

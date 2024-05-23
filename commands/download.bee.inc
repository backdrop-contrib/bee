<?php
/**
 * @file
 * Command(s) for downloading Backdrop projects.
 */

/**
 * The useragent string used for HTTP requests via cURL.
 */
define('BEE_USERAGENT', 'Bee - the command line tool for Backdrop CMS');

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
        'github-token' => array(
          'description' => bt('A Github Personal Access Token (Classic) that can be used to extend the GitHub API rate.'),
          'value' => bt('The token.'),
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
        'github-token' => array(
          'description' => bt('A Github Personal Access Token (Classic) that can be used to extend the GitHub API rate.'),
          'value' => bt('The token.'),
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

  // Set the GitHub Token if entered.
  $github_api_token = $options['github-token'] ?? '';

  // Estimate the number of API calls. We need TWO for each project. We also
  // need TWO for each dependency but we cannot know until we get the info file
  // for each project and also check which dependencies already exist in the
  // file system. We therefore will use a multiplier of THREE on the number of
  // projects entered at the command line to try and ensure we have enough
  // quota to complete. This won't always work but we will catch exceptions
  // before each call.
  $api_call_estimate = count($arguments['projects']) * 3;

  // Check GitHub API quota to ensure we can complete.
  if (!check_github_api_quota($github_api_token, $api_call_estimate)) {
    return;
  }

  // Get list of all core projects so we don't try to download core dependencies
  // when we download a named project.
  $core_project_list = array();
  if (!empty($_bee_backdrop_root)) {
    // Look in folders for core modules, themes and layouts.
    $project_types = array('module', 'theme', 'layout');
    foreach ($project_types as $project_type) {
      $locations = array('core');
      $core_projects_set = download_bee_find_all_projects($project_type, $locations);

      // Add each project to a common array.
      foreach ($core_projects_set as $key => $core_project) {
        $core_projects[$key] = $core_project;
        // Create a simple array of names for debug purposes.
        $core_project_list[] = $key;
      }
    }
    bee_message(bt("Core projects are: !list", array(
      '!list' => implode(",", $core_project_list),
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
      $info = download_bee_git_info($project, $github_api_token);
      if (empty($info)) {
        // If getting the info file has failed, show an error message here and
        // exit.
        bee_message(bt("The info file for !project cannot be retrieved at this time.", array(
          '!project' => $project,
        )), 'error');
        return;
      }
      // Get the list of dependencies and add to list of projects.
      if (!empty($info['dependencies'])) {
        $dependencies = $info['dependencies'];
        foreach ($dependencies as $dependency) {
          // Remove any minimum version requirements to get just the project
          // name.
          $dependency = explode(" ", $dependency, 2);
          $dependency = $dependency[0];
          // Check if the dependency is a core project.
          $dependency_is_core = isset($core_projects[$dependency]);
          // Check if dependency exists in the site file system. Only
          // check contrib dependencies.
          if (!$dependency_is_core) {
            // Get information about the dependency project.
            $dependency_info = download_bee_git_info($dependency, $github_api_token);
            if (empty($dependency_info)) {
              // If getting the info file has failed, show an error message here
              // and exit.
              bee_message(bt("!dependency is a dependency of !project but the info file for !dependency cannot be retrieved at this time.", array(
                '!project' => $project,
                '!dependency' => $dependency,
              )), 'error');
              continue;
            }
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
  // Set the GitHub Token if entered.
  $github_api_token = $options['github-token'] ?? '';

  // Estimate the number of API calls. Only 1 is needed for Backdrop Core.
  $api_call_estimate = 1;

  // Check GitHub API quota to ensure we can complete.
  if (!check_github_api_quota($github_api_token, $api_call_estimate)) {
    return;
  }

  $info = download_bee_git_info('backdrop', $github_api_token);

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
 * @param string $github_api_token
 *   The GitHub API Personal Access Token to apply to API calls.
 *
 * @return array
 *   An associative array of information about the project, possibly containing:
 *   - url: The URL to download the project.
 *   - branch: The default branch of the project.
 *   - type: The 'type' of project (e.g. module, theme or layout).
 */
function download_bee_git_info($project, $github_api_token = '') {
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
      // Check GitHub API quota to ensure we can continue.
      if (!check_github_api_quota($github_api_token)) {
        return;
      }
      if ($curl_handle = curl_init("$api_url/$organisation/$project")) {
        // Prepare headers for curl request.
        $curlopt_httpheader = array();
        $curlopt_httpheader[] = 'Content-Type: application/json';
        // If a token has been passed, add this to the header.
        if (!empty($github_api_token)) {
          $curlopt_httpheader[] = "Authorization: token $github_api_token";
        }
        curl_setopt($curl_handle, CURLOPT_HTTPHEADER, $curlopt_httpheader);
        curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
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
    // Check GitHub API quota to ensure we can continue.
    if (!check_github_api_quota($github_api_token)) {
      return;
    }
    if ($curl_handle = curl_init("$api_url/$organisation/$project/contents")) {
      // If a token has been passed, add this to the header.
      if (!empty($github_api_token)) {
        curl_setopt($curl_handle, CURLOPT_HTTPHEADER, array(
          'Content-Type: application/json',
          "Authorization: token $github_api_token",
        ));
      }
      curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
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
        // Check GitHub API quota to ensure we can continue.
        if (!check_github_api_quota($github_api_token)) {
          return;
        }
        $curl_handle = curl_init("$api_url/$organisation/$project/contents/" . $file['name']);
        // Prepare headers for curl request.
        $curlopt_httpheader = array();
        $curlopt_httpheader[] = 'Accept: application/vnd.github.v3.raw';
        // If a token has been passed, add this to the header.
        if (!empty($github_api_token)) {
          $curlopt_httpheader[] = "Authorization: token $github_api_token";
        }
        curl_setopt($curl_handle, CURLOPT_HTTPHEADER, $curlopt_httpheader);
        curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
        curl_setopt($curl_handle, CURLOPT_HEADER, 0);
        curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);

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
  curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
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
 *    - layout.
 * @param array $locations
 *   Array of 'locations'. These are not path locations, but location types. By
 *   default, this is 'core' and 'contrib', but you could limit to just 'core'
 *   or just 'contrib'.
 *
 * @return array
 *   An associative array of all projects with their locations, together with
 *   the location type and project type.
 */
function download_bee_find_all_projects($type,
  array $locations = array('core', 'contrib')) {
  global $_bee_backdrop_root, $_bee_backdrop_site;
  // Create an empty array for files.
  $files = array();
  // Create an empty array for projects.
  $projects = array();
  // Set the pattern to look for .info files.
  $pattern = '#\.info$#';
  // Check if we need to add 'site_contrib' to list of locations.
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

/**
 * Get quota information for the GitHub API anonymous user or token.
 *
 * @param string $github_api_token
 *   The GitHub Personal Access Token to check. Anonymous if left blank.
 * @param int $minimum_quota
 *   If it is known (or estimated) how much quota is needed then we can pass
 *   that number to the function to check. Otherwise it will return true if
 *   there is more than zero.
 *
 * @return bool
 *   If there
 */
function check_github_api_quota($github_api_token = '', int $minimum_quota = 1) {
  // Try to initiate a curl connection to the GitHub Rate Limit API. This does
  // not use any quota.
  $is_token = FALSE;
  if ($curl_handle = curl_init("https://api.github.com/rate_limit")) {
    // Prepare headers for curl request.
    $curlopt_httpheader = array();
    $curlopt_httpheader[] = 'Content-Type: application/json';
    // If a token has been passed, add this to the header.
    if (!empty($github_api_token)) {
      $curlopt_httpheader[] = "Authorization: token $github_api_token";
      $is_token = TRUE;
    }
    curl_setopt($curl_handle, CURLOPT_HTTPHEADER, $curlopt_httpheader);
    // Set the other curl options.
    curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
    curl_setopt($curl_handle, CURLOPT_HEADER, 0);
    curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);
    // Get the rate information.
    $data = curl_exec($curl_handle);
    curl_close($curl_handle);
    $data = json_decode($data, TRUE);
    if (empty($data)) {
      bee_message(bt('GitHub API Rate Limit information cannot be retrieved at this time. Please try again later.'), 'error');
      return FALSE;
    }
    // Return the core quota details.
    $rate_info = $data['resources']['core'];
    $reset_time = date('H:m T', $rate_info['reset']);
    $used = $rate_info['used'];
    $remaining = $rate_info['remaining'];
    $quota = $rate_info['limit'];
    // Create a debug message with details about the quota.
    bee_message(bt('GitHub API Rate Limit: !used/!quota used with !remaining remaining. Quota will reset at !reset_time.', array(
      '!used' => $used,
      '!remaining' => $remaining,
      '!quota' => $quota,
      '!reset_time' => $reset_time,
    )), 'log');
    // Check the amount remaining.
    if ($remaining < $minimum_quota) {
      if ($remaining == 0) {
        // If there is no quota remaining, provide a meaningful error message.
        bee_message(bt('You have reached your rate limit (!quota/hour) for the GitHub API. Your quota will reset at !reset_time.', array(
          '!quota' => $quota,
          '!reset_time' => $reset_time,
        )), 'error');
      }
      else {
        // If there is some quota remaining, but not enough for the estimated
        // number of calls, provide a meaningful error message.
        bee_message(bt('You have nearly reached your rate limit (!quota/hour) for the GitHub API and it is estimated that the amount remaining will not be enough to complete this operation. Your quota will reset at !reset_time.', array(
          '!quota' => $quota,
          '!reset_time' => $reset_time,
        )), 'error');
      }
      // If there is not a token in use, then also provide an information
      // message about using a token.
      if (!$is_token) {
        bee_message(bt('The GitHub API rate limit is 60/hour for anonymous use. You can increase this by using a token. See the help or wiki for more information.'), 'info');
      }
      return FALSE;
    }
    else {
      // There is still enough quota remaining.
      return TRUE;
    }
  }
  // If we can't initiate the curl connection then return FALSE.
  return FALSE;
}

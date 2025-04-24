<?php
/**
 * @file
 * Command(s) for downloading Backdrop projects.
 */

/**
 * The API URL used for GitHub API calls.
 */
define('BEE_GITHUB_API_URL', 'https://api.github.com/');

/**
 * Implements hook_bee_command().
 */
function download_bee_command() {
  $projects_description = bt('One or more contrib projects to download. You can specify a release tag or branch for each project using the format:');
  $projects_description .= "\n  " . bt('project:release_tag/keyword[:branch]');
  $projects_description .= "\n\n " . bt("Specify a release tag or one of the following keywords:");
  $projects_description .= "\n  - " . bt("'dev' (download the dev version from the default branch.)");
  $projects_description .= "\n  - " . bt("'branch' (download the dev version from an alternative branch. Specified with ':branch_name'.");
  $projects_description .= "\n  - " . bt("'select' (a list of valid options will be offered including dev and all releases that are not draft. Latest and pre-releases will be labelled.)");
  $projects_description .= "\n\n " . bt("If 'branch' is entered for release, then the name of an alternative branch can be entered with this option. It is ignored otherwise.");
  $projects_description .= "\n\n " . bt("By default, 'latest' is selected if no release is specified. Dependencies will all download the 'latest' so if you do want a different version, download these first.");

  return array(
    'download' => array(
      'description' => bt('Download Backdrop contrib projects.'),
      'callback' => 'download_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'projects',
      'arguments' => array(
        'projects' => $projects_description,
      ),
      'multiple_argument' => 'projects',
      'options' => array(
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
        'bee download webform' => bt('Download the latest version of the Webform module.'),
        'bee download simplify thesis bamboo' => bt('Download the Simplify module, Thesis theme, and Bamboo layout.'),
        'bee --site=site_name download simplify --allow-multisite-copy' => bt('Download an additional copy of the Simplify module into the site_name multisite module folder.'),
        'bee download simplify:dev' => bt('Download the dev version of the Simplify module.'),
        'bee download paragraphs:branch:1.x-1.2' => bt('Download the 1.x-1.2 branch of the Paragraphs module'),
        'bee download simplify:1.x-1.2.4' => bt('Download the 1.x-1.2.4 release of the Simplify module'),
        'bee download simplify:select' => bt('Select from a list of releases and the dev version for the Simplify module.'),
      ),
    ),
    'download-core' => array(
      'description' => bt('Download Backdrop core.'),
      'callback' => 'download_core_bee_callback',
      'command_requirements' => array(
        'backdrop_root' => FALSE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'core',
      'arguments' => array(
        'directory' => bt('The directory to download and extract Backdrop into. Leave blank to use the current directory.'),
      ),
      'optional_arguments' => array('directory'),
      'options' => array(
        'version' => array(
          'description' => bt("Specify a release tag or one of the following keywords:")
          . "\n - " . bt("'dev' (download the dev version from the default branch.)")
          . "\n - " . bt("'branch' (download the dev version from an alternative branch. Specified by appending ':branch_name')")
          . "\n - " . bt("'select' (a list of valid options will be offered including dev and all releases that are not draft. Latest and pre-releases will be labelled.)"),
          'value' => bt("The release tag or keyword."),
        ),
        'github-token' => array(
          'description' => bt('A Github Personal Access Token (Classic) that can be used to extend the GitHub API rate.'),
          'value' => bt('The token.'),
        ),
      ),
      'aliases' => array('dl-core'),
      'examples' => array(
        'bee download-core ../backdrop' => bt("Download Backdrop into a 'backdrop' directory in the parent folder."),
        'bee download-core --version=1.28.3' => bt("Download the 1.28.3 release of Backdrop into the current directory."),
        'bee download-core --version=dev' => bt("Download the dev version of Backdrop into the current directory."),
        'bee download-core --version=branch:1.29.x' => bt("Download the 1.29.x branch of Backdrop into the current directory."),
        'bee download-core --version=select' => bt("Select from a list of releases of Backdrop to download into the current directory."),
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
  if (!download_bee_check_github_api_quota($github_api_token, $api_call_estimate)) {
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
  $projects = $arguments['projects'];
  while ($project_count < count($projects)) {
    $project = $projects[$project_count];
    $project_details = explode(':', $project);
    bee_instant_message('$project_details: ', 'debug', $project_details);
    $detail_count = count($project_details);
    switch ($detail_count) {
      case 1:
        $project = array(
          'name' => $project_details[0],
          'release' => '',
          'branch' => '',
        );
        break;
      case 2:
        $project = array(
          'name' => $project_details[0],
          'release' => $project_details[1],
          'branch' => '',
        );
        break;
      case 3:
        $project = array(
          'name' => $project_details[0],
          'release' => $project_details[1],
        );
        $project['branch'] = ($project['release'] == 'branch') ? $project_details[2] : '';
    }

    // Check if the project exists within the site file system. It is desirable
    // to do this check before doing checks about what version to download. At
    // this stage, we don't know if it is a module, theme or layout template so
    // we need to check each but can exit if found.
    $types = array('module', 'theme', 'layout');
    foreach ($types as $type) {
      $project_existing_location = download_bee_check_project_exists($project['name'], $type);
      $project_allow_multisite_copy = download_bee_check_multisite_copy($allow_multisite_copy, $project_existing_location);
      if ($project_existing_location != FALSE && !$project_allow_multisite_copy) {
        bee_message(bt("'!project' already exists in '!existing_location'.", array(
          '!project' => $project['name'],
          '!existing_location' => $project_existing_location,
        )), 'error');
        $project_count++;
        continue 2;
      }
    }

    // Check if the project exists by trying to get the repo homepage.
    $organization = 'backdrop-contrib';
    $url = "https://github.com/$organization/" . $project['name'];
    if (!download_bee_check_url_exists($url)) {
      bee_message(bt("The '!project' project repository could not be found. Please check your spelling and try again.", array(
        '!project' => $project['name'],
      )), 'error');
    }
    else {
      $info = download_bee_get_project_info($project['name'], $organization, $project['release'], $project['branch'], $github_api_token);
      if ($info === FALSE) {
        $project_count++;
        continue;
      }
      if (empty($info)) {
        // If getting the info file has failed, show an error message here and
        // exit.
        bee_message(bt("The info file for !project cannot be retrieved at this time.", array(
          '!project' => $project['name'],
        )), 'error');
        $project_count++;
        continue;
      }
      // Get the list of dependencies and add to list of projects.
      if (!empty($info['dependencies'])) {
        $dependencies = $info['dependencies'];
        foreach ($dependencies as $dependency_detail) {
          $submodule = '';

          // Check whether the dependency is a submodule.
          if (isset($dependency_detail['project'])) {
            // If so, get the submodule.
            $submodule = $dependency_detail['name'];
            // Get the parent module.
            $dependency = $dependency_detail['project'];
          }
          else {
            // If not, get the module name.
            $dependency = $dependency_detail['name'];
          }
          // For now, ignoring the version part of dependencies as in most
          // cases we will be getting the latest versions.
          // Check if the dependency is a core project.
          $dependency_is_core = isset($core_projects[$dependency]);
          // Check if dependency exists in the site file system. Only
          // check contrib dependencies.
          if (!$dependency_is_core) {
            // Get information about the dependency project.
            $dependency_info = download_bee_get_project_info($dependency, $organization, '', '', $github_api_token);
            if ($dependency_info === FALSE) {
              continue;
            }
            if (empty($dependency_info)) {
              // If getting the info file has failed, show an error message here
              // and exit.
              bee_message(bt("!dependency is a dependency of !project but the info file for !dependency cannot be retrieved at this time.", array(
                '!project' => $project['name'],
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
              if (in_array($dependency, $projects)) {
                // If project is already in the list to download, do not add to
                // the list to download again.
                bee_instant_message(bt("The '!dependency' !dependency_type is also required by the '!project' !project_type.", array(
                  '!dependency' => !empty($submodule) ? "$dependency:$submodule" : $dependency,
                  '!dependency_type' => (!empty($submodule) ? 'sub' : '') . $dependency_info['type'],
                  '!project' => $project['name'],
                  '!project_type' => $info['type'],
                )), 'status');
              }
              else {
                // If project is not already in download list, then add to list
                // and prepare message.
                $projects[] = $dependency;
                bee_instant_message(bt("The '!dependency' !dependency_type will also be downloaded, as it is required by the '!project' !project_type.", array(
                  '!dependency' => !empty($submodule) ? "$dependency:$submodule" : $dependency,
                  '!dependency_type' => (!empty($submodule) ? 'sub' : '') . $dependency_info['type'],
                  '!project' => $project['name'],
                  '!project_type' => $info['type'],
                )), 'status');
              }
            }
            else {
              // If project does exist in the site file system, give a
              // meaningful message so user is aware of the dependency and that
              // it is already met.
              bee_instant_message(bt("The '!dependency' !dependency_type is required by the '!project' !project_type but already exists at '!dependency_location'.", array(
                '!dependency' => $dependency,
                '!dependency_type' => $dependency_info['type'],
                '!project' => $project['name'],
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
          '!project' => $project['name'],
        )), 'error');
        $project_count++;
        continue;
      }

      // Check that the destination can be created.
      $destination = download_bee_get_destination_path($project['name'], $info['type']);
      bee_instant_message('$destination:' . $destination, 'debug');
      if (!mkdir($destination, 0755, TRUE)) {
        bee_message(bt("The destination directory '!directory' doesn't exist and couldn't be created.", array(
          '!directory' => $destination,
        )), 'error');
        return;
      }

      // Download the project.
      if (download_bee_download_project($project['name'], $info['download_url'], $destination, $info['branch'])) {
        // If the download is successful then display an appropriate message.
        // Check if a release.
        if (isset($info['release'])) {
          // If download is a release, whether latest or defined, show the
          // release tag and link to release notes.
          bee_instant_message('release data: ', 'debug', $info['release']);
          bee_message(bt("'!project' (!release, published at !release_date) was downloaded into '!directory'. For more information see !url", array(
            '!project' => $project['name'],
            '!release' => $info['release']['tag_name'],
            '!release_date' => $info['release']['published_at'],
            '!url' => $info['release']['release_url'],
            '!directory' => $destination,
          )), 'success');
        }
        else {
          // If download is not a release it must be a branch so we should show
          // the branch name.
          bee_message(bt("'!project' (!branch) was downloaded into '!directory'.", array(
            '!project' => $project['name'],
            '!branch' => $info['branch'],
            '!directory' => $destination,
          )), 'success');
        }

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

  // Get the version details if entered.
  $version = $options['version'] ?? '';

  if (!empty($version)) {
    $version_details = explode(':', $version);
    bee_instant_message('$version_details: ', 'debug', $version_details);
    $detail_count = count($version_details);
    switch ($detail_count) {
      case 1:
        $core_version = array(
          'release' => $version_details[0],
          'branch' => '',
        );
        break;
      case 2:
        $core_version = array(
          'release' => $version_details[0],
        );
        $core_version['branch'] = ($core_version['release'] == 'branch') ? $version_details[1] : '';
    }
  }
  else {
    // If version is not specified then set $core_version accordingly.
    $core_version = array(
      'release' => '',
      'branch' => '',
    );
  }
  // Estimate the number of API calls. Only 1 is needed for Backdrop Core.
  $api_call_estimate = 1;

  // Check GitHub API quota to ensure we can complete.
  if (!download_bee_check_github_api_quota($github_api_token, $api_call_estimate)) {
    return;
  }

  $info = download_bee_get_project_info('backdrop', 'backdrop', $core_version['release'], $core_version['branch'], $github_api_token);

  if (!$info) {
    bee_message(bt("Backdrop Core: Download operation is cancelled."), 'info');
    return;
  }

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
  if (download_bee_download_project('backdrop', $info['download_url'], $destination, $info['branch'])) {
    // If the download is successful then display an appropriate message.
    // Check if a release.
    if (isset($info['release'])) {
      // If download is a release, whether latest or defined, show the
      // release tag and link to release notes.
      bee_message(bt("Backdrop (!release, published at !release_date) was downloaded into '!directory'. For more information see !url", array(
        '!release' => $info['release']['tag_name'],
        '!url' => $info['release']['release_url'],
        '!release_date' => $info['release']['published_at'],
        '!directory' => $destination,
      )), 'success');
    }
    else {
      // If download is not a release it must be a branch so we should show
      // the branch name.
      bee_message(bt("Backdrop (!branch) was downloaded into '!directory'.", array(
        '!branch' => $info['branch'],
        '!directory' => $destination,
      )), 'success');
    }
  }
}

/**
 * Get required information about the project.
 *
 * @param string $project
 *   The name of the project.
 * @param string $organization
 *   The name of the GitHub organization.
 * @param string $release
 *   (Optional) A string relating to a release option. Defaults to empty string
 *   (''):
 *   - 'dev': Download the dev version from the default branch.
 *   - 'branch': Download the dev version from an alternative branch. Specified
 *     using the $branch parameter.
 *   - 'latest': Downloads the latest release that is neither a draft nor a
 *     pre-release. This will be the default anyway if the parameter is left
 *     blank. If there is not a valid latest release, the dev branch will be
 *     offered (see 'dev').
 *   - '1.x-1.2.3': The tag of a given release. If the release cannot be found
 *     an error message will be given and the function will return FALSE.
 *   - 'select': A list of valid options will be offered including:
 *      - dev: (see dev).
 *      - All releases that are not draft. Latest will be labelled and
 *        pre-releases will also be labelled.
 * @param string $branch
 *   (Optional) A string containing the name of the branch. Defaults to empty
 *   string ('').
 * @param string $github_api_token
 *   (Optional) The GitHub API Personal Access Token to apply to API calls.
 *   Defaults to empty string ('').
 *
 * @return array|false
 *   An associative array of information about the project, including all
 *   relevant information:
 *   - 'download_url': The URL to download the project. Always included if
 *   request is successful.
 *   - 'type': the project type (i.e. 'module', 'theme' or 'layout'). Always
 *     included if request is successful.
 *   - 'branch': The branch of the project to be downloaded. Included if the
 *     default or another branch is used. Defaults to blank string if not used.
 *   - 'release': Associative array of key release information:
 *     - 'tag_name': the tag name selected.
 *     - 'published_at': the date the release is published.
 *     - 'release_url': the URL of the release page on GitHub.
 *     Included if a release is specified or selected from the select screen.
 *   - 'dependencies': An array of dependency lines. Defaults to blank array if
 *     no dependencies.
 */
function download_bee_get_project_info($project, $organization, $release = '', $branch = '', $github_api_token = '') {

  // Define the base for all GitHub API calls we may use in this function.
  $endpoint_base = "repos/$organization/$project";

  // Select the actions based on the release information entered.
  switch ($release) {
    case 'latest':
      // If 'latest' is explicitly entered, it is the same as if release is
      // left blank.
    case '':
      $preferred_option = 'latest';
      // Check that a valid latest release exists. For GitHub to count a
      // release as 'latest' it must be published (i.e. not draft) and not a
      // pre-release.
      if (download_bee_check_latest_release_exists($project, $organization)) {
        $final_option = 'latest';
      }
      else {
        // If the URL does not exist, offer to get the dev version instead.
        $choice = bee_confirm(bt("There is no valid latest release for '!organization/!project'. Do you want to download the dev version instead?", array(
          '!organization' => $organization,
          '!project' => $project,
        )), TRUE);
        if ($choice) {
          // If 'Yes' mode was enabled or user responded 'Yes', then continue
          // with dev version.
          $final_option = 'dev';
        }
        else {
          // If 'Yes' mode was not enabled and user responded 'No', then provide
          // a message and return FALSE.
          bee_message(bt("!project: Download operation is cancelled.", array(
            '!project' => $project,
          )), 'info');
          return FALSE;
        }
      }
      break;
    case 'dev':
      $preferred_option = 'dev';
      $final_option = 'dev';
      break;
    case 'branch':
      $preferred_option = 'branch';
      // Check that a branch has been specified and that it exists.
      $branch_specified = !empty($branch);
      if ($branch_specified ) {
        $branch_url = "https://github.com/$organization/$project/tree/$branch";
        $check_branch_url = download_bee_check_url_exists($branch_url);
      }
      if ($branch_specified && $check_branch_url) {
        // If both conditions are satisfied, set the final option.
        $final_option = 'branch';
      }
      else {
        // If the branch is not specified or does not exist, offer to get the
        // dev version of the default branch instead.
        $choice = bee_confirm(bt("The branch you have specified (!branch) is not valid for '!organization/!project'. Do you want to download the dev version of the default branch instead?", array(
          '!branch' => $branch,
          '!organization' => $organization,
          '!project' => $project,
        )), TRUE);
        if ($choice) {
          // If 'Yes' mode was enabled or user responded 'Yes', then continue
          // with dev version.
          $final_option = 'dev';
        }
        else {
          // If 'Yes' mode was not enabled and user responded 'No', then provide
          // a message and return FALSE.
          bee_message(bt("!project: Download operation is cancelled.", array(
            '!project' => $project,
          )), 'info');
          return FALSE;
        }
      }
      break;
    case 'select':
      $preferred_option = 'select';

      $selection = download_bee_select_version($project, $organization, $github_api_token);
      if (empty($selection)) {
        bee_message(bt("!project: Download operation is cancelled.", array(
          '!project' => $project,
        )), 'info');
        return FALSE;
      }
      // Use the selection to set variables that will be used in the next step.
      if ($selection['value'] == 'dev') {
        $final_option = 'dev';
      }
      elseif ($selection['value'] == 'cancel') {
        bee_message(bt("!project: Download operation is cancelled.", array(
          '!project' => $project,
        )), 'info');
        return FALSE;
      }
      else {
        $final_option = 'defined_release';
        $defined_release = $selection['value'];
        $release_data = $selection['data'];
      }
      break;
    default:
      // If user has not entered a keyword such as 'dev', 'latest' or 'select'
      // then, we interpret that as being a release tag and check for it.
      $preferred_option = 'defined_release';
      $defined_release = $release;
      $defined_release_url = "https://github.com/$organization/$project/releases/tag/$defined_release";
      if (download_bee_check_url_exists($defined_release_url)) {
        // If the release page exists, then this entry is valid and we can set
        // the final option.
        $final_option = 'defined_release';
        $endpoint = "$endpoint_base/releases/tags/$defined_release";
        $release_data = download_bee_github_api_call($endpoint, $github_api_token);
      }
      else {
        // If the release page does not exist offer to give them a choice of
        // valid releases and the dev version instead.
        if (is_numeric($defined_release[0])) {
          $confirm_message = bt("The release you have specified (!release_tag) for '!organization/!project' does not exist. Do you want to select a version instead?", array(
            '!release_tag' => $defined_release,
            '!organization' => $organization,
            '!project' => $project,
          ));
        }
        else {
          $confirm_message = bt("The version you specified (!release_tag) does not match a keyword or a release for '!organization/!project'. Do you want to select a version instead?", array(
            '!release_tag' => $defined_release,
            '!organization' => $organization,
            '!project' => $project,
          ));
        }
        $choice = bee_confirm($confirm_message, TRUE);
        if ($choice) {
          // If 'Yes' mode was enabled or user responded 'Yes', then continue
          // with selection.
          $selection = download_bee_select_version($project, $organization, $github_api_token);
          if (empty($selection)) {
            bee_message(bt("!project: Download operation is cancelled.", array(
              '!project' => $project,
            )), 'info');
            return FALSE;
          }
          // Use the selection to set variables that will be used in the next
          // step.
          if ($selection['value'] == 'dev') {
            $final_option = 'dev';
          }
          elseif ($selection['value'] == 'cancel') {
            bee_message(bt("!project: Download operation is cancelled.", array(
              '!project' => $project,
            )), 'info');
            return FALSE;
          }
          else {
            $final_option = 'defined_release';
            $defined_release = $selection['value'];
            $release_data = $selection['data'];
          }
        }
        else {
          // If 'Yes' mode was not enabled and user responded 'No', then provide
          // a message and return FALSE.
          bee_message(bt("!project: Download operation is cancelled.", array(
            '!project' => $project,
          )), 'info');
          return FALSE;
        }
      }
      break;
  }
  bee_instant_message('$final_option:' . $final_option, 'debug');
  // Process the final option.
  switch ($final_option) {
    case 'latest':
      // Check GitHub API quota to ensure we can continue.
      if (!download_bee_check_github_api_quota($github_api_token)) {
        return FALSE;
      }
      // Get the latest release information.
      $endpoint = "$endpoint_base/releases/latest";
      $release_data = download_bee_github_api_call($endpoint, $github_api_token);
      if (empty($release_data)) {
        // If release data can't be retrieved, display an error and return
        // FALSE.
        bee_message(bt("Release data for !project' could not be retrieved. Please try again later.", array(
          '!project' => $project,
        )), 'error');
        return FALSE;
      }
      $defined_release = $release_data['tag_name'];
      bee_instant_message('$defined_release:' . $defined_release, 'debug');
      $download_url = "https://github.com/$organization/$project/releases/download/$defined_release/$project.zip";
      bee_instant_message('$download_url:' . $download_url, 'debug');
      $release_detail = array(
        'tag_name' => $defined_release,
        'published_at' => $release_data['published_at'],
        'release_url' => $release_data['html_url'],
      );
      break;
    case 'dev':
      // Check GitHub API quota to ensure we can continue.
      if (!download_bee_check_github_api_quota($github_api_token)) {
        return FALSE;
      }
      // Get the information about the default branch.
      $endpoint = "$endpoint_base";
      $repo_info = download_bee_github_api_call($endpoint, $github_api_token);
      // Check the results of the API call.
      if (!$repo_info) {
        // If information can't be retrieved, display an error and return
        // FALSE.
        bee_message(bt("Information about '!project' could not be retrieved. Please try again later.", array(
          '!project' => $project,
        )), 'error');
        return FALSE;
      }
      else {
        $branch = $repo_info['default_branch'];
        $download_url = "https://github.com/$organization/$project/archive/$branch.zip";
      }
      break;
    case 'branch':
      $download_url = "https://github.com/$organization/$project/archive/$branch.zip";
      break;
    case 'defined_release':
      $download_url = "https://github.com/$organization/$project/releases/download/$defined_release/$project.zip";
      $release_detail = array(
        'tag_name' => $defined_release,
        'published_at' => $release_data['published_at'],
        'release_url' => $release_data['html_url'],
      );
      break;
  }

  // Start to populate the info array.
  $info = array(
    'download_url' => $download_url,
    'dependencies' => array(),
  );

  // Determine the endpoint reference by whether a release or a branch, and
  // populate the info array with conditional elements.
  switch ($final_option) {
    case 'dev':
      $ref = $branch;
      $info['branch'] = $branch;
    case 'branch':
      $ref = $branch;
      $info['branch'] = $branch;
      break;
    default:
      $ref = $defined_release;
      $info['release'] = $release_detail;
      $info['branch'] = '';
  }
  bee_instant_message('$ref:' . $ref, 'debug');
  // Download the info file to get project type and dependencies.
  // Check GitHub API quota to ensure we can continue.
  if (!download_bee_check_github_api_quota($github_api_token)) {
    return FALSE;
  }

  // If the project is Backdrop it won't have an info file to retrieve.
  if ($project == 'backdrop') {
    $info['type'] = 'core';
  }
  else {
    // If not Backdrop then retrieve the info file and process it.
    // Compile the endpoint.
    $endpoint = "$endpoint_base/contents/$project.info?ref=$ref";
    bee_instant_message('$endpoint:' . $endpoint, 'debug');
    // // Retrieve the info file as an array of lines.
    $info_file = download_bee_github_api_call($endpoint, $github_api_token, 'raw');
    bee_instant_message('$info_file:', 'debug', $info_file);
    // If the .info file cannot be found, then it will return a string
    // containing a JSON object whereas if it can be found, it will return a
    // string and therefore `json_decode` will return 'null'.
    if (!is_null(json_decode($info_file[0]))) {
      // If info file can't be retrieved, display an error and return FALSE.
      bee_message(bt("Error retrieving .info file for '!project'.", array(
        '!project' => $project,
      )), 'error');
      return FALSE;
    }
    $info_parsed = bee_parse_info_format($info_file[0]);
    bee_instant_message(bt('The info file data:'), 'debug', $info_parsed);
    $info['type'] = $info_parsed['type'];
    if (isset($info_parsed['dependencies'])) {
      foreach ($info_parsed['dependencies'] as $dependency) {
        $dependency = bee_parse_dependency($dependency);
        $info['dependencies'][] = $dependency;
      }
    }
  }

  // Return the info array to the calling function.
  return $info;
}

/**
 * Helper function to select version.
 */
function download_bee_select_version($project, $organization, $github_api_token = '') {
  // Create the array for options and hardcode the 'dev' option.
  $select_options = array(
    'dev' => bt("Dev - Download the current default branch."),
  );
  // Check GitHub API quota to ensure we can continue.
  if (!download_bee_check_github_api_quota($github_api_token)) {
    return FALSE;
  }
  // Get the list of releases.
  $endpoint = "repos/$organization/$project/releases?per_page=10";
  $releases = download_bee_github_api_call($endpoint, $github_api_token);
  // If one or more releases exist, then loop through them to get more
  // information.
  if (!empty($releases)) {
    foreach ($releases as $release_data) {
      // Convert the array to being an associative array keyed by tag_name.
      $releases_by_tag[$release_data['tag_name']] = $release_data;
      // If the release is not a prerelease, also put into an array to check
      // for latest release.
      if (!$release_data['prerelease']) {
        $full_release_list[] = $release_data['tag_name'];
      }
    }
    // If full releases exist, find the latest release.
    if (!empty($full_release_list)) {
      usort($full_release_list, 'version_compare');
      $latest_release_tag = end($full_release_list);
      $default = $latest_release_tag;
    }
    else {
      $latest_release_tag = NULL;
      $default = 'dev';
    }
    // Add each release to the list of options.
    foreach ($releases_by_tag as $key => $release_data) {
      // Define any suffixes needed for 'latest' or 'pre-release'.
      $suffix = '';
      if ($key == $latest_release_tag) {
        $suffix = bt(" (latest)");
      }
      if ($release_data['prerelease']) {
        $suffix = bt(" (pre-release)");
      }
      $select_options[$key] = $key . $suffix;
    }
    // Set the message to be used.
    $message = bt("!project: Select the version you wish to download.", array(
      '!project' => $project,
    ));
  }
  else {
    // If no releases exist, tailor the message and default.
    $message = bt("!project: No releases exist but you can download the dev version.", array(
      '!project' => $project,
    ));
    $default = 'dev';
  }
  $select_options['cancel'] = 'Cancel';
  // Present the choice to the user.
  $selection = bee_choice($select_options, $message, $default);
  bee_instant_message('$selection: ' . $selection, 'debug');
  switch ($selection) {
    case 'dev':
    case 'cancel':
      $result = array(
        'value' => $selection,
      );
      break;
    default:
      $result = array(
        'value' => $selection,
        'data' => $releases_by_tag[$selection],
      );
  }
  bee_instant_message('$result: ', 'debug', $result);
  return $result;
}

/**
 * Download a project.
 *
 * @param string $project
 *   The name of the project to download.
 * @param string $source_url
 *   The URL to download the project from.
 * @param string $destination
 *   The path to the destination directory.
 * @param string $branch
 *   If a dev version is being downloaded, branch is needed for the directory
 *   of the unzipped project. Defaults to empty string.
 * @param bool $replace
 *   Specify whether the download replaces an existing version. Defaults to
 *   FALSE.
 * @param bool $backup
 *   Specify, if replacing an existing version, whether or not to backup the
 *   existing code base.
 *
 * @return boolean
 *   TRUE if the project was downloaded successfully, FALSE if not.
 */
function download_bee_download_project($project, $source_url, $destination, $branch = '', bool $replace = FALSE, bool $backup = TRUE) {
  // Get a temp directory.
  if (!$temp = bee_get_temp($project)) {
    bee_message(bt('!project: Failed to get temp directory.', array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }
  if (file_exists($temp)) {
    bee_instant_message('$temp:' . $temp, 'debug');
  }
  // Get the download URL.
  if (empty($url = $source_url)) {
    bee_message(bt("The download URL for '!project' could not be found.", array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }
  bee_instant_message('$url:' . $url, 'debug');
  // Download & extract the project.
  $file = "$temp/$project.zip";
  $directory = !empty($branch) ? $project . '-' . $branch : $project;
  bee_instant_message('$directory:' . $directory, 'debug');

  $file_handle = fopen($file, 'w');
  $curl_handle = curl_init($url);
  curl_setopt($curl_handle, CURLOPT_FILE, $file_handle);
  curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
  curl_setopt($curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
  curl_exec($curl_handle);
  curl_close($curl_handle);
  fclose($file_handle);

  if (file_exists($file)) {
    bee_instant_message('$file:' . $file, 'debug');
  }
  $zip = new ZipArchive;
  if ($zip->open($file)) {
    $zip->extractTo($temp);
    $zip->close();
  }
  else {
    bee_message(bt('!project: Unable to open zip file.', array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }

  if (file_exists($temp)) {
    bee_instant_message('$temp:' . $temp, 'debug');
  }
  if (file_exists("$temp/$directory")) {
    bee_instant_message('$temp/$directory:' . "$temp/$directory", 'debug');
  }
  if ($replace) {
    // If the project is Backdrop we only want to backup and/or replace the
    // 'core' directory.
    if ($project == 'backdrop') {
      $destination .= '/core';
      $directory .= '/core';
    }

    if ($backup) {
      $backup_destination = "$_bee_backdrop_root/" . BEE_BACKUP_DIRECTORY;
      $backup_destination .= bee_format_date($_SERVER['REQUEST_TIME'], $format = 'YmdHis') . '/';
      $backup_destination .= ($project = 'backdrop') ? 'backdrop/' : 'projects/';
      if (!mkdir($backup_destination, 0660, TRUE )) {
        bee_message(bt('!project: Unable to make backup directory.', array(
          '!project' => $project,
        )), 'error');
        return FALSE;
      }
      $backup_copy = bee_copy($destination, $backup_destination);
      if (!$backup_copy) {
        bee_message(bt('!project: Unable to make backup copy.', array(
          '!project' => $project,
        )), 'error');
        return FALSE;
      }
    }
    $existing_delete = bee_delete($destination);
    if (!$existing_delete) {
      bee_message(bt('!project: Unable to delete existing files.', array(
        '!project' => $project,
      )), 'error');
      return FALSE;
    }
  }
  $copy = bee_copy("$temp/$directory", $destination, FALSE);
  if (!$copy) {
    bee_message(bt('!project: Unable to copy module files to the destination files.', array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }

  $temp_delete = bee_delete($temp);
  if (!$temp_delete) {
    bee_message(bt('!project: Unable to delete temp files.', array(
      '!project' => $project,
    )), 'warning');
  }
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
 * Get the destination path for the project.
 *
 * @param string $project
 *   The project system name.
 * @param string $type
 *   The type of project: 'module', 'theme' or 'layout'.
 *
 * @return string|false
 *   The absolute destination path. Return FALSE in the event of an error.
 */
function download_bee_get_destination_path($project, $type) {
  global $_bee_backdrop_root, $_bee_backdrop_site;
  // Add an 's' to the end of the type name.
  $type_folder = "{$type}s";

  // Get the directory to download the project into.
  if (!empty($_bee_backdrop_site)) {
    $destination = "$_bee_backdrop_root/sites/$_bee_backdrop_site/" . $type_folder;
  }
  elseif (!empty($_bee_backdrop_root)) {
    $destination = "$_bee_backdrop_root/" . $type_folder;
  }
  else {
    bee_message(bt("!project: The download destination could not be determined. Re-run the command from within a Backdrop installation, or set the global '--root'/'--site' options.", array(
      '!project' => $project,
    )), 'error');
    return FALSE;
  }

  // Check if the folder has a "contrib" subfolder within.
  if (file_exists("$destination/contrib")) {
    $destination .= '/contrib';
  }

  // Finally, add the project name to the end.
  $destination .= "/$project";
  if (file_exists($destination)) {
    bee_instant_message('$destination:' . $destination, 'debug');
  }
  return $destination;
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
function download_bee_check_github_api_quota($github_api_token = '', int $minimum_quota = 1) {

  $is_token = !empty($github_api_token);

  // Try a call to the GitHub Rate Limit API. This does not use any quota.
  $endpoint = 'rate_limit';
  $data = download_bee_github_api_call($endpoint, $github_api_token);
  if (!$data) {
    bee_message(bt('GitHub API Rate Limit information cannot be retrieved at this time. Please try again later.'), 'error');
    bee_instant_message('GitHub API Rate Limit connection failure', 'log');
    return FALSE;
  }
  // Return the core quota details.
  // var_export($data);
  $rate_info = $data['resources']['core'];
  $reset_time = bee_format_date($rate_info['reset']);
  $used = $rate_info['used'];
  $remaining = $rate_info['remaining'];
  $quota = $rate_info['limit'];
  bee_instant_message("$used/$quota used. Reset at $reset_time", 'debug');
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

/**
 * Call the Github API.
 *
 * @param string $endpoint
 *   The GitHub API endpoint to use.
 * @param string $github_api_token
 *   The GitHub Personal Access Token to check. Anonymous if left blank.
 * @param string $request_type
 *   Set the request type to be either 'json' (default) or 'raw'.
 *
 * @return array|false
 *   Returns an array of the response if successful; FALSE otherwise.
 */
function download_bee_github_api_call($endpoint, $github_api_token = '', $request_type = 'json') {

  if ($curl_handle = curl_init(BEE_GITHUB_API_URL . "$endpoint")) {
    // Prepare headers for curl request.
    $curlopt_httpheader = array();
    switch ($request_type) {
      case 'raw':
        $curlopt_httpheader[] = 'Accept: application/vnd.github.v3.raw';
        break;
      default:
        $curlopt_httpheader[] = 'Content-Type: application/json';
        break;
    }

    // If a token has been passed, add this to the header.
    if (!empty($github_api_token)) {
      $curlopt_httpheader[] = "Authorization: token $github_api_token";
    }

    // Compile the options for the curl request.
    curl_setopt($curl_handle, CURLOPT_HTTPHEADER, $curlopt_httpheader);
    curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
    curl_setopt($curl_handle, CURLOPT_HEADER, 0);
    curl_setopt($curl_handle, CURLOPT_RETURNTRANSFER, 1);

    // Execute and close the curl request.
    $response = curl_exec($curl_handle);
    curl_close($curl_handle);

    // Process the output according to the request type.
    switch ($request_type) {
      case 'raw':
        $output = array($response);
        break;
      default:
        $output = json_decode($response, TRUE);
        break;
    }
    return $output;
  }
  else {
    return FALSE;
  }
}

/**
 * Check whether a URL exists.
 *
 * This helper function provides a simple check for URLs that does not require
 * any use of the GitHub API call quota.
 *
 * @param string $url
 *   The URL to check.
 *
 * @return bool
 *   Returns TRUE if a valid result is returned. FALSE otherwise.
 */
function download_bee_check_url_exists($url) {
  // Initiate the curl request.
  $curl_handle = curl_init($url);

  // Compile the options for the curl request.
  // Set the user agent string.
  curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
  // Allow curl to follow redirects.
  curl_setopt($curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
  // Specify that we don't need the body of the response.
  curl_setopt($curl_handle, CURLOPT_NOBODY, TRUE);

  // Execute the curl request.
  curl_exec($curl_handle);

  // Get the response code.
  $return_code = curl_getinfo($curl_handle, CURLINFO_RESPONSE_CODE);

  // Close the curl request.
  curl_close($curl_handle);

  // Return the result.
  return $return_code == 200;
}

/**
 * Check whether a latest release exists.
 *
 * @param string $project
 *   The project to check for.
 * @param string $organization
 *   The organization to check for.
 *
 * @return bool
 *   TRUE if a latest release exists; FALSE if not.
 */
function download_bee_check_latest_release_exists($project, $organization) {

  $url = "https://github.com/$organization/$project/releases/latest";
  // Initiate the curl request.
  $curl_handle = curl_init($url);

  // Compile the options for the curl request.
  // Set the user agent string.
  curl_setopt($curl_handle, CURLOPT_USERAGENT, BEE_USERAGENT);
  // Allow curl to follow redirects.
  curl_setopt($curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);
  // Specify that we don't need the body of the response.
  curl_setopt($curl_handle, CURLOPT_NOBODY, TRUE);

  // Execute the curl request.
  curl_exec($curl_handle);

  // Get the URL that the latest release URL diverts to.
  $effective_url = curl_getinfo($curl_handle, CURLINFO_EFFECTIVE_URL);

  // Close the curl request.
  curl_close($curl_handle);

  $no_latest_release_url = "https://github.com/$organization/$project/releases";
  return $effective_url != $no_latest_release_url;
}

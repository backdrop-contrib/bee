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
      'aliases' => array('pm-download', 'dl'),
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
    // Get the project type.
    if (!$type = download_b_get_type($project)) {
      b_message(bt("The 'type' of project `@project` could not be determined.", array(
        '@project' => $project,
      )), 'error');
      return;
    }

    // Get the directory to download the project into.
    if (!empty($backdrop_site)) {
      $destination = "$backdrop_root/sites/$backdrop_site/$type";
    }
    elseif (!empty($backdrop_root)) {
      $destination = "$backdrop_root/$type";
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
    if (download_b_download_project($project, $destination)) {
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
  // Get the directory to download Backdrop into.
  $destination = !empty($arguments['directory']) ? $arguments['directory'] : getcwd();

  // Create the directory if it doesn't exist.
  if (!realpath($destination)) {
    if (!mkdir($destination, 0755)) {
      b_message(bt("The specified directory `@directory` doesn't exist and couldn't be created.", array(
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
  if (download_b_download_project('backdrop', $destination)) {
    b_message(bt('Backdrop was downloaded into `@directory`.', array(
      '@directory' => $destination,
    )), 'success');
  }
}

/**
 * Get the 'type' (e.g. module, theme, layout) of a project.
 *
 * @param string $project
 *   The name of the project to get the type of.
 *
 * @return string|false
 *   The 'type' of the project, or FALSE if the project/type could not be found.
 */
function download_b_get_type($project) {
  // Get the contents of the repo.
  $api_url = "https://api.github.com/repos/backdrop-contrib/$project/contents";
  $files = shell_exec('curl ' . $api_url);
  $files = json_decode($files, TRUE);

  // Get the `*.info` file.
  foreach ($files as $file) {
    if ($file['type'] == 'file' && preg_match("/\.info$/", $file['name'])) {
      // Read the contents of the info file.
      $info = shell_exec('curl -H "Accept: application/vnd.github.v3.raw" ' . $api_url . '/' . $file['name']);
      $lines = explode("\n", $info);

      // Get the 'type' value.
      foreach ($lines as $line) {
        $values = explode('=', $line);
        if (trim($values[0]) == 'type') {
          return trim($values[1]);
        }
      }
    }
  }

  return FALSE;
}

/**
 * Download a project.
 *
 * @param string $project
 *   The name of the project to download.
 * @param string $destination
 *   The path to the destination directory.
 *
 * @return boolean
 *   TRUE if the project was downloaded successfully, FALSE if not.
 */
function download_b_download_project($project, $destination) {
  // Get a temp directory.
  if (!$temp = b_get_temp($project)) {
    b_message(bt('Failed to get temp directory.'), 'error');
    return FALSE;
  }

  // Get the download URL.
  if (!$url = download_b_get_url($project)) {
    return FALSE;
  }

  // Download & extract the project.
  $file = "$temp/$project.zip";
  exec("wget --show-progress -qO $file $url");
  exec("unzip $file -d $temp");
  b_copy("$temp/$project", $destination, FALSE);
  b_delete($temp);

  return TRUE;
}

/**
 * Get the download URL for a project.
 *
 * @param string $project
 *   The name of the project to download.
 *
 * @return string|false
 *   The URL to download the project, or FALSE if the URL could not be found.
 */
function download_b_get_url($project) {
  // Get the GitHub organisation.
  $organisation = ($project == 'backdrop') ? 'backdrop' : 'backdrop-contrib';

  // Get and check the URL of the latest release.
  $url = 'https://github.com/' . $organisation . '/' . $project . '/releases/latest/download/' . $project . '.zip';
  $headers = get_headers($url);
  $response = substr($headers[0], 9, 3);
  if ($response >= 400) {
    $answer = b_confirm(bt('There is no release for `@organisation/@project`. Do you you want to download the dev version instead?', array(
      '@organisation' => $organisation,
      '@project' => $project,
    )));

    if (!$answer) {
      b_message(bt('No release for `@project` was found, and the dev version was declined.', array(
        '@project' => $project,
      )), 'warning');
      return FALSE;
    }
    else {
      // Get and check the URL of dev version.
      $repo_info = shell_exec('curl https://api.github.com/repos/' . $organisation . '/' . $project);
      $repo_info = json_decode($repo_info, TRUE);
      $branch = $repo_info['default_branch'];
      $url = 'https://github.com/' . $organisation . '/' . $project . '/archive/' . $branch . '.zip';
      $headers = get_headers($url);
      $response = substr($headers[0], 9, 3);
      if ($response >= 400) {
        b_message(bt('The dev version of `@project` could not be downloaded.', array(
          '@project' => $project,
        )), 'error');
        return FALSE;
      }
    }
  }

  return $url;
}




























/******************************************************************************/

// /**
//  * Command callback: Download Backdrop contrib projects.
//  */
// function download_b_callback($arguments, $options) {
//   foreach ($arguments['projects'] as $project) {
//     $temp = download_b_temp_download($project);
//
//     $download_target = download_get_target($project, $temp . '/' . $project, $options);
//
//     if (!$download_target) {
//       b_message('Failed to detect destination directory.', 'error');
//       b_delete($temp);
//       return FALSE;
//     }
//
//     if (file_exists($download_target . '/' . $project)) {
//       b_message('Folder ' . $download_target . '/' . $project . ' already exists', 'error');
//       b_delete($temp);
//       return FALSE;
//     }
//
//     // Create the folder if it doesn't exist.
//     if (!file_exists($download_target)) {
//       if (!mkdir($download_target, 0755)) {
//         b_message("Folder " . $download_target . " doesn't exist and attempts to create it failed.", 'error');
//         b_delete($temp);
//         return FALSE;
//       }
//     }
//
//     $download_target = $download_target . '/' . $project;
//
//     b_copy($temp . '/' . $project, $download_target);
//     b_message($project . ' downloaded to ' . $download_target . '!', 'success');
//     b_delete($temp);
//   }
// }
//
// /**
//  * Command callback: Download Backdrop core.
//  */
// function download_core_b_callback($arguments, $options) {
//   $project = 'backdrop';
//   $temp = download_b_temp_download($project);
//
//   $download_target = download_get_target($project, $temp . '/' . $project, $options);
//
//   if (!$download_target) {
//     b_message('Failed to detect destination directory.', 'error');
//     b_delete($temp);
//     return FALSE;
//   }
//
//   if (file_exists($download_target . '/' . $project)) {
//     b_message('Folder ' . $download_target . '/' . $project . ' already exists', 'error');
//     b_delete($temp);
//     return FALSE;
//   }
//
//   // Create the folder if it doesn't exist.
//   if (!file_exists($download_target)) {
//     if (!mkdir($download_target, 0755)) {
//       b_message("Folder " . $download_target . " doesn't exist and attempts to create it failed.", 'error');
//       b_delete($temp);
//       return FALSE;
//     }
//   }
//
//   $download_target = $download_target . '/' . $project;
//
//   b_copy($temp . '/' . $project, $download_target);
//   b_message($project . ' downloaded to ' . $download_target . '!', 'success');
//   b_delete($temp);
// }
//
// /**
//  * Download a project to a temporary directory.
//  *
//  * @param string $project
//  *   The name of the project to download.
//  *
//  * @return string
//  *   The path to the temporary directory where the project was downloaded.
//  */
// function download_b_temp_download($project) {
//   // Get the GitHub organisation.
//   $organisation = ($project == 'backdrop') ? 'backdrop' : 'backdrop-contrib';
//
//   // Get the URL of the latest release.
//   $url = 'https://github.com/' . $organisation . '/' . $project . '/releases/latest/download/' . $project . '.zip';
//   $headers = get_headers($url);
//   $response = substr($headers[0], 9, 3);
//   if ($response >= 400) {
//     $answer = b_confirm(bt('There is no release for `@organisation/@project`. Do you you want to download the dev version instead?', array(
//       '@organisation' => $organisation,
//       '@project' => $project,
//     )));
//     if (!$answer) {
//       b_message(bt('No release for `@project` was found, and the dev version was declined.', array(
//         '@project' => $project,
//       )), 'warning');
//       return;
//     }
//
//     // Get the URL of dev version.
//     $repo_info = shell_exec('curl https://api.github.com/repos/' . $organisation . '/' . $project);
//     $repo_info = backdrop_json_decode($repo_info);
//     $branch = $repo_info['default_branch'];
//     $url = 'https://github.com/' . $organisation . '/' . $project . '/archive/' . $branch . '.zip';
//     $headers = get_headers($url);
//     $response = substr($headers[0], 9, 3);
//     if ($response >= 400) {
//       b_message(bt('The dev version of `@project` could not be downloaded.', array(
//         '@project' => $project,
//       )), 'error');
//       return;
//     }
//   }
//
//   // Create a temp directory.
//   if ($temp = b_get_temp()) {
//     $temp .= '/' . uniqid($project);
//     mkdir($temp);
//   }
//   else {
//     b_message(bt('Failed to create temp directory.'), 'error');
//     return;
//   }
//
//   // Download the project.
//   exec('wget -qO ' . $temp . '/' . $project . '.zip ' . $url);
//   exec('unzip ' . $temp . '/' . $project . '.zip -d ' . $temp);
//   unlink($temp . '/' . $project . '.zip');
//
//   return $temp;
// }
//
// /**
//  * Based on type of the project, return directory to move project into.
//  *
//  * @param string $project
//  *   The machine name of the project.
//  * @param string $folder
//  *   The folder.
//  * @param array $options
//  *   Array of options for the command.
//  *
//  * @return string|FALSE
//  *   The directory path to the destination of the project or FALSE.
//  */
// function download_get_target($project, $folder, $options) {
//   $download_target = FALSE;
//   if (defined('BACKDROP_SITE')) {
//     // If BACKDROP_SITE is set, we know BACKDROP_ROOT is set too.
//     $download_target = BACKDROP_ROOT . '/sites/' . BACKDROP_SITE;
//   }
//   elseif (defined('BACKDROP_ROOT')) {
//     $download_target = BACKDROP_ROOT;
//   }
//   if (!$download_target) {
//     b_message('Please change directory to BackdropCMS root or specify --root option.', 'error');
//     return FALSE;
//   }
//   $type = download_get_type($folder);
//   if (FALSE == $type && isset($options['type'])) {
//     $type = $options['type'];
//   }
//   switch ($type) {
//     case 'module':
//       $download_target .= '/modules';
//       break;
//
//     case 'theme':
//       $download_target .= '/themes';
//       break;
//
//     case 'layout':
//       $download_target .= '/layouts';
//       break;
//
//     default:
//       b_message(bt('!project does not have type.', array('!project' => $project)), 'error');
//       return FALSE;
//   }
//   if (file_exists($download_target . '/contrib')) {
//     $download_target = $download_target . '/contrib';
//   }
//   return $download_target;
// }
//
// /**
//  * Scan for info file and return project type.
//  *
//  * @param string $folder
//  *   The folder.
//  *
//  * @return string|FALSE
//  *   Either string of type of project: module, theme or layout or FALSE
//  */
// function download_get_type($folder) {
//   $files = b_file_scan_directory($folder, "/\.info$/");
//   foreach ($files as $file) {
//     $content = file($file->uri);
//     foreach ($content as $line) {
//       if (FALSE !== strpos($line, 'type')) {
//         list($title, $type) = explode("=", $line);
//         $type = trim($type);
//         if (in_array($type, array('module', 'theme', 'layout'))) {
//           return $type;
//         }
//       }
//     }
//   }
//   return FALSE;
// }

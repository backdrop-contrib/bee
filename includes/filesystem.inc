<?php
/**
 * @file
 * Functions for interaction with the filesystem.
 */

/**
 * Find Backdrop's root directory.
 *
 * @param string $path
 *   The path to check for an installation of Backdrop.
 * @param bool $recursive
 *   Whether or not to recurse backwards/up through the filesystem from the
 *   given `$path`. Defaults to TRUE.
 *
 * @return string|false
 *   The path to the root directory of Backdrop, or FALSE if no Backdrop
 *   installation was found.
 */
function bee_find_root($path, $recursive = TRUE) {
  // Make sure we have a valid dir path.
  if (!empty($path) && is_dir($path)) {
    // Check if this is Backdrop's root dir.
    if (file_exists($path . '/index.php') && file_exists($path . '/core/misc/backdrop.js')) {
      return $path;
    }
    elseif ($recursive) {
      // Get the path of the parent dir.
      $path = dirname($path);
      if (!in_array($path, array('/', '.'))) {
        // Check the parent path.
        return bee_find_root($path);
      }
    }
  }
  else {
    bee_message(bt('!path is not a valid directory.', array(
      '!path' => $path,
    )), 'error');
  }

  return FALSE;
}

/**
 * Check if this is a multisite installation, based on the presence of a
 * populated `sites.php` file.
 *
 * @return array|false
 *   Returns an array of sites (as defined in `sites.php`) if this is a
 *   multisite installation, or FALSE if it's a regular (non-multisite)
 *   installation.
 */
function bee_is_multisite() {
  global $_bee_backdrop_root;
  $sites_php = $_bee_backdrop_root . '/sites/sites.php';

  if (file_exists($sites_php)) {
    $sites = array();

    // Populate the $sites array.
    include $sites_php;

    if (!empty($sites)) {
      return $sites;
    }
  }

  return FALSE;
}

/**
 * Find Backdrop's site directory from the given site directory or URL.
 *
 * @param string $site
 *   The directory name or URL of the site to find, as defined in `sites.php`.
 *
 * @return string|false
 *   The site directory, as defined in `sites.php`, or FALSE if no site was
 *   found.
 */
function bee_find_site_by_site($site) {
  global $_bee_backdrop_root, $_bee_multisites;

  if ($_bee_multisites) {
    if (isset($_bee_multisites[$site])) {
      return $_bee_multisites[$site];
    }
    elseif (in_array($site, $_bee_multisites)) {
      return $site;
    }
    else {
      bee_message(bt('!site could not be found in !root/sites/sites.php.', array(
        '!site' => $site,
        '!root' => $_bee_backdrop_root,
      )), 'error');
    }
  }
  else {
    bee_message(bt('!root/sites/sites.php could not be found or is empty.', array(
      '!root' => $_bee_backdrop_root,
    )), 'error');
  }

  return FALSE;
}

/**
 * Find Backdrop's site directory from the given `$path`.
 *
 * @param string $path
 *   The path to check for the current site.
 *
 * @return string|false
 *   The site directory, or FALSE if no site was found.
 */
function bee_find_site_by_path($path) {
  global $_bee_backdrop_root;

  // Make sure we have a valid dir path.
  if (!empty($path) && is_dir($path)) {
    // Check if this is Backdrop's current site dir.
    if (file_exists(realpath($path . '/settings.php'))) {
      if ($path != $_bee_backdrop_root) {
        // Return the last part of the path (the site dir name).
        return basename($path);
      }
    }
    else {
      // Get the path of the parent dir.
      $path = dirname($path);
      if (!in_array($path, array('/', '.'))) {
        // Check the parent path.
        return bee_find_site_by_path($path);
      }
    }
  }
  else {
    bee_message(bt('!path is not a valid directory.', array(
      '!path' => $path,
    )), 'error');
  }

  return FALSE;
}

/**
 * Finds all files that match a given mask in a given directory.
 *
 * @see file_scan_directory()
 *
 * Copied here so that it can be used even when Backdrop is not available.
 * Some changes have been made:
 * - The line: `$uri = file_stream_wrapper_uri_normalize($uri);` has been
 *   commented out since `file_stream_wrapper_uri_normalize()` is a Backdrop
 *   function and it calls other Backdrop functions.
 * - The recursive call to `file_scan_directory()` has been changed to this
 *   `bee` specific version.
 */
function bee_file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
  // By default, do not check for files in common special-purpose directories.
  $ignore_directories = array(
    'node_modules',
    'bower_components',
  );
  $no_mask = '/^((\..*)|' . implode('|', $ignore_directories) .  ')$/';

  // Merge in defaults.
  $options += array(
    'nomask' => $no_mask,
    'callback' => 0,
    'recurse' => TRUE,
    'key' => 'uri',
    'min_depth' => 0,
  );

  $options['key'] = in_array($options['key'], array('uri', 'filename', 'name')) ? $options['key'] : 'uri';
  $files = array();
  if (is_dir($dir) && $handle = opendir($dir)) {
    while (FALSE !== ($filename = readdir($handle))) {
      if (!preg_match($options['nomask'], $filename) && $filename !== '.' && $filename !== '..') {
        $uri = "$dir/$filename";
        // $uri = file_stream_wrapper_uri_normalize($uri);
        if (is_dir($uri) && $options['recurse']) {
          // Give priority to files in this folder by merging them in after
          // any subdirectory files.
          $files = array_merge(bee_file_scan_directory($uri, $mask, $options, $depth + 1), $files);
        }
        elseif ($depth >= $options['min_depth'] && preg_match($mask, $filename)) {
          // Always use this match over anything already set in $files with the
          // same $$options['key'].
          $file = new stdClass();
          $file->uri = $uri;
          $file->filename = $filename;
          $file->name = pathinfo($filename, PATHINFO_FILENAME);
          $key = $options['key'];
          $files[$file->$key] = $file;
          if ($options['callback']) {
            $options['callback']($uri);
          }
        }
      }
    }

    closedir($handle);
  }

  return $files;
}

/**
 * Returns the path to a temporary directory.
 *
 * @param string $prefix
 *   A prefix to use for the subfolder.
 *
 * @return string|false
 *   The path to a subfolder in the first matching temporary directory, or FALSE
 *   if the folder could not be found/created.
 */
function bee_get_temp($prefix = '') {
  static $temp = FALSE;

  if (!$temp) {
    // Various possible paths to check, in order of preference.
    $directories = array(
      sys_get_temp_dir(),
      getenv('TEMP'),
      getenv('TMP'),
      '/tmp',
    );

    // Get the first directory that's writable.
    foreach ($directories as $directory) {
      if (is_dir($directory) && is_writable($directory)) {
        $temp = $directory;
        break;
      }
    }

    if (!$temp) {
      return FALSE;
    }
  }

  // Create a prefixed subfolder.
  $subfolder = $temp . '/' . uniqid($prefix);
  if (!mkdir($subfolder)) {
    return FALSE;
  }

  return $subfolder;
}

/**
 * Deletes a file or directory.
 *
 * @param string $path
 *   The file or directory to be deleted.
 *
 * @return bool
 *   TRUE for success or if the path doesn't exist (since that's the desired
 *   result), or FALSE in the event of an error.
 *
 * @see https://stackoverflow.com/a/3349792/4168619
 */
function bee_delete($path) {
  // Delete files.
  if (is_file($path)) {
    return unlink($path);
  }
  // Delete directories.
  elseif (is_dir($path)) {
    $iterator = new RecursiveDirectoryIterator($path, RecursiveDirectoryIterator::SKIP_DOTS);
    $files = new RecursiveIteratorIterator($iterator, RecursiveIteratorIterator::CHILD_FIRST);

    // Delete all sub-files/directories first.
    foreach ($files as $file) {
      $realpath = $file->getRealPath();

      if ($file->isDir()) {
        rmdir($realpath);
      } else {
        unlink($realpath);
      }
    }

    // Delete the directory itself last.
    return rmdir($path);
  }

  // Log a message if the file/directory doesn't exist (but return TRUE anyway).
  if (!file_exists(realpath($path))) {
    bee_message(bt("'!path' was not deleted because it doesn't exist.", array(
      '!path' => $path,
    )), 'log');
    return TRUE;
  }

  // Display an error for non-files/directories.
  bee_message(bt("'!path' was not deleted because it is not a recognized type.", array(
    '!path' => $path,
  )), 'error');
  return FALSE;
}

/**
 * Copies a file or directory to another location.
 *
 * @param string $source
 *   The file or directory to be copied.
 * @param string $destination
 *   The directory to put the copied files/folders into. It will be created if
 *   it doesn't exist.
 * @param boolean $self
 *   When TRUE, the source directory will be copied along with its children.
 *   When FALSE, only the children will be copied. Has no effect if $source is a
 *   file.
 *
 * @return boolean
 *   Whether or not the copy completed successfully.
 *
 * @see https://stackoverflow.com/a/2050909/4168619
 */
function bee_copy($source, $destination, $self = TRUE) {
  // Copy source directory too.
  if (is_dir($source) && $self) {
    $destination .= '/' . basename(realpath($source));
  }

  // Create the destination directory.
  if (!file_exists(realpath($destination))) {
    if (!mkdir($destination)) {
      bee_message(bt("The destination '!destination' doesn't exist and could not be created.", array(
        '!destination' => $destination,
      )), 'error');
      return FALSE;
    }
  }
  // Make sure the destination is actually a directory.
  elseif (!is_dir($destination)) {
    bee_message(bt("The destination '!destination' is not a directory.", array(
      '!destination' => $destination,
    )), 'error');
    return FALSE;
  }

  if (is_dir($source)) {
    // Copy sub-files/directories recursively.
    $directory = opendir($source);
    while (($file = readdir($directory)) !== FALSE) {
      if ($file != '.' && $file != '..') {
        if (is_dir("$source/$file")) {
          bee_copy("$source/$file", "$destination/$file", FALSE);
        }
        else {
          copy("$source/$file", "$destination/$file");
        }
      }
    }
    closedir($directory);
  }
  else {
    // Copy a single file.
    if (!copy($source, $destination)) {
      bee_message(bt("Failed to copy '!source' to '!destination'.", array(
        '!source' => $source,
        '!destination' => $destination,
      )), 'error');
      return FALSE;
    }
  }

  return TRUE;
}

/**
 * Get the full path for a command executable if it exists in PATH.
 *
 * If your function calls an executable on the system rather than using `bee`,
 * `backdrop` or `php` functions, you can use this function to get the full
 * path of the executable or return FALSE if the executable doesn't exist. The
 * result can then be checked in your calling function, and you can exit with a
 * user friendly error message if the executable doesn't exist in the system.
 *
 * @param string $command
 *   Name of the command to be searched.
 *
 * @return string|FALSE
 *   Full path to the first occurence of $command in the search path.
 *   Returns FALSE if no matching command is found.
 */
function bee_get_executable_path($command) {
  foreach (explode(":", getenv('PATH')) as $directory) {
    $candidate = "$directory/$command";
    if (is_executable($candidate)) {
      return $candidate;
    }
  }
  return FALSE;
}

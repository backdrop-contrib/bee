<?php
/**
 * @file
 * Filesystem utilities.
 */

/**
 * Finds all files that match a given mask in a given directory.
 *
 * Directories and files beginning with a period are excluded; this
 * prevents hidden files and directories (such as SVN working directories)
 * from being scanned.
 *
 * @param $dir
 *   The base directory or URI to scan, without trailing slash.
 * @param $mask
 *   The preg_match() regular expression of the files to find.
 * @param $options
 *   An associative array of additional options, with the following elements:
 *   - 'nomask': The preg_match() regular expression of the files to ignore.
 *     Defaults to '/^(\..*)|(CVS)$/'. This default ignores all hidden files
 *     (those that start with a period) and items named "CVS".
 *   - 'callback': The callback function to call for each match. There is no
 *     default callback.
 *   - 'recurse': When TRUE, the directory scan will recurse the entire tree
 *     starting at the provided directory. Defaults to TRUE.
 *   - 'key': The key to be used for the returned associative array of files.
 *     Possible values are 'uri', for the file's URI; 'filename', for the
 *     basename of the file; and 'name' for the name of the file without the
 *     extension. Defaults to 'uri'.
 *   - 'min_depth': Minimum depth of directories to return files from. Defaults
 *     to 0.
 * @param $depth
 *   Current depth of recursion. This parameter is only used internally and
 *   should not be passed in.
 *
 * @return
 *   An associative array (keyed on the chosen key) of objects with 'uri',
 *   'filename', and 'name' members corresponding to the matching files.
 */
function b_file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
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
//        $uri = file_stream_wrapper_uri_normalize($uri);
        if (is_dir($uri) && $options['recurse']) {
          // Give priority to files in this folder by merging them in after any subdirectory files.
          $files = array_merge(b_file_scan_directory($uri, $mask, $options, $depth + 1), $files);
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
 * Generate temporary directory and return path to it.
 *
 */
function b_tmp_folder(){
  if($tmp = b_find_tmp()) {
    $tmp = $tmp . '/' .hash('sha256', mt_rand());
    mkdir($tmp);
    return $tmp;
  }
  return FALSE;
}
/**
 * Returns the path to a temporary directory.
 *
 * This is a custom version of Drupal's file_directory_path().
 * We can't directly rely on sys_get_temp_dir() as this
 * path is not valid in some setups for Mac, and we want to honor
 * an environment variable (used by tests).
 *
 * Copied from drush_find_tmp();
 */
function b_find_tmp() {
  static $temporary_directory;
  if (!isset($temporary_directory)) {
    $directories = array();
    // Get user specific and operating system temp folders from system environment variables.
    // See http://www.microsoft.com/resources/documentation/windows/xp/all/proddocs/en-us/ntcmds_shelloverview.mspx?mfr=true
    $tempdir = getenv('TEMP');
    if (!empty($tempdir)) {
      $directories[] = $tempdir;
    }
    $tmpdir = getenv('TMP');
    if (!empty($tmpdir)) {
      $directories[] = $tmpdir;
    }
    // Operating system specific dirs.
    $windir = getenv('WINDIR');
    if (isset($windir)) {
      // WINDIR itself is not writable, but it always contains a /Temp dir,
      // which is the system-wide temporary directory on older versions. Newer
      // versions only allow system processes to use it.
      $directories[] = $windir . '/Temp';
    }
    else {
      $directories[] = '/tmp';
    }
    $directories[] = sys_get_temp_dir();
    foreach ($directories as $directory) {
      if (is_dir($directory) && is_writable($directory)) {
        $temporary_directory = $directory;
        break;
      }
    }
    if (empty($temporary_directory)) {
      // If no directory has been found, create one in cwd.
      $temporary_directory = getcwd() . '/tmp';
      mkdir($temporary_directory, TRUE);
      if (!is_dir($temporary_directory)) {
        b_set_message('Unable to create a temporary directory.', 'error');
        return FALSE;
      }
    }
  }
  return $temporary_directory;
}

/**
 * Deletes all files and directories in the specified filepath recursively.
 *
 * If the specified path is a directory then the function will call itself
 * recursively to process the contents. Once the contents have been removed the
 * directory will also be removed.
 *
 * If the specified path is a file then it will be passed to
 * file_unmanaged_delete().
 *
 * Note that this only deletes visible files with write permission.
 *
 * @param $path
 *   A string containing either an URI or a file or directory path.
 *
 * @return
 *   TRUE for success or if path does not exist, FALSE in the event of an
 *   error.
 *
 * @see file_unmanaged_delete()
 */
function b_file_unmanaged_delete_recursive($path) {
  if (is_dir($path)) {
    $dir = dir($path);
    while (($entry = $dir->read()) !== FALSE) {
      if ($entry == '.' || $entry == '..') {
        continue;
      }
      $entry_path = $path . '/' . $entry;
      b_file_unmanaged_delete_recursive($entry_path);
    }
    $dir->close();

    return b_rmdir($path);
  }
  return b_file_unmanaged_delete($path);
}

/**
 * Deletes a file without database changes or hook invocations.
 *
 * This function should be used when the file to be deleted does not have an
 * entry recorded in the files table.
 *
 * @param $path
 *   A string containing a file path or (streamwrapper) URI.
 *
 * @return
 *   TRUE for success or path does not exist, or FALSE in the event of an
 *   error.
 *
 * @see file_delete()
 * @see file_unmanaged_delete_recursive()
 */
function b_file_unmanaged_delete($path) {
  if (is_dir($path)) {
    b_set_message($path . ' is a directory and cannot be removed using file_unmanaged_delete().', 'error');
    return FALSE;
  }
  if (is_file($path)) {
    return b_unlink($path);
  }
  // Return TRUE for non-existent file, but log that nothing was actually
  // deleted, as the current state is the intended result.
  if (!file_exists($path)) {
    b_set_message('The file ' . $path . '  was not deleted because it does not exist.', 'warning');
    return TRUE;
  }
  // We cannot handle anything other than files and directories. Log an error
  // for everything else (sockets, symbolic links, etc).
  b_set_message('The file ' . $path . ' is not of a recognized type so it was not deleted.', 'error');
  return FALSE;
}

/**
 * Removes a directory.
 *
 * PHP's rmdir() is broken on Windows, as it can fail to remove a directory
 * when it has a read-only flag set.
 *
 * @param $uri
 *   A URI or pathname.
 *
 * @return
 *   Boolean TRUE on success, or FALSE on failure.
 *
 * @see rmdir()
 * @ingroup php_wrappers
 */
function b_rmdir($uri) {
  if (substr(PHP_OS, 0, 3) == 'WIN') {
    chmod($uri, 0700);
  }
  return rmdir($uri);
}

/**
 * Deletes a file.
 *
 * PHP's unlink() is broken on Windows, as it can fail to remove a file
 * when it has a read-only flag set.
 *
 * @param $uri
 *   A URI or pathname.
 *
 * @return
 *   Boolean TRUE on success, or FALSE on failure.
 *
 * @see unlink()
 * @ingroup php_wrappers
 */
function b_unlink($uri) {
  if (substr(PHP_OS, 0, 3) == 'WIN') {
    chmod($uri, 0600);
  }
  return unlink($uri);
}

/**
 * Recursively copy one directory to another.
 *
 */
function b_recursive_copy($src, $dst) {
  $dir = opendir($src);
  if(!file_exists($dst)){
    mkdir($dst);
  }
  while(false !== ( $file = readdir($dir)) ) {
    if (( $file != '.' ) && ( $file != '..' )) {
      if ( is_dir($src . '/' . $file) ) {
        b_recursive_copy($src . '/' . $file, $dst . '/' . $file);
      }
      else {
        copy($src . '/' . $file, $dst . '/' . $file);
      }
    }
  }
  closedir($dir);
}

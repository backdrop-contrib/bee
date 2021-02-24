<?php
/**
 * @file
 * Miscellaneous functions.
 */

/**
 * Initialize $_SERVER environment variables.
 */
function b_initialize_server() {
  $_SERVER['HTTP_HOST'] = 'localhost';
  // @codingStandardsIgnoreLine -- Not applicable to use ip_address() instead.
  $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_SOFTWARE'] = NULL;
  $_SERVER['SERVER_NAME'] = 'localhost';
  $_SERVER['REQUEST_URI'] = '/';
  $_SERVER['REQUEST_METHOD'] = 'GET';
  $_SERVER['SCRIPT_NAME'] = '/index.php';
  $_SERVER['PHP_SELF'] = '/index.php';
  $_SERVER['HTTP_USER_AGENT'] = 'Backdrop Console';
}

/**
 * Initialize Backdrop Console.
 */
function b_initialize_console() {
  global $options, $backdrop_installed, $backdrop_root, $backdrop_site, $multisites, $yes_mode, $debug_mode;

  // Get the root directory of the Backdrop installation to use.
  if (isset($options['root'])) {
    $backdrop_root = b_find_root($options['root'], FALSE);
  }
  else {
    $backdrop_root = b_find_root(getcwd());
  }

  if ($backdrop_root) {
    if ($multisites = b_is_multisite($backdrop_root)) {
      // Get the site directory of the Backdrop multisite to use.
      if (isset($options['site'])) {
        $backdrop_site = b_find_site_by_site($options['site']);
      }
      elseif (!isset($options['root'])) {
        $backdrop_site = b_find_site_by_path(getcwd(), $backdrop_root);
      }
    }

    chdir($backdrop_root);
    $backdrop_root = getcwd();
    define('BACKDROP_ROOT', $backdrop_root);
    $_SERVER['HTTP_HOST'] = !empty($backdrop_site) ? $backdrop_site : basename($backdrop_root);

    // Determine if Backdrop is installed or not.
    require_once $backdrop_root . '/core/includes/bootstrap.inc';
    if (function_exists('backdrop_bootstrap_is_installed')) {
      backdrop_settings_initialize();

      if (backdrop_bootstrap_is_installed()) {
        $backdrop_installed = TRUE;
      }
      else {
        if (!$backdrop_site && !empty($multisites)) {
          b_message(bt('The Backdrop site within the multisite installation could not be determined.'), 'log');
        }
        else {
          b_message(bt('Backdrop has not yet been installed.'), 'log');
        }
      }
    }
  }

  // Turn on various 'modes' as appropriate.
  if (!empty($options['yes']) || !empty($options['y'])) {
    $yes_mode = TRUE;
    b_message(bt("'Yes' mode enabled."), 'log');
  }
  if (!empty($options['debug']) || !empty($options['d'])) {
    $debug_mode = TRUE;
    b_message(bt("'Debug' mode enabled."), 'log');
  }
}

/**
 * Bootstrap Backdrop to the specified level.
 *
 * @param int $level
 *   The desired bootstrap level.
 *
 * @return bool
 *   TRUE if Backdrop is bootstrapped to the desired level, FALSE if not.
 */
function b_bootstrap($level) {
  global $backdrop_installed, $backdrop_root;

  if (!$backdrop_installed && $level > B_BOOTSTRAP_PAGE_CACHE) {
    return FALSE;
  }

  if (!empty($backdrop_root) && function_exists('backdrop_bootstrap')) {
    backdrop_bootstrap($level);

    // Backdrop sets its own error handler, so reset ours here.
    set_error_handler('b_error_handler');

    return TRUE;
  }

  return FALSE;
}

/**
 * Add a message for the user to the messages list (to be displayed later).
 *
 * @param string $message
 *   The translated message to display.
 * @param string $type
 *   The type of message to display (this determines the formatting used). One
 *   of: info, success, warning, error, log. Defaults to 'info'.
 *   Note that 'log' messages are only displayed when `debug` mode is enabled.
 */
function b_message($message, $type = 'info') {
  global $messages;

  // Append the message to the `$messages` array.
  $messages[$type][] = $message;
}

/**
 * Allow a string to be translated into other languages.
 *
 * @param string $string
 *   String to process, possibly with replacement items.
 * @param array $args
 *   An associative array of replacement items.
 *
 * @return string
 *   The processed string.
 *
 * @see t()
 */
function bt($string, $args = array()) {
  if (function_exists('t')) {
    return t($string, $args);
  }
  else {
    if (!empty($args)) {
      return strtr($string, $args);
    }
    else {
      return $string;
    }
  }
}

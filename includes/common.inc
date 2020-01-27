<?php
/**
 * @file
 * Common functions.
 */

/**
 * Define bootstrap phase constants.
 */
// 1st phase: Initialize configuration.
define('B_BOOTSTRAP_CONFIGURATION', 0);
// 2nd phase: Try to serve a cached page.
define('B_BOOTSTRAP_PAGE_CACHE', 1);
// 3rd phase: Initialize database layer.
define('B_BOOTSTRAP_DATABASE', 2);
// 4th phase: Initialize the lock system.
define('B_BOOTSTRAP_LOCK', 3);
// 5th phase: Initialize the variable system.
define('B_BOOTSTRAP_VARIABLES', 4);
// 6th phase: Initialize session handling.
define('B_BOOTSTRAP_SESSION', 5);
// 7th phase: Set up the page header.
define('B_BOOTSTRAP_PAGE_HEADER', 6);
// 8th phase: Get page language.
define('B_BOOTSTRAP_LANGUAGE', 7);
// 9th phase: Backdrop is fully loaded; validate and fix input data.
define('B_BOOTSTRAP_FULL', 8);

/**
 * Initialize Backdrop Console.
 */
function b_init() {
  global $options, $multisites;

  // Separate the input into the command, options and arguments, and store them
  // in global variables for later use.
  b_command_parse_input();

  // Set global server variables.
  b_init_globals();

  // Get the root directory of the Backdrop installation to use.
  if (isset($options['root'])) {
    $root = b_find_root($options['root'], FALSE);
  }
  else {
    $root = b_find_root(getcwd());
  }

  if ($root) {
    $site = FALSE;
    // Store array of multisites for future reference, if applicable.
    if ($multisites = b_is_multisite($root)) {
      // Get the site directory of the Backdrop multisite to use.
      if (isset($options['url'])) {
        $site = b_find_site_by_url($options['url'], $root);
      }
      elseif (!isset($options['root'])) {
        $site = b_find_site_by_path(getcwd(), $root);
      }
    }

    // Store the root and site variables for future reference.
    chdir($root);
    $full_path = getcwd();
    define('BACKDROP_ROOT', $full_path);
    $_SERVER['HTTP_HOST'] = basename($full_path);
    if ($site) {
      define('BACKDROP_SITE', $site);
      $_SERVER['HTTP_HOST'] = $site;
    }

    // Determine if Backdrop is installed or not.
    require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';
    if (function_exists('backdrop_bootstrap_is_installed')) {
      backdrop_settings_initialize();

      if (backdrop_bootstrap_is_installed()) {
        b_backdrop_installed(TRUE);
      }
      else {
        b_backdrop_installed(FALSE);

        if (!$_backdrop_site && b_is_multisite($_backdrop_root)) {
          b_set_message(bt('The Backdrop site within the multisite installation could not be determined.'), 'log');
        }
        else {
          b_set_message(bt('Backdrop has not yet been installed.'), 'log');
        }
      }
    }
  }

  // Turn on various 'modes' as appropriate.
  if (isset($options['yes']) || isset($options['y'])) {
    b_yes_mode(TRUE);
    b_set_message(bt('`Yes` mode enabled.'), 'log');
  }
  if (isset($options['debug']) || isset($options['d'])) {
    b_debug_mode(TRUE);
    b_set_message(bt('`Debug` mode enabled.'), 'log');
  }
}

/**
 * Initialize $_SERVER environment variables.
 */
function b_init_globals() {
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

  // If applicable, ensure any appropriate variables are changed to `https`.
  if (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') {
    foreach ($_SERVER as $key => $value) {
      $_SERVER[$key] = str_replace('http://', 'https://', $_SERVER[$key]);
    }
  }
}

/**
 * Bootstrap Backdrop to the specified level.
 *
 * @param integer $level
 *   The desired bootstrap level.
 *
 * @return boolean
 *   TRUE if Backdrop is bootstrapped to the desired level, FALSE if not.
 */
function b_bootstrap($level) {
  if (!b_backdrop_installed() && $level > B_BOOTSTRAP_PAGE_CACHE) {
    return FALSE;
  }

  if (defined('BACKDROP_ROOT') && function_exists('backdrop_bootstrap')) {
    backdrop_bootstrap($level);

    // Backdrop sets its own error handler, so reset ours here.
    set_error_handler('b_errorHandler');

    return TRUE;
  }

  return FALSE;
}

/**
 * An alias for `b_messages()`, defaulting to 'status' type messages.
 *
 * @see b_messages()
 */
function b_set_message($message = FALSE, $type = 'status') {
  b_messages($message, $type);
}

/**
 * Add a message to the `$messages` array.
 *
 * @param string $message
 *   The message to be displayed.
 * @param string $type
 *   The type of message to be displayed. One of: status, success, warning,
 *   error, log.
 *
 * @return array
 *   An array of messages to be displayed, grouped by type.
 */
function b_messages($message = FALSE, $type = FALSE) {
  static $messages = array();

  if ($message) {
    if (!isset($messages[$type])) {
      $messages[$type] = array();
    }
    $messages[$type][] = $message;
  }

  return $messages;
}

/**
 * Allow a string to be translated into other languages.
 *
 * @see t()
 *
 * @param string $string
 *   String to process, possibly with replacement items.
 * @param array $args
 *   An associative array of replacement items.
 *
 * @return string
 *   The processed string.
 */
function bt($string, $args = array()) {
  if (function_exists('t')) {
    // @codingStandardsIgnoreLine -- Need to send variable here.
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

/**
 * Check/set if Backdrop is installed.
 *
 * @param boolean|null $value
 *   The value to set, or NULL to get the existing value.
 *
 * @param boolean
 *   TRUE if Backdrop is installed, FALSE if not.
 */
function b_backdrop_installed($value = NULL) {
  static $installed;

  if (!is_null($value)) {
    $installed = $value;
  }

  return $installed;
}

/**
 * Check/set if `Yes` mode is enabled.
 *
 * @param boolean|null $value
 *   The value to set, or NULL to get the existing value.
 *
 * @param boolean
 *   TRUE if `Yes` mode is enabled, FALSE if not.
 */
function b_yes_mode($value = NULL) {
  static $yes;

  if (!is_null($value)) {
    $yes = $value;
  }

  return $yes;
}

/**
 * Check/set if `Debug` mode is enabled.
 *
 * @param boolean|null $value
 *   The value to set, or NULL to get the existing value.
 *
 * @param boolean
 *   TRUE if `Debug` mode is enabled, FALSE if not.
 */
function b_debug_mode($value = NULL) {
  static $debug;

  if (!is_null($value)) {
    $debug = $value;
  }

  return $debug;
}

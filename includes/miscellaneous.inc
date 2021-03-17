<?php
/**
 * @file
 * Miscellaneous functions.
 */

/**
 * Initialize $_SERVER environment variables.
 */
function bee_initialize_server() {
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
  $_SERVER['HTTP_USER_AGENT'] = 'Bee';
}

/**
 * Initialize Bee.
 */
function bee_initialize_console() {
  global $_bee_options, $_bee_backdrop_installed, $_bee_backdrop_root, $_bee_backdrop_site, $_bee_multisites, $_bee_yes_mode, $_bee_debug_mode;

  // Get the root directory of the Backdrop installation to use.
  if (isset($_bee_options['root'])) {
    $_bee_backdrop_root = bee_find_root($_bee_options['root'], FALSE);
  }
  else {
    $_bee_backdrop_root = bee_find_root(getcwd());
  }

  if ($_bee_backdrop_root) {
    if ($_bee_multisites = bee_is_multisite($_bee_backdrop_root)) {
      // Get the site directory of the Backdrop multisite to use.
      if (isset($_bee_options['site'])) {
        $_bee_backdrop_site = bee_find_site_by_site($_bee_options['site']);
      }
      elseif (!isset($_bee_options['root'])) {
        $_bee_backdrop_site = bee_find_site_by_path(getcwd(), $_bee_backdrop_root);
      }
    }

    chdir($_bee_backdrop_root);
    $_bee_backdrop_root = getcwd();
    define('BACKDROP_ROOT', $_bee_backdrop_root);
    $_SERVER['HTTP_HOST'] = !empty($_bee_backdrop_site) ? $_bee_backdrop_site : basename($_bee_backdrop_root);

    // Determine if Backdrop is installed or not.
    require_once $_bee_backdrop_root . '/core/includes/bootstrap.inc';
    if (function_exists('backdrop_bootstrap_is_installed')) {
      backdrop_settings_initialize();

      if (backdrop_bootstrap_is_installed()) {
        $_bee_backdrop_installed = TRUE;
      }
      else {
        if (!$_bee_backdrop_site && !empty($_bee_multisites)) {
          bee_message(bt('The Backdrop site within the multisite installation could not be determined.'), 'log');
        }
        else {
          bee_message(bt('Backdrop has not yet been installed.'), 'log');
        }
      }
    }
  }

  // Turn on various 'modes' as appropriate.
  if (!empty($_bee_options['yes']) || !empty($_bee_options['y'])) {
    $_bee_yes_mode = TRUE;
    bee_message(bt("'Yes' mode enabled."), 'log');
  }
  if (!empty($_bee_options['debug']) || !empty($_bee_options['d'])) {
    $_bee_debug_mode = TRUE;
    bee_message(bt("'Debug' mode enabled."), 'log');
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
function bee_bootstrap($level) {
  global $_bee_backdrop_installed, $_bee_backdrop_root;

  if (!$_bee_backdrop_installed && $level > BEE_BOOTSTRAP_PAGE_CACHE) {
    return FALSE;
  }

  if (!empty($_bee_backdrop_root) && function_exists('backdrop_bootstrap')) {
    backdrop_bootstrap($level);

    // Backdrop sets its own error handler, so reset ours here.
    set_error_handler('bee_error_handler');

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
function bee_message($message, $type = 'info') {
  global $_bee_messages;

  // Append the message to the `$_bee_messages` array.
  $_bee_messages[$type][] = $message;
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

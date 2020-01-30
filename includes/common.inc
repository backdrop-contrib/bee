<?php
/**
 * @file
 * Common/miscellaneous functions.
 */

/**
 * Initialize Backdrop Console.
 */
function b_init() {
  global $options, $installed, $root, $site, $multisites;

  // Separate the input into the command, options and arguments, and store them
  // for later use.
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
    if ($multisites = b_is_multisite($root)) {
      // Get the site directory of the Backdrop multisite to use.
      if (isset($options['url'])) {
        $site = b_find_site_by_url($options['url'], $root);
      }
      elseif (!isset($options['root'])) {
        $site = b_find_site_by_path(getcwd(), $root);
      }
    }

    chdir($root);
    $root = getcwd();
    $_SERVER['HTTP_HOST'] = basename($root);
    if ($site) {
      $_SERVER['HTTP_HOST'] = $site;
    }

    // Determine if Backdrop is installed or not.
    require_once $root . '/core/includes/bootstrap.inc';
    if (function_exists('backdrop_bootstrap_is_installed')) {
      backdrop_settings_initialize();

      if (backdrop_bootstrap_is_installed()) {
        $installed = TRUE;
      }
      else {
        if (!$site && !empty($multisites)) {
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
    $yes_mode = TRUE;
    b_set_message(bt('`Yes` mode enabled.'), 'log');
  }
  if (isset($options['debug']) || isset($options['d'])) {
    $debug_mode = TRUE;
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
 * @param int $level
 *   The desired bootstrap level.
 *
 * @return bool
 *   TRUE if Backdrop is bootstrapped to the desired level, FALSE if not.
 */
function b_bootstrap($level) {
  global $installed, $root;

  if (!$installed && $level > B_BOOTSTRAP_PAGE_CACHE) {
    return FALSE;
  }

  if (!empty($root) && function_exists('backdrop_bootstrap')) {
    backdrop_bootstrap($level);

    // Backdrop sets its own error handler, so reset ours here.
    set_error_handler('b_error_handler');

    return TRUE;
  }

  return FALSE;
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

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
  $options = b_command_get_options();

  b_init_globals();

  // Get root directory.
  if (isset($options['root'])) {
    $_backdrop_root = b_find_root($options['root'], FALSE);
  }
  else {
    $_backdrop_root = b_find_root(getcwd());
  }

  if ($_backdrop_root) {
    // Get site directory.
    $_backdrop_site = FALSE;
    if (isset($options['url'])) {
      $_backdrop_site = b_find_site_by_url($options['url'], $_backdrop_root);
    }
    elseif (!isset($options['root'])) {
      $_backdrop_site = b_find_site_by_path(getcwd(), $_backdrop_root);
    }

    chdir($_backdrop_root);
    $full_path = getcwd();
    define('BACKDROP_ROOT', $full_path);
    $_SERVER['HTTP_HOST'] = basename($full_path);
    if ($_backdrop_site) {
      define('BACKDROP_SITE', $_backdrop_site);
      $_SERVER['HTTP_HOST'] = $_backdrop_site;
    }
    require_once BACKDROP_ROOT . '/core/includes/bootstrap.inc';

    if (function_exists('backdrop_bootstrap_is_installed')) {
      backdrop_settings_initialize();

      if (backdrop_bootstrap_is_installed()) {
        b_backdrop_installed(TRUE);
      }
      else {
        b_backdrop_installed(FALSE);
        if (!$_backdrop_site && b_is_multisite($_backdrop_root)) {
          b_set_message(bt('The Backdrop site within the multisite installation could not be determined. Some functionality may not be available.'), 'warning');
        }
        else {
          b_set_message(bt('Backdrop has not yet been installed.'), 'warning');
        }
      }
    }
  }

  if (isset($options['drush'])) {
    drush_mode(TRUE);
    b_set_message('Drush mode on');
  }

  if (isset($options['y']) or isset($options['yes'])) {
    b_yes_mode(TRUE);
    b_set_message('Yes mode on');
  }
  if (isset($options['d']) or isset($options['debug'])) {
    b_is_debug(TRUE);
    b_set_message('Debug mode on');
  }
}

/**
 * Initialize $_SERVER environment variables.
 */
function b_init_globals() {
  $host = 'localhost';
  $path = '';

  $_SERVER['HTTP_HOST'] = $host;
  // @codingStandardsIgnoreLine -- Not applicable to use ip_address() instead.
  $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_ADDR'] = '127.0.0.1';
  $_SERVER['SERVER_SOFTWARE'] = NULL;
  $_SERVER['SERVER_NAME'] = 'localhost';
  $_SERVER['REQUEST_URI'] = $path . '/';
  $_SERVER['REQUEST_METHOD'] = 'GET';
  $_SERVER['SCRIPT_NAME'] = $path . '/index.php';
  $_SERVER['PHP_SELF'] = $path . '/index.php';
  $_SERVER['HTTP_USER_AGENT'] = 'Backdrop Console';

  if (!empty($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on') {
    // Ensure that any and all environment variables are changed to https://.
    foreach ($_SERVER as $key => $value) {
      $_SERVER[$key] = str_replace('http://', 'https://', $_SERVER[$key]);
    }
  }
}

/**
 * Bootstrap Backdrop.
 */
function b_bootstrap($bootstrap_level) {
  if (!b_backdrop_installed() && $bootstrap_level > B_BOOTSTRAP_PAGE_CACHE) {
    return FALSE;
  }
  if (defined('BACKDROP_ROOT')) {
    if (function_exists('backdrop_bootstrap')) {
      $status = backdrop_bootstrap($bootstrap_level);
      set_error_handler('b_errorHandler');
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Format and print and error message.
 */
function b_print_error($text) {
  b_render(array(
    '#type' => 'text',
    '#color' => 'red',
    'value' => $text,
  ));
}

/**
 * Add a message to the $messages array.
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
 * An alias for b_messages(), defaulting to 'status' type messages.
 */
function b_set_message($message = FALSE, $type = 'status') {
  b_messages($message, $type);
}

/**
 * Set drush compatible mode and return status.
 */
function drush_mode($set = NULL) {
  static $mode;

  if ($set != NULL) {
    $mode = $set;
  }

  return $mode;
}

/**
 * Set Yes to all mode and return status.
 */
function b_yes_mode($set = NULL) {
  static $mode;

  if ($set != NULL) {
    $mode = $set;
  }

  return $mode;
}

/**
 * If -d specified.
 */
function b_is_debug($set = NULL) {
  static $mode;

  if ($set != NULL) {
    $mode = $set;
  }

  return $mode;
}

/**
 * If backdrop installed, it will return TRUE.
 */
function b_backdrop_installed($set = NULL) {
  static $mode;

  if ($set != NULL) {
    $mode = $set;
  }

  return $mode;
}

/**
 * Asks the user a basic yes/no question.
 *
 * @param string $msg
 *   The question to ask.
 *
 * @return bool
 *   TRUE if the user enters "y" or FALSE if "n".
 */
function b_confirm($msg) {
  $text = array(
    '#color' => 'red',
    'value' => $msg . " (y/N): ",
  );
  render_text($text, FALSE);

  // Automatically accept confirmations if the --yes argument was supplied.
  if (b_yes_mode()) {
    b_print("y");
    return TRUE;
  }

  // See http://drupal.org/node/499758 before changing this.
  $stdin = fopen("php://stdin", "r");

  while ($line = fgets($stdin)) {
    $line = trim($line);
    if (strtolower($line) == 'y') {
      return TRUE;
    }
    if ($line == "") {
      return FALSE;
    }
    if ($line == 'n') {
      return FALSE;
    }
    b_print_prompt($msg . " (y/N): ");
  }
}

/**
 * Ask the user to select an item from a list.
 *
 * From a provided associative array, drush_choice will
 * display all of the questions, numbered from 1 to N,
 * and return the item the user selected. "0" is always
 * cancel; entering a blank line is also interpreted
 * as cancelling.
 *
 * @param array $options
 *   A list of questions to display to the user.  The
 *   KEYS of the array are the result codes to return to the
 *   caller; the VALUES are the messages to display on
 *   each line. Special keys of the form '-- something --' can be
 *   provided as separator between choices groups. Separator keys
 *    don't alter the numbering.
 * @param string $title
 *   The message to display to the user prompting for input.
 */
function b_choice($options, $title) {
  $text = array(
    '#color' => 'blue',
    'value' => $title . ": ",
  );
  $keys = array_keys($options);

  $rows = array();
  // Default Cancel option.
  $rows[] = array(
    array('data' => '[0]', '#color' => 'red'),
    bt('Cancel'),
  );
  foreach ($keys as $num => $value) {
    $rows[] = array(
      array('data' => '[' . ($num + 1) . ']', '#color' => 'bold_blue'),
      $options[$value],
    );
  }

  // Automatically accept confirmations if the --yes argument was supplied.
  if (b_yes_mode()) {
    b_print("y");
    return $keys[0];
  }

  b_render(array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '  :  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  ));

  render_text("");
  render_text($text, FALSE);

  $stdin = fopen("php://stdin", "r");

  while ($line = fgets($stdin)) {
    $line = trim($line);
    $num = (int) $line;

    if ($num == "") {
      return FALSE;
    }
    if ($num == 'n') {
      return FALSE;
    }
    if ($num == 0) {
      return FALSE;
    }

    if (isset($keys[$num - 1])) {
      return $keys[$num - 1];
    }

    render_text($text, FALSE);
  }
}

/**
 * Prompt user for input.
 *
 * @param string $prompt
 *   The prompt for user input.
 *
 * @param string $default
 *   The defualt if the user just hits enter.
 */
function b_prompt($prompt, $default) {
  if (isset($default)) {
    $prompt .= " [" . $default . "]";
  }
  $prompt .= ": ";

  print "\t\e[38;5;62m$prompt\e[0m";

  $stdin = fopen('php://stdin', 'r');

  while (($line = fgets($stdin)) !== FALSE) {
    $line = trim($line);
    if ($line === "") {
      $line = $default;
    }
    if ($line) {
      break;
    }
  }
  fclose($stdin);

  return $line;
}

#!/usr/bin/env php
<?php
/**
 * @file
 * A command line utility for Backdrop CMS.
 */

// Exit gracefully with a meaningful message if installed within a web
// accessible location and accessed in the browser.
if (!bee_is_cli()) {
  echo "<html><head><title>Bee Gone!</title></head><body>";
  echo "<img src='./images/bee.png' align='right' width='150' height='157' style='max-width: 100%;'>";
  echo "<h1>Bee gone!</h1>";
  echo "<p>Bee is a command line tool only and will not work in the browser.</p></body>";
  die();
}

// Set custom error handler.
set_error_handler('bee_error_handler');

// Include files.
require_once __DIR__ . '/includes/miscellaneous.inc';
require_once __DIR__ . '/includes/command.inc';
require_once __DIR__ . '/includes/render.inc';
require_once __DIR__ . '/includes/filesystem.inc';
require_once __DIR__ . '/includes/input.inc';
require_once __DIR__ . '/includes/globals.inc';

// Main execution code.
bee_initialize_server();
bee_parse_input();
bee_initialize_console();
bee_process_command();
bee_print_messages();
bee_display_output();
exit();

/**
 * Custom error handler for `bee`.
 *
 * @param int $errno
 *   The level of the error.
 * @param string $message
 *   Error message to output to the user.
 * @param string $filename
 *   The file that the error came from.
 * @param int $line
 *   The line number the error came from.
 * @param array $context
 *   An array of all variables from where the error was triggered.
 *
 * @see https://www.php.net/manual/en/function.set-error-handler.php
 */
function bee_error_handler($errno, $message, $filename, $line, array $context = NULL) {
  if (error_reporting() > 0) {
    // Core uses the @ error operator in url_stat() to suppress the warning for
    // non-existent files. However, since PHP 8.0, certain errors are no longer
    // suppressed which causes unnecessary error messages to appear in some
    // 'bee' commands.
    if (version_compare(PHP_VERSION, '8', '>=')) {
      $trace = debug_backtrace();
      if (isset($trace[2]) && $trace[2]['function'] == 'url_stat') {
        return;
      }
    }
    echo "$message\n";
    echo " $filename:$line\n\n";
  }
}

/**
 * Detects whether the current script is running in a command-line environment.
 */
function bee_is_cli() {
  return (empty($_SERVER['SERVER_SOFTWARE']) && (php_sapi_name() == 'cli' || (is_numeric($_SERVER['argc']) && $_SERVER['argc'] > 0)));
}

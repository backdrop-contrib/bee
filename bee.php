#!/usr/bin/env php
<?php
/**
 * @file
 * A command line utility for Backdrop CMS.
 */

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
function bee_error_handler($errno, $message, $filename, $line, $context = NULL) {
  if (error_reporting() > 0) {
    // Core uses the @ error operator in url_stat() to suppress the warning for
    // non-existent files. But that doesn't work for us in PHP 8+.
    if (version_compare(PHP_VERSION, '8', '>=')) {
      $trace = debug_backtrace();
      if (isset($trace[5]) && $trace[5]['function'] == 'installer_clear_update_disk_cache') {
        return;
      }
    }
    echo "$message\n";
    echo " $filename:$line\n\n";
  }
}

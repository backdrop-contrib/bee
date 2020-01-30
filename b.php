#!/usr/bin/env php
<?php
/**
 * @file
 * A command line utility for Backdrop CMS.
 */

// Set custom error handler.
set_error_handler('b_error_handler');

// Include files.
require_once __DIR__ . '/includes/globals.inc';
require_once __DIR__ . '/includes/common.inc';
require_once __DIR__ . '/includes/command.inc';
require_once __DIR__ . '/includes/input_output.inc';
require_once __DIR__ . '/includes/render.inc';
require_once __DIR__ . '/includes/filesystem.inc';

// Main execution code.
b_init();
b_command_process();
b_print_messages();
b_render();
exit();

/**
 * Custom error handler for `b`.
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
 */
function b_error_handler($errno, $message, $filename, $line, $context) {
  if (error_reporting() > 0) {
    echo "$message\n";
    echo " $filename:$line\n\n";
  }
}

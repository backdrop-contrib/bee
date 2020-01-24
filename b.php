#!/usr/bin/env php
<?php
/**
 * @file
 * A command line utility for Backdrop CMS.
 */

// Set custom error handler.
set_error_handler('b_errorHandler');

// Include files.
require_once __DIR__ . '/includes/common.inc';
require_once __DIR__ . '/includes/command.inc';
require_once __DIR__ . '/includes/output.inc';
require_once __DIR__ . '/includes/render.inc';
require_once __DIR__ . '/includes/filesystem.inc';

// Global variables.
$global_options = array(
  'root' => array(
    'description' => bt('Specify the root directory of the Backdrop installation to use. If not set, will try to find the Backdrop installation automatically based on the current directory.'),
  ),
  'url' => array(
    'description' => bt('Specify the URL of the Backdrop site to use (as defined in `sites.php`). If not set, will try to find the Backdrop site automatically based on the current directory.'),
  ),
  'drush' => array(
    'description' => bt('Use `.drush.inc` files instead. Drupal 7 Drush commands compatibility.'),
  ),
  'yes' => array(
    'short' => 'y',
    'description' => bt('Answer `yes` automatically to all questions without prompting.'),
  ),
  'debug' => array(
    'short' => 'd',
    'description' => bt('Enables `debug` mode.'),
  ),
);
$command = NULL;
$options = array();
$arguments = array();
$multisites = array();
$elements = array();

// Main execution code.
b_init();
b_command_process();
b_print_messages();
b_render($elements);
exit();

/**
 * Setup a custom error handler.
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
function b_errorHandler($errno, $message, $filename, $line, $context) {
  if (error_reporting() > 0) {
    echo "$message\n";
    echo " $filename:$line\n\n";
  }
}

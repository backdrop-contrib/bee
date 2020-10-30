<?php
/**
 * @file
 * Global variables and constants.
 */

/**
 * Define global variables for storing data needed in various files/functions.
 */
// A list of all available global options for `b` (e.g. not specific to a
// certain command).
$global_options = array(
  'root' => array(
    'description' => bt('Specify the root directory of the Backdrop installation to use. If not set, will try to find the Backdrop installation automatically based on the current directory.'),
  ),
  'url' => array(
    'description' => bt('Specify the URL of the Backdrop site to use (as defined in `sites.php`). If not set, will try to find the Backdrop site automatically based on the current directory.'),
  ),
  'yes' => array(
    'description' => bt('Answer `yes` to questions without prompting.'),
    'short' => 'y',
  ),
  'debug' => array(
    'description' => bt("Enables `debug` mode, in which 'log' type messages will be displayed (in addition to all other messages)."),
    'short' => 'd',
  ),
);

// The command that was given (e.g. `core-status`).
$command = '';

// A list of arguments given to the command.
$arguments = array();

// A list of given options (this includes both global and command-specific
// options).
$options = array();

// A list that maps command aliases to their commands.
$command_aliases = array();

// A boolean indicating whether or not Backdrop is installed.
$backdrop_installed = FALSE;

// The root directory of the Backdrop installation to use.
$backdrop_root = '';

// The site directory of the Backdrop multisite to use.
$backdrop_site = FALSE;

// A list of multisites populated from `sites.php`.
$multisites = array();

// A boolean indicating whether or not `Yes` mode is enabled.
$yes_mode = FALSE;

// A boolean indicating whether or not `Debug` mode is enabled.
$debug_mode = FALSE;

// A list of messages to display to the user, grouped by type.
$messages = array();

// A collection of elements to render and display as a result of processing
// the given command.
$output = array();

/**
 * Define constants for bootstrap phases.
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

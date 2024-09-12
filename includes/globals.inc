<?php
/**
 * @file
 * Global variables and constants.
 */

/**
 * Define global variables for storing data needed in various files/functions.
 */
// A list of all available global options for `bee` (e.g. not specific to a
// certain command).
$_bee_global_options = array(
  'root' => array(
    'description' => bt('Specify the root directory of the Backdrop installation to use. If not set, will try to find the Backdrop installation automatically based on the current directory.'),
  ),
  'site' => array(
    'description' => bt("Specify the directory name or URL of the Backdrop site to use (as defined in 'sites.php'). If not set, will try to find the Backdrop site automatically based on the current directory."),
  ),
  'base-url' => array(
    'description' => bt("Specify the base URL of the Backdrop site, such as https://example.com. May be useful with commands that output URLs to pages on the site."),
  ),
  'yes' => array(
    'description' => bt("Answer 'yes' to questions without prompting."),
    'short' => 'y',
  ),
  'debug' => array(
    'description' => bt("Enables 'debug' mode, in which 'debug' and 'log' type messages will be displayed (in addition to all other messages)."),
    'short' => 'd',
  ),
);

// The command that was given (e.g. `core-status`).
$_bee_command = '';

// A list of arguments given to the command.
$_bee_arguments = array();

// A list of given options (this includes both global and command-specific
// options).
$_bee_options = array();

// A list that maps command aliases to their commands.
$_bee_command_aliases = array();

// A boolean indicating whether or not Backdrop is installed.
$_bee_backdrop_installed = FALSE;

// The root directory of the Backdrop installation to use.
$_bee_backdrop_root = '';

// The site directory of the Backdrop multisite to use.
$_bee_backdrop_site = FALSE;

// A list of multisites populated from `sites.php`.
$_bee_multisites = array();

// A boolean indicating whether or not `Yes` mode is enabled.
$_bee_yes_mode = FALSE;

// A boolean indicating whether or not `Debug` mode is enabled.
$_bee_debug_mode = FALSE;

// A constant indicating the bootstrap level of the current command.
$_bee_bootstrap_level = -1;

// A list of messages to display to the user, grouped by type.
$_bee_messages = array();

// A collection of elements to render and display as a result of processing
// the given command.
$_bee_output = array();

/**
 * The useragent string used for HTTP requests via cURL.
 */
define('BEE_USERAGENT', 'Bee - the command line tool for Backdrop CMS');

/**
 * Define constants for bootstrap phases.
 */
// No bootstrap.
define('BEE_BOOTSTRAP_NONE', $_bee_bootstrap_level);

// 1st phase: Initialize configuration.
define('BEE_BOOTSTRAP_CONFIGURATION', 0);

// 2nd phase: Try to serve a cached page.
define('BEE_BOOTSTRAP_PAGE_CACHE', 1);

// 3rd phase: Initialize database layer.
define('BEE_BOOTSTRAP_DATABASE', 2);

// 4th phase: Initialize the lock system.
define('BEE_BOOTSTRAP_LOCK', 3);

// 5th phase: Initialize the variable system.
define('BEE_BOOTSTRAP_VARIABLES', 4);

// 6th phase: Initialize session handling.
define('BEE_BOOTSTRAP_SESSION', 5);

// 7th phase: Set up the page header.
define('BEE_BOOTSTRAP_PAGE_HEADER', 6);

// 8th phase: Get page language.
define('BEE_BOOTSTRAP_LANGUAGE', 7);

// 9th phase: Backdrop is fully loaded; validate and fix input data.
define('BEE_BOOTSTRAP_FULL', 8);

/**
 * Define constants for Bee's current version and latest release.
 */
// Current version of Bee.
// This will normally be based on the next minor release (e.g '1.x-1.1.x-dev')
// but will be changed briefly when a release is generated.
define('BEE_VERSION', '1.x-1.2.x-dev');

// Latest release.
// This will be updated at each release.
define('BEE_LATEST_RELEASE', '1.x-1.1.0');

/**
 * Define constants for symbols to be used within Bee.
 */
// Bee (U+1F41D, Honeybee)
define('BEE_SYMBOL_BEE', '🐝 ');

// Success (U+2714, Heavy Check Mark)
define('BEE_SYMBOL_SUCCESS', '✔ ');

// Warning (U+26A0, Warning Sign)
define('BEE_SYMBOL_WARNING', '⚠️ ');

// Error (U+2718, Heavy Ballot X)
define('BEE_SYMBOL_ERROR', '✘ ');

// Info (U+2139, Information Source)
define('BEE_SYMBOL_INFO', 'ℹ ');

// Debug (U+1F575, Sleuth or Spy)
define('BEE_SYMBOL_DEBUG', '🕵 ');

// Log (U+1F4DD, Memo)
define('BEE_SYMBOL_LOG', '📝 ');

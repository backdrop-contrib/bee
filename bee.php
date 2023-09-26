#!/usr/bin/env php
<?php
/**
 * @file
 * A command line utility for Backdrop CMS.
 */

// Exit gracefully with a meaningful message if installed within a web
// accessible location and accessed in the browser.
if (!bee_is_cli()) {
  // Set the title to use in h1 and title elements.
  $title = "Bee Gone!";
  // Place a white block over "#!/usr/bin/env php" as this is output before
  // anything else.
  $browser_output = "<div style='background-color:white;position:absolute;width:15rem;height:3rem;top:0;left:0;z-index:9;'>&nbsp;</div>";
  // Add the bee logo and style appropriately.
  $browser_output .= "<img src='./images/bee.png' align='right' width='150' height='157' style='max-width:100%;margin-top:3rem;'>";
  // Add meaningful text.
  $browser_output .= "<h1 style='font-family:Tahoma;'>$title</h1>";
  $browser_output .= "<p style='font-family:Verdana;'>Bee is a command line tool only and will not work in the browser.</p>";
  // Add the document title using javascript when the window loads.
  $browser_output .= "<script>window.onload = function(){document.title='$title';}</script>";
  // Output the combined string.
  echo $browser_output;
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

<?php
/**
 * @file
 * The command for displaying the current version of Bee.
 */

/**
 * Define the path for the CHANGELOG.
 */
define('BEE_CHANGELOG_PATH', __DIR__ . '/../CHANGELOG.md');

/**
 * Implements hook_bee_command().
 */
function version_bee_command() {
  return array(
    'version' => array(
      'description' => bt('Display the current version of Bee.'),
      'callback' => 'version_bee_callback',
      'group' => 'information',
      'examples' => array(
        'bee version' => bt('Output the current version.'),
      ),
    ),
    'changelog' => array(
      'description' => bt('Display the current CHANGELOG of Bee.'),
      'callback' => 'changelog_bee_callback',
      'group' => 'information',
      'examples' => array(
        'bee changelog' => bt('Output the current CHANGELOG.'),
      ),
    ),
  );
}

/**
 * Command callback: Version.
 */
function version_bee_callback($arguments, $options) {
  $current_version = BEE_VERSION;
  $latest_release = BEE_LATEST_RELEASE;

  // If the version is the dev version, then we also want the date of the last
  // change.
  $dev_version = FALSE;
  // Current version will only match latest release for the actual release
  // packages so any difference will indicate the dev version is in use.
  if ($current_version != $latest_release) {
    $dev_version = TRUE;
    // Get the CHANGELOG lines as an array so we can check each line.
    $changelog_lines = file(BEE_CHANGELOG_PATH);
    // Define the search string to identify the line.
    $search_string = "## [Unreleased] - ";
    // Loop through each line and look for the search string.
    foreach ($changelog_lines as $line) {
      if (strpos($line, $search_string) !== FALSE) {
        // Extract the latest date from this line.
        $unreleased_date = substr($line, 18, 10);
        // Append it to the current version.
        $current_version .= ' - ' . $unreleased_date;
        // Stop processing more lines.
        break;
      }
    }
  }

  $output = array();
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => "\n " . BEE_SYMBOL_BEE . ' ' . bt('Bee for Backdrop CMS'),
    ),
  );
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => ' ' . bt('Version: ') . $current_version,
    ),
  );
  $output[] = array(
    'type' => 'text',
    'variables' => array(
      'value' => ' ' . bt('Latest Release: ') . $latest_release . "\n",
    ),
  );
  return $output;
}

/**
 * Command callback: Changelog.
 */
function changelog_bee_callback($arguments, $options) {
  echo file_get_contents(BEE_CHANGELOG_PATH);
}

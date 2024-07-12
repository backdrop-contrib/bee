<?php
/**
 * @file
 * The command for displaying the current version of Bee.
 */

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
  );
}

/**
 * Command callback: Run cron.
 */
function version_bee_callback($arguments, $options) {
  $current_version = BEE_VERSION;
  $latest_release = BEE_LATEST_RELEASE;

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

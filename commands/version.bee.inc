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
  bee_message(bt('Bee for Backdrop CMS - Version ' . $current_version), 'success');
}

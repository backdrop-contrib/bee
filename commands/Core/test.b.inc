<?php
/**
 * @file
 * Command(s) for providing an example of implementation.
 */

/**
 * Implements hook_b_command().
 */
function test_b_command() {
  $items['core-status'] = array(
    'description' => 'This is test example how to add your own callback on command.',
    'callback' => 'test_b_callback',
    'aliases' => array('status', 'st'),
  );

  return $items;
}

/**
 * Callback function for hook_b_command().
 */
function test_b_callback($arguments, $options) {
  if (defined('BACKDROP_ROOT') && file_exists(BACKDROP_ROOT . '/index.php')) {
    if (b_is_multisite(BACKDROP_ROOT)) {
      b_set_message('Backdrop multisite installation detected.', 'status');
    }
    else {
      b_set_message('Backdrop installation detected.', 'status');
    }
  }
  else {
    b_set_message('Backdrop installation not found.', 'warning');
  }
}

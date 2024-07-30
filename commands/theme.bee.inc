<?php
/**
 * @file
 * Command(s) for working with themes.
 */

/**
 * Implements hook_bee_command().
 */
function theme_bee_command() {
  return array(
    'theme-enable-debug' => array(
      'description' => bt('Enable theme debugging.'),
      'callback' => 'theme_bee_enable_debug_callback',
      'group' => 'themes',
      'aliases' => array('ted'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee theme-enable-debug' => bt('Enable theme debugging.'),
      ),
    ),
    'theme-disable-debug' => array(
      'description' => bt('Disable theme debugging.'),
      'callback' => 'theme_bee_disable_debug_callback',
      'group' => 'themes',
      'aliases' => array('tdd'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee theme-disable-debug' => bt('Disable theme debugging.'),
      ),
    ),
  );
}

/**
 * Command callback: Enable theme debugging.
 */
function theme_bee_enable_debug_callback($arguments, $options) {
  config_set('system.core', 'theme_debug', 1);

  bee_message(bt('Theme debugging has been turned on.'), 'success');
}

/**
 * Command callback: Enable theme debugging.
 */
function theme_bee_disable_debug_callback($arguments, $options) {
  config_set('system.core', 'theme_debug', 0);

  bee_message(bt('Theme debugging has been turned off.'), 'success');
}

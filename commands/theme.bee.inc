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
    'theme-default' => array(
      'description' => bt('Set the default theme.'),
      'callback' => 'theme_default_bee_callback',
      'group' => 'themes',
      'arguments' => array(
        'theme' => bt('The theme to set as the default theme. It will be enabled, if not already.'),
      ),
      'aliases' => array('default-theme'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee theme-default bartik' => bt('Set Bartik as the default theme.'),
      ),
    ),
    'theme-admin' => array(
      'description' => bt('Set the admin theme.'),
      'callback' => 'theme_admin_bee_callback',
      'group' => 'themes',
      'arguments' => array(
        'theme' => bt('The theme to set as the admin theme. It will be enabled, if not already.'),
      ),
      'aliases' => array('admin-theme'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee theme-admin basis' => bt('Set Basis as the admin theme.'),
      ),
    ),
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
 * Command callback: Set the default theme.
 */
function theme_default_bee_callback($arguments, $options) {
  // Get theme info.
  $theme_name = $arguments['theme'];
  $themes = system_rebuild_theme_data();
  if (!isset($themes[$theme_name])) {
    bee_message(bt("The '!name' theme could not be found.", array(
      '!name' => $theme_name,
    )), 'error');
    return;
  }
  $theme = $themes[$theme_name];

  // Enable if not already.
  if (!$theme->status) {
    theme_enable(array($theme_name));
  }

  // Set as default theme.
  config_set('system.core', 'theme_default', $theme_name);

  // Display success message.
  bee_message(bt("'!name' was set as the default theme.", array(
    '!name' => $theme->info['name'],
  )), 'success');

  // Flush caches.
  backdrop_flush_all_caches();
}

/**
 * Command callback: Set the admin theme.
 */
function theme_admin_bee_callback($arguments, $options) {
  // Get theme info.
  $theme_name = $arguments['theme'];
  $themes = system_rebuild_theme_data();
  if (!isset($themes[$theme_name])) {
    bee_message(bt("The '!name' theme could not be found.", array(
      '!name' => $theme_name,
    )), 'error');
    return;
  }
  $theme = $themes[$theme_name];

  // Enable if not already.
  if (!$theme->status) {
    theme_enable(array($theme_name));
  }

  // Set as admin theme.
  config_set('system.core', 'admin_theme', $theme_name);

  // Display success message.
  bee_message(bt("'!name' was set as the admin theme.", array(
    '!name' => $theme->info['name'],
  )), 'success');

  // Flush caches.
  backdrop_flush_all_caches();
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

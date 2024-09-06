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
    'theme-debug' => array(
      'description' => bt('Enable or disable "Theme debug" for Backdrop.'),
      'callback' => 'theme_debug_bee_callback',
      'group' => 'themes',
      'arguments' => array(
        'value' => bt('A boolean value to enable (true/1) or disable (false/0) theme debug. Omitting the value will return the current theme debug status.'),
      ),
      'optional_arguments' => array('value'),
      'aliases' => array('td'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee theme-debug true' => bt('Enable theme debug for the site (not case-sensitive).'),
        'bee theme-debug FALSE' => bt('Disable theme debug for the site (not case-sensitive).'),
        'bee theme-debug 1' => bt('Enable theme debug for the site.'),
        'bee theme-debug 0' => bt('Disable theme debug for the site.'),
        'bee theme-debug' => bt('Get the theme debug status for the site.'),
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
 * Command callback: Set the value of theme debug for Backdrop.
 */
function theme_debug_bee_callback($arguments, $options) {
  // Check if a value has been provided.
  if (isset($arguments['value'])) {
    // Check for valid boolean value - allow true/false AND 1/0.
    $value = strtoupper($arguments['value']);
    switch ($value) {
      case 'TRUE':
        $value = (bool) TRUE;
        break;
      case '1':
        $value = (bool) TRUE;
        break;
      case 'FALSE':
        $value = (bool) FALSE;
        break;
      case '0':
        $value = (bool) FALSE;
        break;
      default:
        $err_msg = bt("'!value' is not a valid value. Enter 'TRUE', 'FALSE' (not case sensitive), '1' or '0'. Omit providing a value to get the current value.", array (
          '!value' => (string) $value,)
        );
        bee_message((string) $err_msg, 'error');
        return;
    }
    // Check if argument matches existing value.
    if ((bool) config_get('system.core', 'theme_debug') == $value) {
      $msg = bt('Theme debug is already !value', array(
        '!value' => ($value) ? 'enabled' : 'disabled',
      ));
      bee_message((string) $msg, 'status');
    }
    else {
      // Attempt to set maintenance mode.
      try {
        config_set('system.core', 'theme_debug', $value);
        $msg = bt('Theme debug was !value', array(
          '!value' => ($value) ? 'enabled' : 'disabled',
        ));
        bee_message((string) $msg, 'success');
      }
      catch (ParseError $e) {
        // This is more readable than the default error we would get from PHP.
        $err_msg = bt('!msg in: !value', array(
          '!msg' => $e->getMessage(),
          '!value' => $arguments['value'],
        ));
        bee_message((string) $err_msg, 'error');
      }
    }
  }
  else {
    // No value provided - get status.
    $value = (bool) config_get('system.core', 'theme_debug');
    $msg = bt('Theme debug is !status', array(
      '!status' => ($value) ? 'enabled' : 'disabled',
    ));
    bee_message((string) $msg, 'info');
  }
}

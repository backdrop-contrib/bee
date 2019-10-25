<?php
/**
 * @file
 * Command(s) for getting an overview of a Backdrop installation.
 */

/**
 * Implements hook_b_command().
 */
function status_b_command() {
  $items = array();
  // We specify command callbacks here because the defaults would collide with
  // the drush cache api functions.
  $items['core-status'] = array(
    'description' => 'Provides a birds-eye view of the current Backdrop installation, if any.',
    'bootstrap' => B_BOOTSTRAP_FULL,
    'aliases' => array('status', 'st'),
    'examples' => array(
      'b core-status ' => 'Show status.',
      'b core-status --show-passwords' => 'Show status include password.',
    ),
    'options' => array(
      'show-passwords' => 'Show database password.',
    ),
    'callback' => 'status_b_callback',
  );

  return $items;
}

/**
 * Callback function for hook_b_command().
 */
function status_b_callback($arguments, $options) {
  $rows = array();
  if (defined('BACKDROP_VERSION')) {
    $color = 'green';
    $rows[] = array(
      'Backdrop CMS',
      array('data' => BACKDROP_VERSION, '#color' => $color),
    );
  }

  if (defined('BACKDROP_ROOT')) {
    $rows[] = array('Backdrop root', BACKDROP_ROOT);
  }
  if (defined('BACKDROP_SITE')) {
    $rows[] = array('Backdrop site', BACKDROP_SITE);
  }
  if (class_exists('Database')) {
    if ($connection = Database::getConnection('default', 'default')) {
      $conn = $connection->getConnectionOptions();
      $rows[] = array('Database driver', $conn['driver']);
      $rows[] = array('Database host', $conn['host']);

      $rows[] = array('Database name', $conn['database']);
      $rows[] = array('Database username', $conn['username']);
      if (isset($options['show-passwords']) or isset($options['full'])) {
        $rows[] = array(
          'Database password',
          array('data' => $conn['password'], '#color' => 'red'),
        );
      }
    }
  }
  else {
    b_set_message('No database connection', 'warning');
  }
  if (function_exists('conf_path')) {
    $settings_path = realpath(conf_path());
    $rows[] = array('Settings.php path', $settings_path);
    include $settings_path . '/settings.php';
    $rows[] = array('Drupal compability', $settings['backdrop_drupal_compatibility']);
    $rows[] = array('Config dir active', $config_directories['active']);
    $rows[] = array('Config dir staging', $config_directories['staging']);
  }
  if (function_exists('config_get')) {
    $default_theme = config_get('system.core', 'theme_default');
    $rows[] = array('Default theme', $default_theme);
    $files_public = config_get('system.core', 'file_public_path');
    $rows[] = array('Public files path', $files_public);
    $files_private = config_get('system.core', 'file_private_path');
    if (!empty($files_private)) {
      $rows[] = array('Private files path', $files_private);
    }
  }
  if (function_exists('state_get') && function_exists('format_interval')) {
    $cron_last = state_get('cron_last');
    $ago = format_interval(time() - $cron_last);
    $rows[] = array('Cron run latest', $ago);
  }
  $rows[] = array('PHP version', PHP_VERSION );
  $rows[] = array('PHP OS', PHP_OS);

  return array(
    '#type' => 'table',
    'rows' => $rows,
    'delimiter' => '  :  ',
    'delimiter_left' => ' ',
    'delimiter_right' => ' ',
  );
}

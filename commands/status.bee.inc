<?php
/**
 * @file
 * Command(s) for getting an overview of a Backdrop installation.
 */

/**
 * Implements hook_bee_command().
 */
function status_bee_command() {
  return array(
    'status' => array(
      'description' => bt('Provides an overview of the current Backdrop installation/site.'),
      'callback' => 'status_bee_callback',
      'options' => array(
        'show-password' => array(
          'description' => bt('Show the database password.'),
        ),
      ),
      'aliases' => array('st', 'info', 'core-status'),
      'examples' => array(
        'bee status' => bt('Get an overview of the Backdrop installation.'),
        'bee status --show-password' => bt('Include the database password in the overview.'),
      ),
    ),
  );
}

/**
 * Command callback: Provides an overview of the current Backdrop
 * installation/site.
 */
function status_bee_callback($arguments, $options) {
  global $_bee_backdrop_root, $_bee_backdrop_site, $_bee_multisites;
  $rows = array();

  if (!empty($_bee_backdrop_root)) {
    // Backdrop core.
    global $settings, $config_directories;
    if (defined('BACKDROP_VERSION')) {
      $rows[] = array(
        array('value' => bt('Backdrop CMS')),
        array('value' => BACKDROP_VERSION),
      );
    }
    $rows[] = array(
      array('value' => bt('Bee version')),
      array('value' => BEE_VERSION),
    );
    $rows[] = array(
      array('value' => bt('Root directory')),
      array('value' => $_bee_backdrop_root),
    );

    // Multisite.
    $rows[] = array(
      array('value' => bt('Site type')),
      array('value' => empty($_bee_multisites) ? bt('Single') : bt('Multisite')),
    );
    if (!empty($_bee_backdrop_site)) {
      $rows[] = array(
        array('value' => bt('Site directory')),
        array('value' => $_bee_backdrop_site),
      );
    }

    // Database.
    if (class_exists('Database')) {
      $db_connection = Database::getConnectionInfo();
      if (!empty($db_connection['default']) && $info = $db_connection['default']) {
        $rows[] = array(
          array('value' => bt('Database')),
          array('value' => $info['driver']),
        );
        $rows[] = array(
          array('value' => bt('Database name')),
          array('value' => rawurldecode($info['database'])),
        );
        $rows[] = array(
          array('value' => bt('Database username')),
          array('value' => rawurldecode($info['username'])),
        );
        $rows[] = array(
          array('value' => bt('Database password')),
          array('value' => isset($options['show-password']) ? rawurldecode($info['password']) : '**********'),
        );
        $rows[] = array(
          array('value' => bt('Database host')),
          array('value' => $info['host']),
        );
        $rows[] = array(
          array('value' => bt('Database port')),
          array('value' => $info['port']),
        );

        // States directly from database, as state_get() requires a higher
        // bootstrap level with caching and locking.
        $states = db_query('SELECT name, value FROM {state}')->fetchAllKeyed();
        foreach ($states as $name => $serialized) {
          switch ($name) {
            case 'cron_last':
              $rows[] = array(
                array('value' => bt('Cron last run')),
                array('value' => bee_format_date(unserialize($serialized))),
              );
              break;

            case 'install_time':
              $rows[] = array(
                array('value' => bt('Install time')),
                array('value' => bee_format_date(unserialize($serialized))),
              );
              break;

            case 'maintenance_mode':
              $rows[] = array(
                array('value' => bt('Maintenance mode')),
                array('value' => (unserialize($serialized)) ? 'on' : 'off'),
              );
              break;

            case 'update_last_check':
              $rows[] = array(
                array('value' => bt('Update last check')),
                array('value' => bee_format_date(unserialize($serialized))),
              );
              break;

          }
        }
      }
    }

    // Global settings.
    $rows[] = array(
      array('value' => bt('Settings.php path')),
      array('value' => realpath(conf_path()) . '/settings.php'),
    );
    $rows[] = array(
      array('value' => bt('Drupal compatibility')),
      array('value' => ($settings['backdrop_drupal_compatibility']) ? 'on' : 'off'),
    );
    if ($config_directories['active'] && is_dir($config_directories['active'])) {
      $rows[] = array(
        array('value' => bt('Config dir active')),
        array('value' => realpath($config_directories['active'])),
      );
    }
    if ($config_directories['staging'] && is_dir($config_directories['staging'])) {
      $rows[] = array(
        array('value' => bt('Config dir staging')),
        array('value' => realpath($config_directories['staging'])),
      );
    }

    // Config including overridden values.
    if (!function_exists('config')) {
      include $_bee_backdrop_root . '/core/includes/config.inc';
    }
    $config_core = config('system.core');
    if (!$config_core->isNew()) {
      $rows[] = array(
        array('value' => bt('Site name')),
        array('value' => $config_core->get('site_name')),
      );
      $rows[] = array(
        array('value' => bt('Default theme')),
        array('value' => $config_core->get('theme_default')),
      );
      $rows[] = array(
        array('value' => bt('Admin theme')),
        array('value' => $config_core->get('admin_theme')),
      );
      if ($files_public = $config_core->get('file_public_path')) {
        $rows[] = array(
          array('value' => bt('Public files path')),
          array('value' => realpath($files_public)),
        );
      }
      if ($files_private = $config_core->get('file_private_path')) {
        $rows[] = array(
          array('value' => bt('Private files path')),
          array('value' => realpath($files_private)),
        );
      }
      if ($files_temp = $config_core->get('file_temporary_path')) {
        $rows[] = array(
          array('value' => bt('Temporary files path')),
          array('value' => realpath($files_temp)),
        );
      }
      $rows[] = array(
        array('value' => bt('Preprocess CSS')),
        array('value' => ($config_core->get('preprocess_css')) ? 'on' : 'off'),
      );
      $rows[] = array(
        array('value' => bt('Preprocess JS')),
        array('value' => ($config_core->get('preprocess_js')) ? 'on' : 'off'),
      );
      $rows[] = array(
        array('value' => bt('Theme debug')),
        array('value' => ($config_core->get('theme_debug')) ? 'on' : 'off'),
      );
      $rows[] = array(
        array('value' => bt('Error display level')),
        array('value' => $config_core->get('error_level')),
      );
    }

    // PHP command line info.
    $rows[] = array(
      array('value' => bt('PHP cli version')),
      array('value' => PHP_VERSION),
    );
    $rows[] = array(
      array('value' => bt('PHP ini path')),
      array('value' => php_ini_loaded_file()),
    );

    echo "\n";
    return array(
      array(
        'type' => 'table',
        'variables' => array(
          'rows' => $rows,
          'delimiter' => '   ',
          'delimiter_left' => ' ',
          'delimiter_right' => ' ',
        ),
      ),
    );
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}

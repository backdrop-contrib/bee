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
      'command_requirements' => array(
        'backdrop_root' => TRUE,
        'backdrop_installed' => FALSE,
      ),
      'group' => 'information',
      'options' => array(
        'show-password' => array(
          'description' => bt('Show the database password.'),
          'short' => 'p',
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

  // Initialize the $rows array.
  $rows = array();

  // Set $telemetry_status to FALSE initially in case we can't retrieve status
  // from database.
  $telemetry_status = FALSE;

  if (!empty($_bee_backdrop_root)) {
    global $settings, $config_directories;
    $rows[] = array(
      array('value' => BEE_SYMBOL_BEE),
      array('value' => ''),
    );
    // Backdrop core.
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
      array('value' => bt('Bee latest release')),
      array('value' => BEE_LATEST_RELEASE),
    );
    $rows[] = array(
      array('value' => bt('Bee root directory')),
      array('value' => substr(__DIR__, 0, strlen(__DIR__) - 9)),
    );
    $rows[] = array(
      array('value' => bt('Site root directory')),
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
        $status_show_password = isset($options['show-password']);
        $rows[] = array(
          array('value' => bt('Database password')),
          array('value' => $status_show_password ? rawurldecode($info['password']) : '**********'),
        );
        $rows[] = array(
          array('value' => bt('Database host')),
          array('value' => $info['host']),
        );
        $rows[] = array(
          array('value' => bt('Database port')),
          array('value' => array_key_exists('port', $info) ? $info['port'] : NULL),
        );

        try {
          // Check if telemetry is enabled.
          $telemetry_query = "SELECT `status` FROM {system} WHERE `name` = 'telemetry'";
          $telemetry_module = (bool) db_query($telemetry_query)->fetchField();
          $telemetry_override = isset($settings['telemetry_enabled']) ? $settings['telemetry_enabled'] : TRUE;
          $telemetry_status = $telemetry_module && $telemetry_override;

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

              case 'bee_telemetry_last':
                $bee_telemetry_last = bee_format_date(unserialize($serialized));
            }
          }
        }
        // If the database query fails then add a simple row about database
        // status and a warning message.
        catch (Exception $e) {
          $rows[] = array(
            array('value' => bt('Database status')),
            array('value' => bt('Empty')),
          );
          bee_message(bt('The database is currently empty.'), 'warning');
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
    // Active config storage.
    $config_active_class = isset($settings['config_active_class']) ? $settings['config_active_class'] : 'ConfigFileStorage';
    $config_active_storage = '';
    if ($config_active_class == 'ConfigDatabaseStorage') {
      $config_active_storage = 'Database';
    }
    else {
      if ($config_directories['active'] && is_dir($config_directories['active'])) {
        $config_active_storage = realpath($config_directories['active']);
      }
    }
    if ($config_active_storage) {
      $rows[] = array(
        array('value' => bt('Config storage active')),
        array('value' => $config_active_storage),
      );
    }
    // Staging config storage.
    $config_staging_class = isset($settings['config_staging_class']) ? $settings['config_staging_class'] : 'ConfigFileStorage';
    $config_staging_storage = '';
    if ($config_staging_class == 'ConfigDatabaseStorage') {
      $config_staging_storage = 'Database';
    }
    else {
      if ($config_directories['staging'] && is_dir($config_directories['staging'])) {
        $config_staging_storage = realpath($config_directories['staging']);
      }
    }
    if ($config_staging_storage) {
      $rows[] = array(
        array('value' => bt('Config storage staging')),
        array('value' => $config_staging_storage),
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

    // Telemetry.
    $rows[] = array(
      array('value' => ''),
      array('value' => ''),
    );
    $rows[] = array(
      array('value' => BEE_SYMBOL_INFO . ' ' . bt('TELEMETRY')),
      array('value' => ''),
    );
    $rows[] = array(
      array('value' => bt('Status')),
      array('value' => $telemetry_status ? 'on' : 'off'),
    );
    if ($telemetry_status) {
      $rows[] = array(
        array('value' => bt('Last sent')),
        array('value' => $bee_telemetry_last),
      );
    }
    if ($telemetry_status) {
      $data_summary = json_encode(bee_telemetry_get_data());
    }
    else {
      $data_summary = bt('None');
    }
    $data_string = " Data: $data_summary\n";

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
      array(
        'type' => 'text',
        'variables' => array(
          'value' => $data_string,
        ),
      ),
    );
  }
  else {
    // No Backdrop root found.
    bee_message(bt("No Backdrop installation found. Run this command again from within a Backdrop installation, or use the '--root' global option."), 'warning');
  }
}

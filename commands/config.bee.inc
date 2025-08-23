<?php
/**
 * @file
 * Command(s) for configuration management.
 */

/**
 * Implements hook_bee_command().
 */
function config_bee_command() {
  return array(
    'config-get' => array(
      'description' => bt('Get the value of a specific config option, or view all the config options in a given file.'),
      'callback' => 'config_get_bee_callback',
      'group' => 'configuration',
      'arguments' => array(
        'file' => bt("The name of the config object to retrieve. This is the name of the config file, less the '.json' suffix."),
        'option' => bt('The name of the config option within the file to read. This may contain periods to indicate levels within the config file. Leave blank to display the entire config file.'),
      ),
      'optional_arguments' => array('option'),
      'aliases' => array('cget'),
      'bootstrap' => BEE_BOOTSTRAP_CONFIGURATION,
      'examples' => array(
        'bee config-get system.core site_name' => bt("Get the value of the 'site_name' config option."),
        'bee config-get devel.settings' => bt('See all the config options for the Devel module.'),
      ),
    ),
    'config-set' => array(
      'description' => bt('Set the value of an option in a config file.'),
      'callback' => 'config_set_bee_callback',
      'group' => 'configuration',
      'arguments' => array(
        'file' => bt("The name of the config object to retrieve. This is the name of the config file, less the '.json' suffix."),
        'option' => bt('The name of the config option within the file to set. This may contain periods to indicate levels within the config file.'),
        'value' => bt('The value to save in the config file.'),
      ),
      'aliases' => array('cset'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee config-set image.style.thumbnail effects.0.data.width 200' => bt('Change the width of the Thumbnail image style.'),
      ),
    ),
    'config-clear' => array(
      'description' => bt('Clear the value of an option in a config file.'),
      'callback' => 'config_clear_bee_callback',
      'group' => 'configuration',
      'arguments' => array(
        'file' => bt("The name of the config object to retrieve. This is the name of the config file, less the '.json' suffix."),
        'option' => bt('The name of the config option within the file to clear. This may contain periods to indicate levels within the config file.'),
      ),
      'aliases' => array('cclr'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee config-clear somefile somevalue' => bt('Clear the option somevalue in the file somefile.json.'),
      ),
    ),
    'config-import' => array(
      'description' => bt('Import config into the site.'),
      'callback' => 'config_import_bee_callback',
      'group' => 'configuration',
      'aliases' => array('cim', 'bcim'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee config-import' => bt("Imports config (from 'staging' to 'active') for the current site."),
      ),
    ),
    'config-export' => array(
      'description' => bt('Export config from the site.'),
      'callback' => 'config_export_bee_callback',
      'group' => 'configuration',
      'aliases' => array('cex', 'bcex'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee config-export' => bt("Exports config (from 'active' to 'staging') for the current site."),
      ),
    ),
  );
}

/**
 * Command callback: Get the value of a specific config option, or view all the
 * config options in a given file.
 */
function config_get_bee_callback($arguments, $options) {
  // Get the config value.
  $value = config_get($arguments['file'], $arguments['option']);

  // Display errors for blank/missing values.
  if (!empty($arguments['option']) && is_null($value)) {
    bee_message(bt("'!option' could not be found in the '!file' config file.", array(
      '!option' => $arguments['option'],
      '!file' => $arguments['file'],
    )), 'error');
    return;
  }
  elseif (empty($arguments['option']) && empty($value)) {
    bee_message(bt("The '!file' config file could not be found.", array(
      '!file' => $arguments['file'],
    )), 'error');
    return;
  }

  // Display the value.
  return array(
    array(
      'type' => 'text',
      'variables' => array(
        'value' => var_export($value, TRUE),
      ),
    ),
  );
}

/**
 * Command callback: Set the value of an option in a config file.
 */
function config_set_bee_callback($arguments, $options) {
  $config = config($arguments['file']);

  // Ask to create file if it doesn't exist.
  if (empty($config->get())) {
    $answer = bee_confirm(bt("The '!file' config file doesn't exist. Would you like to create it?", array(
      '!file' => $arguments['file'],
    )), FALSE);
    if (!$answer) {
      return;
    }
  }

  // Check the existing value and type.
  $existing_value = $config->get($arguments['option']);
  $existing_type = gettype($existing_value);

  // Disallow setting the value of complex data types (e.g. arrays).
  if (!is_scalar($existing_value) && $existing_type != 'NULL') {
    bee_message(bt("'!option' is a complex data type and cannot be set via the command line.", array(
      '!option' => $arguments['option'],
    )), 'error');
    return;
  }

  // Attempt to set the config item using the correct type.
  $value = $arguments['value'];
  $config_item = $arguments['option'];
  switch ($existing_type) {
    case 'integer':
      // Make sure input is a number, and then ensure it's passed as integer.
      if (is_numeric($value)) {
        $value = (int) $value;
      }
      else {
        // Reject input that is invalid.
        $err_msg = bt("The type of the '!config_item' config item is !type, but the value you provided is of a different type.", array(
          '!type' => $existing_type,
          '!config_item' => $config_item,
        ));
        bee_message((string) $err_msg, 'error');
        return;
      }
      break;
    case 'boolean':
      // Check input is valid boolean - allow true/false and 1/0.
      $value = strtoupper($value);
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
          // Reject invalid input.
          $err_msg = bt("The type of the value of the '!config_item' config item is !type, but the value you provided is not a valid !type. Accepted values are 'TRUE', 'FALSE' (not case sensitive), '1' or '0'.", array(
            '!type' => $existing_type,
            '!config_item' => $config_item,
          ));
          bee_message((string) $err_msg, 'error');
          return;
      }
      break;
    case 'string':
      // Pass string as is but cast to string to be safe.
      $value = (string) $value;
      break;
    case 'NULL':
      // If config item is not set, default to string.
      $value = (string) $value;
      break;
    default:
        // Reject any other types - complex types already ruled out and no
        // others known at present.
        $err_msg = bt("The type of the '!config_item' config item is !type, which is not currently supported to be set via the command line. Please submit an issue listing the config_item, the file, the type, and which module has created it.", array(
          '!type' => $existing_type,
          '!state' => $config_item,
        ));
        bee_message((string) $err_msg, 'error');
        return;
  }

  // Set the value.
  $config->set($config_item, $value);
  $config->save();

  bee_message(bt("'!option' was set to '!value' in '!file'.", array(
    '!option' => $arguments['option'],
    '!value' => $arguments['value'],
    '!file' => $arguments['file'],
  )), 'success');
}

/**
 * Command callback: Clear the value of a specific config option in a given
 * file.
 */
function config_clear_bee_callback($arguments, $options) {
  // Get the config value.
  $value = config_get($arguments['file'], $arguments['option']);

  // Display errors for blank/missing values.
  if (!empty($arguments['option']) && is_null($value)) {
    bee_message(bt("'!option' could not be found in the '!file' config file.", array(
      '!option' => $arguments['option'],
      '!file' => $arguments['file'],
    )), 'error');
    return;
  }
  elseif (empty($arguments['option']) && empty($value)) {
    bee_message(bt("The '!file' config file could not be found.", array(
      '!file' => $arguments['file'],
    )), 'error');
    return;
  }

  // Clear the value.
  config_clear($arguments['file'], $arguments['option']);
  bee_message(bt("'!option' has been cleared from '!file'.", array(
    '!option' => $arguments['option'],
    '!file' => $arguments['file'],
  )), 'success');
}

/**
 * Command callback: Import config into the site.
 */
function config_import_bee_callback($arguments, $options) {
  global $_bee_backdrop_root;
  require_once $_bee_backdrop_root . '/core/modules/config/config.admin.inc';

  $statuses = config_get_statuses();
  $rows = array();

  // Get config to be imported.
  foreach ($statuses as $file => $status) {
    if (!empty($status)) {
      $rows[] = array(
        array('value' => $file),
        array('value' => $status),
      );
    }
  }

  // Show message and exit if there's no config to import.
  if (empty($rows)) {
    bee_message(bt('There is no config to import.'));
    return;
  }

  // Display config to be imported to the user.
  bee_render_text(array('value' => bt("The following config will be imported:\n")));
  bee_render_table(array(
    'rows' => $rows,
    'header' => array(
      array('value' => bt('Config')),
      array('value' => bt('Status')),
    ),
  ));

  // Prompt to continue.
  if (!bee_confirm(bt('Would you like to continue?'), FALSE)) {
    return;
  }

  require_once $_bee_backdrop_root . '/core/modules/config/config.sync.inc';

  // Validate config.
  try {
    foreach ($rows as $row) {
      config_sync_validate_file($row[0]['value'], $row[1]['value'], $statuses);
    }
  }
  catch (ConfigValidateException $e) {
    bee_message($e->getMessage(), 'error');
    return;
  }

  // As of Backdrop 1.32.0, modules and themes may be enabled via config import.
  // Import the special system.extensions config file first if it exists.
  if (isset($statuses['system.extensions']) && $statuses['system.extensions'] == 'update') {
    config_sync_file('system.extensions', 'update');

    // Revalidate the configuration after new modules have been enabled.
    // Hooks such as hook_config_import_validate() in the newly enabled modules
    // will be included as module_enable() clears the module_implements() cache.
    try {
      foreach ($rows as $row) {
        config_sync_validate_file($row[0]['value'], $row[1]['value'], $statuses);
      }
    }
    catch (ConfigValidateException $e) {
      bee_message($e->getMessage(), 'error');
      return;
    }
  }

  // Import config.
  foreach ($rows as $row) {
    config_sync_file($row[0]['value'], $row[1]['value']);
  }

  // Clean up.
  if (config_get('system.core', 'config_sync_clear_staging')) {
    $staging = config_get_config_storage('staging');
    $staging->deleteAll();
  }
  backdrop_flush_all_caches();

  bee_message(bt("Config was imported to '!active'.", array(
    '!active' => config_bee_get_config_location('active'),
  )), 'success');
  bee_message(format_plural(count($rows),
    '1 file was synced.',
    '@count files were synced.'
  ));
}

/**
 * Command callback: Export config from the site.
 */
function config_export_bee_callback($arguments, $options) {
  // Delete staging config.
  $staging = config_get_config_storage('staging');
  $staging->deleteAll();

  // Get active config.
  $active = config_get_config_storage('active');

  // Write each active config to staging.
  foreach ($active->listAll() as $name) {
    $data = $active->read($name);
    $staging->write($name, $data);
  }

  bee_message(bt("Config was exported to '!staging'.", array(
    '!staging' => config_bee_get_config_location('staging'),
  )), 'success');
}

/**
 * Helper function to get config storage location.
 *
 * @param string $type
 *   The type of config location: 'active' or 'staging'.
 *
 * @return string
 *   The selected storage location.
 */
function config_bee_get_config_location($type) {
  $class = settings_get('config_' . $type . '_class', 'ConfigFileStorage');
  switch ($class) {
    case 'ConfigDatabaseStorage':
      $config_location = 'db://default/config_' . $type;
      break;
    case 'ConfigFileStorage':
    default:
      $config_location = config_get_config_directory($type);
  }
  return $config_location;
}

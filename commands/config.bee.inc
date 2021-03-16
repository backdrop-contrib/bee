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
    bee_message(bt("'@option' could not be found in the '@file' config file.", array(
      '@option' => $arguments['option'],
      '@file' => $arguments['file'],
    )), 'error');
    return;
  }
  elseif (empty($arguments['option']) && empty($value)) {
    bee_message(bt("The '@file' config file could not be found.", array(
      '@file' => $arguments['file'],
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
    $answer = bee_confirm(bt("The '@file' config file doesn't exist. Would you like to create it?", array(
      '@file' => $arguments['file'],
    )), FALSE);
    if (!$answer) {
      return;
    }
  }
  // Disallow setting the value of complex data types (e.g. arrays).
  elseif (!is_scalar($config->get($arguments['option']))) {
    bee_message(bt("'@option' is a complex data type and cannot be set via the command line.", array(
      '@option' => $arguments['option'],
    )), 'error');
    return;
  }

  // Set the value.
  $config->set($arguments['option'], $arguments['value']);
  $config->save();

  bee_message(bt("'@option' was set to '@value'.", array(
    '@option' => $arguments['option'],
    '@value' => $arguments['value'],
  )), 'success');
}

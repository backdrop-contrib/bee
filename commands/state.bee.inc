<?php
/**
 * @file
 * Command(s) for getting and setting Backdrop states.
 */

/**
 * Implements hook_bee_command().
 */
function state_bee_command() {
  return array(
    'state-get' => array(
      'description' => bt('Get the value of a Backdrop state.'),
      'callback' => 'state_get_bee_callback',
      'arguments' => array(
        'state' => bt('The name of the state to get.'),
      ),
      'aliases' => array('sg', 'sget'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee state-get maintenance_mode' => bt("Get the value of the 'maintenance_mode' state."),
      ),
    ),
    'state-set' => array(
      'description' => bt('Set the value of a Backdrop state.'),
      'callback' => 'state_set_bee_callback',
      'arguments' => array(
        'state' => bt('The name of the state to set.'),
        'value' => bt('The value to set the state to.'),
      ),
      'aliases' => array('ss', 'sset'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee state-set maintenance_mode 1' => bt('Set the value of the maintenance_mode state.'),
      ),
    ),
    'maintenance-mode' => array(
      'description' => bt('Enable or disable maintenance mode for Backdrop.'),
      'callback' => 'maintenance_mode_bee_callback',
      'arguments' => array(
        'value' => bt('The value to set maintenance_mode to.'),
      ),
      'optional_arguments' => array('value'),
      'aliases' => array('mm'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee maintenance-mode 1' => bt('Enable maintenance mode for the site.'),
        'bee maintenance-mode 0' => bt('Disable maintenance mode for the site.'),
        'bee maintenance-mode' => bt('Get the maintenance mode status for the site.'),
      ),
    ),
  );
}

/**
 * Command callback: Get the value of a Backdrop state.
 */
function state_get_bee_callback($arguments, $options) {
  try {
    $value = state_get($arguments['state']);
    $type = gettype($value);
    switch ($type) {
      case 'integer':
        $value = (int) $value;
        break;
      case 'boolean':
        $value = ($value) ? 'TRUE' : 'FALSE';
        break;
      case 'string':
        $value = (string) $value;
        break;
      case 'array':
        $value = var_export($value, TRUE);
      default:
        $value = $value;
    }
    $msg = bt("The value of the '!state' state is: !value ", array(
      '!state' => $arguments['state'],
      '!value' => $value,
    ));
    bee_message((string) $msg, 'status');
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !state', array(
      '!msg' => $e->getMessage(),
      '!state' => $arguments['state'],
    ));
    bee_message((string) $err_msg, 'error');
  }
}

/**
 * Command callback: Set the value of a Backdrop state.
 */
function state_set_bee_callback($arguments, $options) {
  try {
    state_set($arguments['state'], $arguments['value']);
    $msg = bt("The '!state' state was set to: !value", array(
      '!state' => $arguments['state'],
      '!value' => $arguments['value'],
    ));
    bee_message((string) $msg, 'success');
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !state', array(
      '!msg' => $e->getMessage(),
      '!state' => $arguments['state'],
    ));
    bee_message((string) $err_msg, 'error');
  }
}

  /**
   * Command callback: Set the value of maintenance_mode for Backdrop.
   */
  function maintenance_mode_bee_callback($arguments, $options) {
    // Check if argument provided first
    if (isset($arguments['value'])) {
      // Check if argument matches existing value
      $value = (strtoupper($arguments['value']) == 'TRUE') ? (bool) TRUE : (bool) FALSE;
      if ((bool) state_get('maintenance_mode', FALSE) == $value) {
        $msg = bt('Maintenance mode is already !value', array(
          '!value' => ($value) ? 'enabled' : 'disabled',
        ));
        bee_message((string) $msg, 'status');
      }
      else {
        // Attempt to set maintenance mode
        try {
          state_set('maintenance_mode', $value);
          $msg = bt('Maintenance mode was !value', array(
            '!value' => ($value) ? 'enabled' : 'disabled',
          ));
          bee_message((string) $msg, 'success');
        }
        catch(ParseError $e) {
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
      // Argument not provided so get status
      $value = (bool) state_get('maintenance_mode', FALSE);
      $msg = bt('Maintenance mode is !status', array(
        '!status' => ($value) ? 'enabled' : 'disabled',
      ));
      bee_message((string) $msg, 'status');
    }
  }

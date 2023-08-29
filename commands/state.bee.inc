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
      'group' => 'state',
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
      'description' => bt('Set the value of an existing Backdrop state.'),
      'callback' => 'state_set_bee_callback',
      'group' => 'state',
      'arguments' => array(
        'state' => bt('The name of the state to set.'),
        'value' => bt('The value to set the state to.'),
      ),
      'aliases' => array('ss', 'sset'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee state-set maintenance_mode 1' => bt("Set the value of the 'maintenance_mode' state to '1' (enabled)."),
        'bee state-set database_utf8mb4_active TRUE' => bt("Set the value of the 'database_utf8mb4_active' state to 'TRUE'."),
      ),
    ),
    'maintenance-mode' => array(
      'description' => bt('Enable or disable maintenance mode for Backdrop.'),
      'callback' => 'maintenance_mode_bee_callback',
      'group' => 'miscellaneous',
      'arguments' => array(
        'value' => bt('A boolean value to enable (true/1) or disable (false/0) maintenance mode. Omitting the value will return the current maintenance mode status.'),
      ),
      'optional_arguments' => array('value'),
      'aliases' => array('mm'),
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee maintenance-mode true' => bt('Enable maintenance mode for the site (not case-sensitive).'),
        'bee maintenance-mode FALSE' => bt('Disable maintenance mode for the site (not case-sensitive).'),
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
    // Check the type and set to display in a meaningful way.
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
        break;
      case 'NULL':
        // If type is NULL then state does not exist.
        $err_msg = bt("State '!state' cannot be found. Please check your spelling and try again.", array(
          '!state' => $arguments['state'],
        ));
        bee_message((string) $err_msg, 'error');
        return;
      default:
        $value = $value;
        break;
    }
    $msg = bt("The value of the '!state' state is: !value (!type)", array(
      '!state' => $arguments['state'],
      '!value' => $value,
      '!type' => $type,
    ));
    return array(
      array(
        'type' => 'text',
        'variables' => array(
          'value' => $msg,
        ),
        'newline' => TRUE,
      ),
    );
  }
  catch (ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !state', array(
      '!msg' => $e->getMessage(),
      '!state' => $arguments['state'],
    ));
    bee_message((string) $err_msg, 'error');
    return;
  }
}

/**
 * Command callback: Set the value of a Backdrop state.
 */
function state_set_bee_callback($arguments, $options) {
  try {
    $state_name = (string) $arguments['state'];
    $value = $arguments['value'];
    // Check the type of the existing value.
    $existing_value = state_get($state_name);
    $existing_type = gettype($existing_value);
    switch ($existing_type) {
      case 'integer':
        // Make sure input is a number, and then ensure it's passed as integer.
        if (is_numeric($value)) {
          $value = (int) $value;
        }
        else {
          // Reject input that is invalid.
          $err_msg = bt("The type of the '!state' state is !type, but the value you provided is of a different type.", array(
            '!type' => $existing_type,
            '!state' => $state_name,
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
            $err_msg = bt("The type of the value of the '!state' state is !type, but the value you provided is not a valid !type. Accepted values are 'TRUE', 'FALSE' (not case sensitive), '1' or '0'.", array(
              '!type' => $existing_type,
              '!state' => $state_name,
            ));
            bee_message((string) $err_msg, 'error');
            return;
        }
        break;
      case 'string':
        // Pass string as is but cast to string to be safe.
        $value = (string) $value;
        break;
      case 'array':
        // Do not allow updating of array states.
        $err_msg = bt("The type of the '!state' state is !type, which cannot be set via the command line.", array(
          '!type' => $existing_type,
          '!state' => $state_name,
        ));
        bee_message((string) $err_msg, 'error');
        return;
      case 'NULL':
        // If type is NULL, then state does not exist and we do not want to
        // allow user to set it.
        $err_msg = bt("State '!state' cannot be found. New states cannot be created from the command line. Please check your spelling and try again.", array(
          '!state' => $arguments['state'],
        ));
        bee_message((string) $err_msg, 'error');
        return;
      default:
        // Reject any other types - no others known at present.
        $err_msg = bt("The type of the '!state' state is !type, which is not currently supported to be set via the command line. Please submit an issue listing the state, the type, and which module has created it.", array(
          '!type' => $existing_type,
          '!state' => $state_name,
        ));
        bee_message((string) $err_msg, 'error');
        return;
    }
    // If no errors, then set the state value accordingly.
    state_set($state_name, $value);
    // Feedback message needs boolean in readable form.
    if ($existing_type === 'boolean') {
      $value = $value ? 'TRUE' : 'FALSE';
    }
    // Give feedback on operation.
    $msg = bt("The '!state' state was set to: !value", array(
      '!state' => $state_name,
      '!value' => $value,
    ));
    bee_message((string) $msg, 'success');
    return;
  }
  catch (ParseError $e) {
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
    if ((bool) state_get('maintenance_mode', FALSE) == $value) {
      $msg = bt('Maintenance mode is already !value', array(
        '!value' => ($value) ? 'enabled' : 'disabled',
      ));
      bee_message((string) $msg, 'status');
    }
    else {
      // Attempt to set maintenance mode.
      try {
        state_set('maintenance_mode', (bool) $value);
        $msg = bt('Maintenance mode was !value', array(
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
    $value = (bool) state_get('maintenance_mode', FALSE);
    $msg = bt('Maintenance mode is !status', array(
      '!status' => ($value) ? 'enabled' : 'disabled',
    ));
    return array(
      array(
        'type' => 'text',
        'variables' => array(
          'value' => $msg,
        ),
        'newline' => TRUE,
      ),
    );
  }
}

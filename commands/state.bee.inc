<?php
/**
 * @file
 * Command(s) for getting and setting backdrop states.
 */

/**
 * Implements hook_bee_command().
 */
function state_bee_command() {
  return [
    'state-get' => [
      'description' => bt('Get the value of a Backdrop state.'),
      'callback' => 'state_get_bee_callback',
      'arguments' => [
        'state' => bt('The name of the state to get.'),
      ],
      'aliases' => ['sg, sget'],
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => [
        'bee state-get maintanance_mode' => bt('Get the value of the Backdrop state named "maintanance_mode".'),
      ],
    ],
    'state-set' => [
      'description' => bt('Set the value of a Backdrop state.'),
      'callback' => 'state_set_bee_callback',
      'arguments' => [
        'state' => bt('The name of the state to set.'),
        'value' => bt('The value to set the state to.'),
      ],
      'aliases' => ['ss, sset'],
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => [
        'bee state-set maintanance_mode 1' => bt('Set the value of the Backdrop state named "maintanance_mode" to 1.'),
      ],
    ],
  ];
}

/**
 * Command callback: Get the value of a Backdrop state.
 */
function state_get_bee_callback($arguments, $options) {
  try {
    $value = state_get($arguments['state']);
    $msg = bt('The value of the Backdrop state named "@state" is "@value".', [
      '@state' => $arguments['state'],
      '@value' => $value,
    ]);
    bee_message($msg, 'status');
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !state', array(
      '!msg' => $e->getMessage(),
      '!state' => $arguments['state'],
    ));
    bee_message($err_msg, 'error');
  }
}

/**
 * Command callback: Set the value of a Backdrop state.
 */
function state_set_bee_callback($arguments, $options) {
  try {
    state_set($arguments['state'], (int) $arguments['value']);
    $msg = bt('The value of the Backdrop state named "@state" is "@value".', [
      '@state' => $arguments['state'],
      '@value' => $arguments['value'],
    ]);
    bee_message($msg);
  }
  catch(ParseError $e) {
    // This is more readable than the default error we would get from PHP.
    $err_msg = bt('!msg in: !state', array(
      '!msg' => $e->getMessage(),
      '!state' => $arguments['state'],
    ));
    bee_message($err_msg, 'error');
  }
}

<?php
/**
 * @file
 * Command(s) for running cron.
 */

/**
 * Implements hook_bee_command().
 */
function cron_bee_command() {
  return array(
    'cron' => array(
      'description' => bt('Run cron.'),
      'callback' => 'cron_bee_callback',
      'group' => 'miscellaneous',
      'bootstrap' => BEE_BOOTSTRAP_FULL,
      'examples' => array(
        'bee cron' => bt('Initiate a cron run.'),
      ),
    ),
  );
}

/**
 * Command callback: Run cron.
 */
function cron_bee_callback($arguments, $options) {
  // Run cron.
  $result = backdrop_cron_run();

  // Display appropriate message.
  if ($result) {
    bee_message(bt('Cron ran successfully.'), 'success');
  }
  else {
    bee_message(bt('Cron is already running.'), 'error');
  }
}

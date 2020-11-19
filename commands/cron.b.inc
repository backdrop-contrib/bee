<?php
/**
 * @file
 * Command(s) for running cron.
 */

/**
 * Implements hook_b_command().
 */
function cron_b_command() {
  return array(
    'cron' => array(
      'description' => bt('Run cron.'),
      'callback' => 'cron_b_callback',
      'bootstrap' => B_BOOTSTRAP_FULL,
      'examples' => array(
        'b cron' => bt('Initiate a cron run.'),
      ),
    ),
  );
}

/**
 * Command callback: Run cron.
 */
function cron_b_callback($arguments, $options) {
  // Run cron.
  $result = backdrop_cron_run();

  // Display appropriate message.
  if ($result) {
    b_message(bt('Cron ran successfully.'), 'success');
  }
  else {
    b_message(bt('Cron is already running.'), 'error');
  }
}
